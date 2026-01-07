/* syntax.c  Merlin syntax module for vasm */
/* (c) in 2002-2025 by Frank Wille */
/* Merlin syntax adaptation (c) 2025 Bryan Woodruff, Cortexa LLC*/

#include "vasm.h"
#include <time.h>

/* The Merlin syntax module parses Merlin/Merlin 32 assembler syntax
   as used in Apple II/IIgs 6502/65816 development.

   Key features:
   - Comments: * in column 1, ; inline
   - Three-tier labels: global, local (:LABEL), variable (]LABEL)
   - Directives have NO prefix (e.g., ORG, DA, MAC not .OR, .DA, .MA)
   - Macros: MAC...<<<, direct call or PMC/>>> invocation, ]0-]8 parameters
   - String delimiters: " = high bit set, ' = high bit clear (same as SCMASM)
*/

const char *syntax_copyright="vasm Merlin syntax module 1.0 (c) 2025 Bryan Woodruff, Cortexa LLC";
hashtable *dirhash;
int dotdirs;

static char textname[]=".text",textattr[]="acrx";
static char dataname[]=".data",dataattr[]="adrw";
static char rodataname[]=".rodata",rodataattr[]="adr";
static char bssname[]=".bss",bssattr[]="aurw";
static char zeroname[]=".zero",zeroattr[]="aurwz";

char commentchar=';';  /* Merlin also allows * in column 1 (handled in parse loop) */

/* Merlin macro directives: MAC and <<< (or EOM) */
/* Note: dot prefix needed for internal formula compatibility (macroname+!dotdirs) */
static char macroname[] = ".mac";
static char endmname[] = ".<<<";
static char endrname[] = ".--^";  /* Merlin loop end */
static char reptname[] = ".lup";  /* Merlin loop */
static char repeatname[] = "repeat";
static struct namelen macro_dirlist[] = {
  { 3,macroname+1 }, { 0,0 }  /* mac (skip dot) */
};
static struct namelen endm_dirlist[] = {
  { 3,"<<<" }, { 2,"em" }, { 3,"eom" }, { 4,"endm" }, { 6,"endmac" }, { 8,"endmacro" }, { 0,0 }  /* <<< literal string */
};
static struct namelen rept_dirlist[] = {
  { 3,reptname+1 }, { 6,repeatname }, { 0,0 }  /* lup (skip dot) */
};
static struct namelen endr_dirlist[] = {
  { 3,endrname+1 }, { 4,"endr" }, { 6,"endrep" }, { 9,"endrepeat" }, { 0,0 }  /* --^ (skip dot) */
};
static struct namelen dmacro_dirlist[] = {
  { 3,macroname+1 }, { 0,0 }  /* mac (skip dot) */
};
static struct namelen dendm_dirlist[] = {
  { 3,"<<<" }, { 2,"em" }, { 3,"eom" }, { 4,"endm" }, { 6,"endmac" }, { 8,"endmacro" }, { 0,0 }  /* <<< literal string */
};
static struct namelen drept_dirlist[] = {
  { 3,reptname+1 }, { 6,repeatname }, { 0,0 }  /* lup (skip dot) */
};
static struct namelen dendr_dirlist[] = {
  { 3,endrname+1 }, { 4,"endr" }, { 6,"endrep" }, { 9,"endrepeat" }, { 0,0 }  /* --^ (skip dot) */
};

static char local_modif_name[] = "_";  /* ._ for abyte directive */
static char current_pc_str[2];

static int parse_end,nocprefix,nointelsuffix;
static int astcomment,dot_idchar,sect_directives;
static taddr orgmode = ~0;
static section *last_alloc_sect;
static taddr dsect_offs;
static int dsect_active;
static unsigned anon_labno;

/* isolated local labels block */
#define INLSTACKSIZE 100
#define INLLABFMT "=%06d"
static int inline_stack[INLSTACKSIZE];
static int inline_stack_index;
static const char *saved_last_global_label;
static char inl_lab_name[8];

/* Merlin three-tier label system */
static const char *merlin_last_global_label = NULL;  /* Last global label for :LABEL local labels */

/* Variable labels (]LABEL) - mutable labels with backward-only references */
struct varlabel {
  char *name;              /* Original ]LABEL name (with ] prefix) */
  char *unique_name;       /* Current unique name (unid_NNNN) */
  char *pending_name;      /* Pending unique name for deferred update */
  int definition_count;    /* Number of times this variable has been defined */
};

static hashtable *varlabel_hash = NULL;  /* Hash table for variable labels */
static int varlabel_counter = 0;         /* Unique ID counter for variable labels */
static struct varlabel *pending_varlabel = NULL;  /* Variable label awaiting finalization */

/* Merlin directive metadata */
static int merlin_file_type = -1;        /* ProDOS file type (TYP directive) */
static int merlin_aux_type = -1;         /* ProDOS auxiliary type (AUX directive) */
static int merlin_cycle_counting = 0;    /* CYC directive - cycle counting flag */
static char merlin_output_filename[256] = "";  /* SAV directive - output filename */

/* USR directive metadata for RW18 disk format (crackle compatibility) */
static int merlin_usr_valid = 0;         /* Flag: USR directive was processed */
static int merlin_usr_side = 0;          /* RW18 side (0xa9=side1, 0xad=side2, 0x79=side3) */
static int merlin_usr_track = 0;         /* RW18 track (0-34) */
static int merlin_usr_offset = 0;        /* RW18 intra-track offset (0-4607) */

/* Variable label management functions */
static struct varlabel *find_or_create_varlabel(const char *name, int len)
{
  hashdata data;
  struct varlabel *vl;

  if (varlabel_hash == NULL)
    varlabel_hash = new_hashtable(MAXMACPARAMS * 16);  /* reasonable initial size */

  /* Try to find existing variable label */
  if (find_namelen_nc(varlabel_hash, name, len, &data))
    return (struct varlabel *)data.ptr;

  /* Create new variable label entry */
  vl = mymalloc(sizeof(struct varlabel));
  vl->name = cnvstr(name, len);
  vl->unique_name = NULL;  /* Will be set on first definition */
  vl->pending_name = NULL;
  vl->definition_count = 0;

  /* Add to hash table - use case insensitive (1) to match find_namelen_nc */
  data.ptr = vl;
  add_hashentry(varlabel_hash, vl->name, data, 1);

  return vl;
}

static char *get_varlabel_unique_name(const char *name, int len)
{
  struct varlabel *vl = find_or_create_varlabel(name, len);

  /* For forward references: if no unique name yet, create one.
     Merlin allows forward references to variable labels within the same
     scope. The label will be defined later and resolved on a subsequent pass. */
  if (vl->unique_name == NULL) {
    char unique_name[32];
    snprintf(unique_name, sizeof(unique_name), "unid_%d", varlabel_counter++);
    vl->unique_name = mystrdup(unique_name);
  }

  return vl->unique_name;
}

/* Prepare a new variable label definition - creates new unique name but
   doesn't update unique_name yet (deferred until finalize_varlabel) */
static char *prepare_varlabel_definition(const char *name, int len)
{
  struct varlabel *vl = find_or_create_varlabel(name, len);
  char unique_name[32];

  /* For the first definition, if unique_name already exists (from forward ref),
     use it. Only create a new name for redefinitions. */
  if (vl->definition_count == 0 && vl->unique_name != NULL) {
    /* This is the first definition, but we have a forward reference.
       Use the existing unique_name (don't replace it). */
    if (vl->pending_name)
      myfree(vl->pending_name);
    vl->pending_name = mystrdup(vl->unique_name);
  }
  else {
    /* New definition (or first definition without forward ref) */
    snprintf(unique_name, sizeof(unique_name), "unid_%d", varlabel_counter++);
    if (vl->pending_name)
      myfree(vl->pending_name);
    vl->pending_name = mystrdup(unique_name);
  }

  /* Track this as the pending variable label */
  pending_varlabel = vl;

  return vl->pending_name;
}

/* Finalize the pending variable label definition - updates unique_name */
static void finalize_varlabel(void)
{
  if (pending_varlabel && pending_varlabel->pending_name) {
    /* Move pending_name to unique_name */
    if (pending_varlabel->unique_name)
      myfree(pending_varlabel->unique_name);
    pending_varlabel->unique_name = pending_varlabel->pending_name;
    pending_varlabel->pending_name = NULL;
    pending_varlabel->definition_count++;
  }
  pending_varlabel = NULL;
}

/* Legacy define_varlabel for immediate update (non-EQU contexts) */
static char *define_varlabel(const char *name, int len)
{
  char *result = prepare_varlabel_definition(name, len);
  finalize_varlabel();
  return result;
}

int igntrail;  /* ignore everything after a blank in the operand field */


int isidchar(char c)
{
  /* Merlin: tildes and question marks allowed in identifiers */
  /* Tilde (~) used for wrapper macros (e.g., ~CtlStartUp) */
  /* Question mark (?) used for boolean-style routines (e.g., StartGame?) */
  /* Underscores allowed except as sole character */
  /* NOTE: Period (.) is NOT allowed - it's the bit-OR operator (e.g., RdGrp.Inc = RdGrp | Inc) */
  if (isalnum((unsigned char)c) || c=='_' || c=='~' || c=='?')
    return 1;
  return 0;
}


char *skip(char *s)
{
  while (isspace((unsigned char )*s))
    s++;
  return s;
}


/* check for end of line, issue error, if not */
void eol(char *s)
{
  if (igntrail) {
    if (!ISEOL(s) && !isspace((unsigned char)*s))
      syntax_error(6);
  }
  else {
    s = skip(s);
    if (!ISEOL(s))
      syntax_error(6);
  }
}


char *exp_skip(char *s)
{
  if (!igntrail) {
    s = skip(s);
    if (*s == commentchar)
      *s = '\0';  /* rest of operand is ignored */
  }
  else if (isspace((unsigned char)*s) || *s==commentchar)
    *s = '\0';  /* rest of operand is ignored */
  return s;
}


static char *skip_operand(int instoper,char *s)
{
#ifdef VASM_CPU_Z80
  unsigned char lastuc = 0;
#endif
  int par_cnt = 0;
  char c = 0;

  for (;;) {
#ifdef VASM_CPU_Z80
    s = exp_skip(s);  /* @@@ why do we need that? */
    if (c)
      lastuc = toupper((unsigned char)*(s-1));
#endif
    c = *s;

    if (START_PARENTH(c))
      par_cnt++;
    else if (END_PARENTH(c)) {
      if (par_cnt>0)
        par_cnt--;
      else
        syntax_error(3);  /* too many closing parentheses */
    }
#ifdef VASM_CPU_Z80
    /* For the Z80 ignore ' behind a letter, as it may be a register */
    else if ((c=='\'' && (lastuc<'A' || lastuc>'Z')) || c=='\"') {
#else
    else if (c=='\'' || c=='\"') {
#endif
      /* a quote expects just a single character with an optional
         quote character following it */
      if (*(s+1) != '\0') {
        s++;
        if (*s == '\\')
          s = escape(s,NULL) - 1;
        else if (*s==c && *(s+1)==c)
          s++;  /* "" or '' is a single quote-character */
      }
      if (*(s+1) == c)  /* optional */
        s++;
    }
    else if ((!instoper || (instoper && OPERSEP_COMMA)) &&
             c==',' && par_cnt==0)
      break;
    else if (((instoper && OPERSEP_BLANK) || igntrail)
             && isspace((unsigned char)c) && par_cnt==0)
      break;
    else if (c=='\0' || c==commentchar)
      break;

    s++;
  }
  if(par_cnt != 0)
    syntax_error(4);  /* missing closing parentheses */
  return s;
}


char *my_skip_macro_arg(char *s)
{
  if (*s == '\\')
    s++;  /* leading \ in argument list is optional */
  return skip_identifier(s);
}


/* Merlin macro parameter separator
   Merlin uses semicolon (;) as primary separator, but also allows:
   comma (,), dot (.), dash (-), space, and opening paren (() */
char *merlin_macro_param_sep(char *p)
{
  if (*p == ';' || *p == ',' || *p == '.' || *p == '-' || *p == '/' ||
      *p == '(' || isspace((unsigned char)*p)) {
    /* Skip the separator character */
    p++;
    /* Skip any additional whitespace after separator */
    while (isspace((unsigned char)*p))
      p++;
    return p;
  }
  return NULL;  /* No valid separator found */
}


#define handle_data(a,b) handle_data_mod(a,b,NULL)

static void handle_data_mod(char *s,int size,expr *tree)
{
  expr **mod;

  if (tree) {
    /* modifier-expression is given, check for special symbol ._ in it */
    strbuf *buf = make_local_label(0,NULL,0,
                                   local_modif_name,
                                   sizeof(local_modif_name)-1);
    if (mod = find_sym_expr(&tree,buf->str)) {
      /* convert ._ into a harmless LABSYM symbol */
      (*mod)->c.sym->type = LABSYM;
      free_expr(*mod);
    }
    else {
      /* no ._ symbol in it - just treat the expression as addend */
      tree = make_expr(ADD,NULL,tree);
      mod = &tree->left;
    }
  }
  else
    mod = NULL;  /* no modifier */

  for (;;) {
    char *opstart = s;
    operand *op;
    dblock *db;

    if (OPSZ_BITS(size)==size && (*s=='\"' || *s=='\'')) {
      db = parse_string(&opstart,*s,size);
      s = opstart;
    }
    else
      db = NULL;

    if (db == NULL) {
      op = new_operand();
      s = skip_operand(0,s);
      if (parse_operand(opstart,s-opstart,op,DATA_OPERAND(size))) {
        atom *a;

#if defined(VASM_CPU_650X) || defined(VASM_CPU_Z80) || defined(VASM_CPU_6800)
        if (mod != NULL) {
          expr *tmpvalue = *mod = op->value;
          op->value = copy_tree(tree);
          free_expr(tmpvalue);
        }
#endif
        a = new_datadef_atom(OPSZ_BITS(size),op);
        a->align = 1;
        add_atom(0,a);
      }
      else
        syntax_error(8);  /* invalid data operand */
    }
    else {  /* got string in dblock */
#if defined(VASM_CPU_650X) || defined(VASM_CPU_Z80) || defined(VASM_CPU_6800)
      if (mod!=NULL && size==8) {
        /* make a defblock with an operand expression for each character */
        char buf[8];
        expr *tmpvalue;
        int i;

        for (i=0; i<db->size; i++) {
          op = new_operand();
          if (parse_operand(buf,sprintf(buf,"%u",(unsigned char)db->data[i]),
                            op,DATA_OPERAND(size))) {
            atom *a;

            *mod = tmpvalue = op->value;
            op->value = copy_tree(tree);
            free_expr(tmpvalue);
            a = new_datadef_atom(8,op);
            a->align = 1;
            add_atom(0,a);
          }
          else
            ierror(0);  /* shouldn't happen - it's only a decimal number */
        }
      }
      else
#endif
        add_atom(0,new_data_atom(db,1));  /* add dblock-string unmodified */
    }

    s = skip(s);
    if (*s == ',') {
      s = skip(s+1);
    }
    else if (*s == commentchar) {
      break;
    }
    else if (*s) {
      syntax_error(9);  /* , expected */
      return;
    }
    else
      break;
  }

  eol(s);
}


static void handle_secdata(char *s)
{
  if (sect_directives) {
    set_section(new_section(dotdirs?dataname:dataname+1,dataattr,1));
    eol(s);
  }
  else
    handle_data(s,8);
}


static atom *do_space(int size,expr *cnt,expr *fill)
{
  atom *a = new_space_atom(cnt,size>>3,fill);

  add_atom(0,a);
  return a;
}


static void handle_space(char *s,int size)
{
  expr *cnt,*fill=0;

  cnt = parse_expr_tmplab(&s);
  s = skip(s);
  if (*s == ',') {
    s = skip(s+1);
    fill = parse_expr_tmplab(&s);
  }
  do_space(size,cnt,fill);
  eol(s);
}


static void handle_uspace(char *s,int size)
{
  expr *cnt;
  atom *a;

  cnt = parse_expr_tmplab(&s);
  a = do_space(size,cnt,0);
  a->content.sb->flags |= SPC_UNINITIALIZED;
  eol(s);
}


static void handle_fixedspc(char *s,int nb)
{
  do_space(8,number_expr(nb),0);
  eol(s);
}


static void handle_d8(char *s)
{
#if BITSPERBYTE == 8
  s = skip(s);
  if (ISEOL(s))
    handle_fixedspc(s,1);
  else
#endif
    handle_data(s,8);
}


static void handle_dblbyt(char *s)
{
  s = skip(s);
  if (ISEOL(s))
    do_space(BITSPERBYTE,number_expr(2),0);
  else
    handle_data(s,BITSPERBYTE*2);
}


/* Merlin: DDB - Define Double Byte (big-endian word) */
static void handle_ddb(char *s)
{
  for (;;) {
    char *opstart = s;
    operand *op_high, *op_low;
    expr *value;
    atom *a;

    s = skip(s);
    if (ISEOL(s))
      break;

    /* Parse the expression */
    op_high = new_operand();
    s = skip_operand(0,s);
    if (!parse_operand(opstart,s-opstart,op_high,DATA_OPERAND(16))) {
      syntax_error(8);  /* invalid data operand */
      return;
    }

    value = op_high->value;

    /* Create operands for high and low bytes */
    op_low = new_operand();
    op_low->value = make_expr(BAND,copy_tree(value),number_expr(0xff));

    op_high->value = make_expr(RSH,copy_tree(value),number_expr(8));
    op_high->value = make_expr(BAND,op_high->value,number_expr(0xff));

    /* Emit high byte first (big-endian) */
    a = new_datadef_atom(8,op_high);
    a->align = 1;
    add_atom(0,a);

    /* Then low byte */
    a = new_datadef_atom(8,op_low);
    a->align = 1;
    add_atom(0,a);

    free_expr(value);

    s = skip(s);
    if (*s == ',')
      s++;
    else if (!ISEOL(s)) {
      syntax_error(9);  /* , expected */
      return;
    }
  }
}


static void handle_d16(char *s)
{
#if BITSPERBYTE == 8
  s = skip(s);
  if (ISEOL(s))
    handle_fixedspc(s,2);
  else
#endif
    handle_data(s,16);
}


static void handle_d24(char *s)
{
#if BITSPERBYTE == 8
  s = skip(s);
  if (ISEOL(s))
    handle_fixedspc(s,3);
  else
#endif
    handle_data(s,24);
}


static void handle_d32(char *s)
{
#if BITSPERBYTE == 8
  s = skip(s);
  if (ISEOL(s))
    handle_fixedspc(s,4);
  else
#endif
    handle_data(s,32);
}


/* Merlin: ADRL - Address Long (32-bit little-endian) */
static void handle_adrl(char *s)
{
#if BITSPERBYTE == 8
  s = skip(s);
  if (ISEOL(s))
    handle_fixedspc(s,4);
  else
#endif
    handle_data(s,32);
}


static void handle_d64(char *s)
{
#if BITSPERBYTE == 8
  s = skip(s);
  if (ISEOL(s))
    handle_fixedspc(s,8);
  else
#endif
    handle_data(s,64);
}


static void handle_taddr(char *s)
{
#if BITSPERBYTE == 8
  s = skip(s);
  if (ISEOL(s))
    handle_fixedspc(s,bytespertaddr);
  else
#endif
    handle_data(s,bytespertaddr*BITSPERBYTE);
}


#if defined(VASM_CPU_650X) || defined(VASM_CPU_Z80) || defined(VASM_CPU_6800)
static void handle_d8_mod(char *s)
{
  expr *modtree = parse_expr(&s);

  s = skip(s);
  if (*s == ',') {
    s = skip(s+1);
    handle_data_mod(s,8,modtree);
  }
  else
    syntax_error(9);  /* , expected */
}
#endif



static void do_text(char *s,unsigned char add)
{
  char *opstart = s;
  dblock *db;

  if (db = parse_string(&opstart,*s,8)) {
    if (db->data) {
      add_atom(0,new_data_atom(db,1));
      db->data[db->size-1] += add;
    }
    eol(opstart);
  }
  else if (!ISEOL(s) && !ISEOL(s+1)) {  /* store single character */
    db = new_dblock();

    db->size = 1;  /* 8 bits! */
    db->data = mymalloc(db->size);
    db->data[0] = s[1] + add;
    add_atom(0,new_data_atom(db,1));
  }
  else
    syntax_error(8);  /* invalid data operand */
}


static void handle_sectext(char *s)
{
  if (sect_directives) {
    set_section(new_section(dotdirs?textname:textname+1,textattr,1));
    eol(s);
  }
  else
    do_text(s,0);
}


static void handle_text(char *s)
{
  do_text(s,0);
}


static void handle_fcs(char *s)
{
  do_text(s,0x80);
}


static void handle_secbss(char *s)
{
  if (sect_directives) {
    set_section(new_section(dotdirs?bssname:bssname+1,bssattr,1));
    eol(s);
  }
  else
    handle_space(s,8);
}


static void do_alignment(taddr align,expr *offset)
{
  atom *a = new_space_atom(offset,1,0);

  a->align = align;
  add_atom(0,a);
}


static void handle_align(char *s)
{
  taddr a = parse_constexpr(&s);

  if (a > 63)
    syntax_error(21);  /* bad alignment */
  do_alignment(1LL<<a,number_expr(0));
  eol(s);
}


static void handle_even(char *s)
{
  do_alignment(2,number_expr(0));
  eol(s);
}


static void handle_spc8(char *s)
{
  s = skip(s);
  /* Merlin ds \ - pad to next page boundary (256 bytes) */
  if (*s == '\\') {
    s++;
    do_alignment(256,number_expr(0));
    eol(s);
    return;
  }
  handle_space(s,8);
}


static void handle_spc16(char *s)
{
  handle_space(s,16);
}


static void handle_spc24(char *s)
{
  handle_space(s,24);
}


static void handle_spc32(char *s)
{
  handle_space(s,32);
}


/* Helper function: Apply high bit based on delimiter
   Merlin rule: delimiters < apostrophe (0x27) set high bit */
static void apply_delimiter_highbit(dblock *db, char delim)
{
  if (db && db->size > 0 && (unsigned char)delim < 0x27) {
    /* Delimiter < apostrophe → set high bit on all characters */
    size_t i;
    for (i = 0; i < db->size; i++) {
      db->data[i] |= 0x80;
    }
  }
}


/* Helper function: Parse string handling single-char case
   parse_string() returns NULL for single-char strings, so we handle them manually */
static dblock *parse_merlin_string(char **sp, char delim)
{
  char *s = *sp;
  dblock *db = parse_string(sp, delim, 8);

  if (db == NULL) {
    /* Check for single-char string: delimiter + char + delimiter */
    if (s[0] && s[1] && s[2] == delim) {
      db = new_dblock();
      db->size = 1;
      db->data = mymalloc(1);
      db->data[0] = (unsigned char)s[1];
      *sp = s + 3;  /* skip past delimiter + char + delimiter */
    }
  }
  return db;
}


static void handle_spc64(char *s)
{
  handle_space(s,64);
}


static void handle_ascii(char *s)
{
  handle_data(s,8);
}


/* Merlin HEX directive - hex string */
static void handle_hs(char *s)
{
  char *p;
  unsigned char *hexdata;
  int len = 0, maxlen = 256;
  dblock *db;

  hexdata = mymalloc(maxlen);
  s = skip(s);

  while (*s && !ISEOL(s)) {
    int nibble1, nibble2;

    /* Skip whitespace and optional periods (Merlin allows periods as separators in HEX data) */
    while (*s && (isspace((unsigned char)*s) || *s == '.' || *s == ','))
      s++;

    if (ISEOL(s))
      break;

    /* Get first hex digit */
    if (isxdigit((unsigned char)*s)) {
      nibble1 = isdigit((unsigned char)*s) ? *s - '0' :
                tolower((unsigned char)*s) - 'a' + 10;
      s++;
    } else {
      syntax_error(15);  /* invalid hex digit */
      myfree(hexdata);
      return;
    }

    /* Get second hex digit */
    if (isxdigit((unsigned char)*s)) {
      nibble2 = isdigit((unsigned char)*s) ? *s - '0' :
                tolower((unsigned char)*s) - 'a' + 10;
      s++;
    } else {
      syntax_error(14);  /* hex string must contain even number of hex digits */
      myfree(hexdata);
      return;
    }

    /* Expand buffer if needed */
    if (len >= maxlen) {
      maxlen += 256;
      hexdata = myrealloc(hexdata, maxlen);
    }

    hexdata[len++] = (nibble1 << 4) | nibble2;
  }

  if (len > 0) {
    db = new_dblock();
    db->size = len;
    db->data = hexdata;
    add_atom(0, new_data_atom(db, 1));
  } else {
    myfree(hexdata);
  }

  eol(s);
}


static void handle_string(char *s)
{
  handle_data(s,8);
  add_atom(0,new_space_atom(number_expr(1),1,0));  /* terminating zero */
}


/* Merlin ASC directive - ASCII string with flexible delimiters */
static void handle_as(char *s)
{
  dblock *db;

  for (;;) {
    s = skip(s);
    if (ISEOL(s))
      break;

    /* Parse string with any non-whitespace delimiter */
    if (*s && !isspace((unsigned char)*s)) {
      char delim = *s;
      db = parse_merlin_string(&s,delim);
      if (db) {
        apply_delimiter_highbit(db, delim);  /* Apply delimiter rule */
        add_atom(0,new_data_atom(db,1));
      }
      else {
        syntax_error(30);  /* missing closing delimiter for string */
        return;
      }
    }
    else {
      syntax_error(30);  /* missing closing delimiter for string */
      return;
    }

    s = skip(s);
    if (*s == ',')
      s = skip(s+1);
    else if (!ISEOL(s)) {
      syntax_error(9);  /* , expected */
      return;
    }
  }
  eol(s);
}


/* Merlin STR directive - ASCII string with length prefix */
static void handle_az(char *s)
{
  dblock *db;

  for (;;) {
    s = skip(s);
    if (ISEOL(s))
      break;

    /* Parse string with any non-whitespace delimiter */
    if (*s && !isspace((unsigned char)*s)) {
      char delim = *s;
      db = parse_merlin_string(&s,delim);
      if (db) {
        apply_delimiter_highbit(db, delim);  /* Apply delimiter rule */
        add_atom(0,new_data_atom(db,1));
        /* Add zero terminator as separate space atom */
        add_atom(0,new_space_atom(number_expr(1),1,0));
      }
      else {
        syntax_error(30);  /* missing closing delimiter for string */
        return;
      }
    }
    else {
      syntax_error(30);  /* missing closing delimiter for string */
      return;
    }

    s = skip(s);
    if (*s == ',')
      s = skip(s+1);
    else if (!ISEOL(s)) {
      syntax_error(9);  /* , expected */
      return;
    }
  }
  eol(s);
}


/* Merlin DCI directive - ASCII with high bit inverted on last character */
static void handle_at(char *s)
{
  dblock *db;

  for (;;) {
    s = skip(s);

    if (ISEOL(s))
      break;

    /* Parse string with any non-whitespace delimiter */
    if (*s && !isspace((unsigned char)*s)) {
      char delim = *s;
      db = parse_merlin_string(&s,delim);
      if (db && db->size > 0) {
        /* Apply delimiter rule first */
        apply_delimiter_highbit(db, delim);
        /* Then invert high bit on last character (XOR 0x80) */
        db->data[db->size - 1] ^= 0x80;
        add_atom(0,new_data_atom(db,1));
      }
      else {
        syntax_error(30);  /* missing closing delimiter for string */
        return;
      }
    }
    else {
      syntax_error(30);  /* missing closing delimiter for string */
      return;
    }

    s = skip(s);
    if (*s == ',')
      s = skip(s+1);
    else if (!ISEOL(s)) {
      syntax_error(9);  /* , expected */
      return;
    }
  }
  eol(s);
}


/* Merlin INV directive - all chars with high bit set */
static void handle_inv(char *s)
{
  dblock *db;
  size_t i;

  for (;;) {
    s = skip(s);
    if (ISEOL(s))
      break;

    /* Parse string with any non-whitespace delimiter */
    if (*s && !isspace((unsigned char)*s)) {
      char delim = *s;
      db = parse_merlin_string(&s,delim);
      if (db) {
        /* Set high bit on all characters */
        for (i = 0; i < db->size; i++)
          db->data[i] |= 0x80;
        add_atom(0,new_data_atom(db,1));
      }
      else {
        syntax_error(30);  /* missing closing delimiter for string */
        return;
      }
    }
    else {
      syntax_error(30);  /* missing closing delimiter for string */
      return;
    }

    s = skip(s);
    if (*s == ',')
      s = skip(s+1);
    else if (!ISEOL(s)) {
      syntax_error(9);  /* , expected */
      return;
    }
  }
  eol(s);
}


/* Merlin FLS directive - flashing (alternating high bit) */
static void handle_fls(char *s)
{
  dblock *db;
  size_t i;

  for (;;) {
    s = skip(s);
    if (ISEOL(s))
      break;

    /* Parse string with any non-whitespace delimiter */
    if (*s && !isspace((unsigned char)*s)) {
      char delim = *s;
      db = parse_merlin_string(&s,delim);
      if (db) {
        /* Apply delimiter rule first */
        apply_delimiter_highbit(db, delim);
        /* Then alternate high bit: toggle on odd chars */
        for (i = 0; i < db->size; i++) {
          if (i & 1)
            db->data[i] ^= 0x80;
        }
        add_atom(0,new_data_atom(db,1));
      }
      else {
        syntax_error(30);  /* missing closing delimiter for string */
        return;
      }
    }
    else {
      syntax_error(30);  /* missing closing delimiter for string */
      return;
    }

    s = skip(s);
    if (*s == ',')
      s = skip(s+1);
    else if (!ISEOL(s)) {
      syntax_error(9);  /* , expected */
      return;
    }
  }
  eol(s);
}


/* Merlin REV directive - reversed byte order */
static void handle_rev(char *s)
{
  dblock *db;
  size_t i;

  for (;;) {
    s = skip(s);
    if (ISEOL(s))
      break;

    /* Parse string with any non-whitespace delimiter */
    if (*s && !isspace((unsigned char)*s)) {
      char delim = *s;
      db = parse_merlin_string(&s,delim);
      if (db) {
        /* Apply delimiter rule first */
        apply_delimiter_highbit(db, delim);
        /* Then reverse byte order */
        for (i = 0; i < db->size / 2; i++) {
          unsigned char temp = db->data[i];
          db->data[i] = db->data[db->size - 1 - i];
          db->data[db->size - 1 - i] = temp;
        }
        add_atom(0,new_data_atom(db,1));
      }
      else {
        syntax_error(30);  /* missing closing delimiter for string */
        return;
      }
    }
    else {
      syntax_error(30);  /* missing closing delimiter for string */
      return;
    }

    s = skip(s);
    if (*s == ',')
      s = skip(s+1);
    else if (!ISEOL(s)) {
      syntax_error(9);  /* , expected */
      return;
    }
  }
  eol(s);
}


/* Merlin STR directive - 1-byte length prefix */
static void handle_str(char *s)
{
  dblock *db, *lendb;

  for (;;) {
    s = skip(s);
    if (ISEOL(s))
      break;

    /* Parse string with any non-whitespace delimiter */
    if (*s && !isspace((unsigned char)*s)) {
      char delim = *s;
      db = parse_merlin_string(&s,delim);
      if (db) {
        unsigned char len;
        /* Apply delimiter rule */
        apply_delimiter_highbit(db, delim);
        /* Add 1-byte length prefix */
        len = (db->size > 255) ? 255 : db->size;
        lendb = new_dblock();
        lendb->size = 1;
        lendb->data = mymalloc(1);
        lendb->data[0] = len;
        add_atom(0,new_data_atom(lendb,1));
        add_atom(0,new_data_atom(db,1));
      }
      else {
        syntax_error(30);  /* missing closing delimiter for string */
        return;
      }
    }
    else {
      syntax_error(30);  /* missing closing delimiter for string */
      return;
    }

    s = skip(s);
    if (*s == ',')
      s = skip(s+1);
    else if (!ISEOL(s)) {
      syntax_error(9);  /* , expected */
      return;
    }
  }
  eol(s);
}


/* Merlin STRL directive - 2-byte length prefix (little-endian) */
static void handle_strl(char *s)
{
  dblock *db, *lendb;

  for (;;) {
    s = skip(s);
    if (ISEOL(s))
      break;

    /* Parse string with any non-whitespace delimiter */
    if (*s && !isspace((unsigned char)*s)) {
      char delim = *s;
      db = parse_merlin_string(&s,delim);
      if (db) {
        unsigned short len;
        /* Apply delimiter rule */
        apply_delimiter_highbit(db, delim);
        /* Add 2-byte length prefix (little-endian) */
        len = (db->size > 65535) ? 65535 : db->size;
        lendb = new_dblock();
        lendb->size = 2;
        lendb->data = mymalloc(2);
        lendb->data[0] = len & 0xFF;        /* low byte */
        lendb->data[1] = (len >> 8) & 0xFF; /* high byte */
        add_atom(0,new_data_atom(lendb,1));
        add_atom(0,new_data_atom(db,1));
      }
      else {
        syntax_error(30);  /* missing closing delimiter for string */
        return;
      }
    }
    else {
      syntax_error(30);  /* missing closing delimiter for string */
      return;
    }

    s = skip(s);
    if (*s == ',')
      s = skip(s+1);
    else if (!ISEOL(s)) {
      syntax_error(9);  /* , expected */
      return;
    }
  }
  eol(s);
}




/* Track XC state for cumulative XC calls */
static int xc_level = 0;  /* 0=6502, 1=65C02, 2=65816 */

static void handle_op(char *s)
{
  /* Merlin XC directive - change CPU type
     XC       - no args: increment CPU level (6502->65C02->65816)
     XC OFF   - reset to 6502
     XC ON    - enable 65816 mode
     Also accepts explicit CPU names: 6502, 65C02, 65816 */
  char cpuname[32];
  char *start, *end;
  int len;

  /* Skip whitespace */
  s = skip(s);
  start = s;

  /* Parse identifier (if any) */
  while (isalnum((unsigned char)*s))
    s++;

  end = s;
  len = end - start;

  if (len == 0) {
    /* XC with no arguments - increment CPU level */
    xc_level++;
    if (xc_level >= 2) {
      xc_level = 2;
      set_cpu_type("65816");
    } else {
      set_cpu_type("65c02");
    }
    eol(s);
    return;
  }

  /* Copy identifier to buffer */
  if (len >= sizeof(cpuname)) {
    syntax_error(24);  /* invalid CPU type for XC directive */
    return;
  }
  memcpy(cpuname, start, len);
  cpuname[len] = '\0';

  /* Check for OFF/ON keywords */
  if (!strnicmp(cpuname, "off", 3) && len == 3) {
    /* XC OFF - reset to 6502 and 8-bit mode */
    xc_level = 0;
    set_cpu_type("6502");
    set_65816_sizes(8, 8);  /* Reset to 8-bit A and X/Y */
  }
  else if (!strnicmp(cpuname, "on", 2) && len == 2) {
    /* XC ON - enable 65816 */
    xc_level = 2;
    set_cpu_type("65816");
  }
  else {
    /* Try as explicit CPU name */
    if (!set_cpu_type(cpuname)) {
      syntax_error(24);  /* invalid CPU type for XC directive */
      return;
    }
    /* Update xc_level based on CPU */
    if (!strnicmp(cpuname, "6502", 4) && (len == 4 || cpuname[4] == '\0'))
      xc_level = 0;
    else if (!strnicmp(cpuname, "65c02", 5))
      xc_level = 1;
    else if (!strnicmp(cpuname, "65816", 5))
      xc_level = 2;
  }

  eol(s);
}


/* Merlin MX directive - set 65816 M and X flags */
static void handle_mx(char *s)
{
  taddr flags;
  int asize, xsize;

  s = skip(s);
  flags = parse_constexpr(&s);

  /* MX sets processor status flags:
     Bit 0 (1): X/Y index registers = 8-bit (1) or 16-bit (0)
     Bit 1 (2): Accumulator = 8-bit (1) or 16-bit (0)
     MX %00 = 16-bit accumulator and index
     MX %01 = 16-bit accumulator, 8-bit index
     MX %10 = 8-bit accumulator, 16-bit index
     MX %11 = 8-bit accumulator and index (6502 emulation)
  */

  /* MX directive implies 65816 CPU - automatically enable it */
  set_cpu_type("816");

  /* Set immediate operand sizes (calls cpu_opts_init internally) */
  asize = (flags & 2) ? 8 : 16;  /* Bit 1: A size (1=8bit, 0=16bit) */
  xsize = (flags & 1) ? 8 : 16;  /* Bit 0: X/Y size (1=8bit, 0=16bit) */
  set_65816_sizes(asize, xsize);

  eol(s);
}


/* Merlin LONGA directive - set accumulator size mode */
static void handle_longa(char *s)
{
  int mode;

  s = skip(s);

  /* Parse ON or OFF */
  if (!strnicmp(s, "ON", 2)) {
    mode = 1;  /* 16-bit accumulator */
    s += 2;
  }
  else if (!strnicmp(s, "OFF", 3)) {
    mode = 0;  /* 8-bit accumulator */
    s += 3;
  }
  else {
    syntax_error(10);  /* identifier expected */
    return;
  }

  /* LONGA directive implies 65816 CPU - automatically enable it */
  set_cpu_type("816");

  /* Set accumulator size (X/Y defaults to 8-bit; use MX for both) */
  set_65816_sizes(mode ? 16 : 8, 8);

  eol(s);
}


/* Merlin LONGI directive - set index register size mode */
static void handle_longi(char *s)
{
  int mode;

  s = skip(s);

  /* Parse ON or OFF */
  if (!strnicmp(s, "ON", 2)) {
    mode = 1;  /* 16-bit index registers */
    s += 2;
  }
  else if (!strnicmp(s, "OFF", 3)) {
    mode = 0;  /* 8-bit index registers */
    s += 3;
  }
  else {
    syntax_error(10);  /* identifier expected */
    return;
  }

  /* LONGI directive implies 65816 CPU - automatically enable it */
  set_cpu_type("816");

  /* Set index register size (A defaults to 8-bit; use MX for both) */
  set_65816_sizes(8, mode ? 16 : 8);

  eol(s);
}


static void handle_str_cr(char *s)  /* String with CR terminator */
{
  expr *fill = number_expr(13);
  handle_data(s,8);
  add_atom(0,new_space_atom(number_expr(1),1,fill));  /* terminating CR */
}


static void handle_end(char *s)
{
  parse_end = 1;
  eol(s);
}


static void handle_fail(char *s)
{
  add_atom(0,new_assert_atom(NULL,NULL,mystrdup(s)));
}


static void handle_org(char *s)
{
  if (*s==current_pc_char && !isxdigit((unsigned char)s[1])) {
    char *s2 = skip(s+1);

    if (*s2++ == '+') {
      handle_uspace(skip(s2),8);  /*  "* = * + <expr>" to reserves bytes */
      return;
    }
  }
  else {
    taddr addr;
    expr *tree;

#if !defined(VASM_CPU_Z80)
    if (*s == '#')
      s = skip(s+1);  /* some strange assemblers allow ORG #<addr> */
#endif
    /* Parse expression and try to evaluate (supports multi-pass resolution) */
    tree = parse_expr(&s);
    if (tree) {
      simplify_expr(tree);
      if (tree->type == NUM) {
        addr = tree->c.val;
      }
      else if (eval_expr(tree, &addr, current_section,
                         current_section ? current_section->pc : 0)) {
        /* Successfully evaluated */
      }
      else {
        general_error(30);  /* expression must be constant */
        free_expr(tree);
        return;
      }
      free_expr(tree);

      if (dsect_active)
        switch_offset_section(NULL, addr);
      else
        set_section(new_org(addr));
    }
  }
  eol(s);
}


static void handle_rorg(char *s)
{
  start_rorg(parse_constexpr(&s));
  eol(s);
}


static void handle_rend(char *s)
{
  if (end_rorg())
    eol(s);
}


static void handle_roffs(char *s)
{
  add_atom(0,new_roffs_atom(parse_expr_tmplab(&s),NULL));
}


static void handle_section(char *s)
{
  char *name,*attr=NULL;
  section *sec;
  strbuf *buf;

  if (buf = parse_name(0,&s))
    name = buf->str;
  else
    return;

  s = skip(s);
  if (*s == ',') {
    strbuf *attrbuf;

    s = skip(s+1);
    if (attrbuf = get_raw_string(&s,'\"')) {
      attr = attrbuf->str;
      s = skip(s);
    }
  }
  if (attr == NULL) {
    if (!strcmp(name,textname) || !strcmp(name,textname+1))
      attr = textattr;
    else if (!strcmp(name,dataname) || !strcmp(name,dataname+1))
      attr = dataattr;
    else if (!strcmp(name,rodataname) || !strcmp(name,rodataname+1))
      attr = rodataattr;
    else if (!strcmp(name,bssname) || !strcmp(name,bssname+1))
      attr = bssattr;
    else if (!strcmp(name,zeroname) || !strcmp(name,zeroname+1))
      attr = zeroattr;
    else attr = defsecttype;
  }

  sec = new_section(name,attr,1);

  if (strchr(attr,'z'))
    sec->flags |= NEAR_ADDRESSING; /* meaning of zero-page addressing */
  else if (strchr(attr,'f'))
    sec->flags |= FAR_ADDRESSING;  /* use far-addressing (e.g. 24-bit) */

  set_section(sec);
  eol(s);
}


static void handle_dsect(char *s)
{
  taddr offs;
  expr *tree;

  /* Merlin allows consecutive DUM without DEND - just save current section once */
  if (!dsect_active) {
    last_alloc_sect = current_section;
    dsect_active = 1;
  }

  /* Parse optional address argument (e.g., "dum $f880" or "dum master") */
  s = skip(s);
  if (!ISEOL(s)) {
    tree = parse_expr(&s);
    if (tree) {
      /* Try to evaluate - this can resolve symbols defined earlier */
      simplify_expr(tree);
      if (tree->type == NUM) {
        offs = tree->c.val;
      }
      else if (eval_expr(tree, &offs, current_section,
                         current_section ? current_section->pc : 0)) {
        /* Successfully evaluated to a constant */
      }
      else {
        general_error(30);  /* expression must be a constant */
        offs = dsect_offs;
      }
      free_expr(tree);
    }
    else {
      offs = dsect_offs;
    }
  }
  else {
    offs = dsect_offs;  /* Use last offset if no argument */
  }

  /* Add zero-size space atom to prevent label on same line from being
     moved to the new section. In Merlin, "label dum" defines label at
     the current PC in the original section, not in the DUM section. */
  add_atom(0,new_space_atom(number_expr(0),1,NULL));
  switch_offset_section(NULL, offs);
  eol(s);
}


static void handle_dend(char *s)
{
  if (dsect_active) {
    dsect_offs = current_section->pc;
    /* Add zero-size space atom to prevent label on same line from being
       moved to the new section. In Merlin, "label dend" defines label at
       the end of the DUM section, not in the section being switched to. */
    add_atom(0,new_space_atom(number_expr(0),1,NULL));
    set_section(last_alloc_sect);
    last_alloc_sect = NULL;
    dsect_active = 0;
  }
  else
    syntax_error(21);  /* .ED without .DUMMY */
  eol(s);
}


static void do_binding(char *s,int bind)
{
  symbol *sym;
  strbuf *name;

  while (1) {
    if (!(name=parse_identifier(0,&s)))
      return;
    sym = new_import(name->str);
    if (sym->flags&(EXPORT|WEAK|LOCAL)!=0 &&
        sym->flags&(EXPORT|WEAK|LOCAL)!=bind)
      general_error(62,sym->name,get_bind_name(sym)); /* binding already set */
    else
      sym->flags |= bind;
    s = skip(s);
    if (*s != ',')
      break;
    s = skip(s+1);
  }
  eol(s);
}


static void handle_global(char *s)
{
  do_binding(s,EXPORT);
}


static void handle_weak(char *s)
{
  do_binding(s,WEAK);
}


static void handle_local(char *s)
{
  do_binding(s,LOCAL);
}


static void handle_symdepend(char *s)
{
  do_space(0,number_expr(0),parse_expr(&s));
  eol(s);
}


static void ifdef(char *s,int b)
{
  char *name;
  symbol *sym;
  int result;

  if (!(name = parse_symbol(&s))) {
    syntax_error(10);  /* identifier expected */
    return;
  }
  if (sym = find_symbol(name))
    result = sym->type != IMPORT;
  else
    result = 0;
  cond_if(result == b);
  eol(s);
}


static void ifused(char *s, int b)
{
  strbuf *name;
  symbol *sym;
  int result;

  if (!(name = parse_identifier(0,&s))) {
      syntax_error(10);  /* identifier expected */
      return;
  }

  if (sym = find_symbol(name->str)) {
    if (sym->type != IMPORT) {
      syntax_error(22,name->str);
      result = 0;
    }
    else
      result = 1;
  }
  else
    result = 0;

  cond_if(result == b);
  eol(s);
}


static void ifblank(char *s, int b)
{
  cond_if((*s==0 || *s==commentchar) == b);
}


static void handle_ifused(char *s)
{
  ifused(s,1);
}


static void handle_ifnused(char *s)
{
  ifused(s,0);
}


static void handle_ifd(char *s)
{
  ifdef(s,1);
}


static void handle_ifnd(char *s)
{
  ifdef(s,0);
}


static void handle_ifblank(char *s)
{
  ifblank(s,1);
}


static void handle_ifnblank(char *s)
{
  ifblank(s,0);
}


static char *ifexp(char *s,int c)
{
  expr *condexp = parse_expr_tmplab(&s);
  taddr val;
  int b;

  if (eval_expr(condexp,&val,NULL,0)) {
    switch (c) {
      case 0: b = val == 0; break;
      case 1: b = val != 0; break;
      case 2: b = val > 0; break;
      case 3: b = val >= 0; break;
      case 4: b = val < 0; break;
      case 5: b = val <= 0; break;
      default: ierror(0); break;
    }
  }
  else {
    general_error(30);  /* expression must be constant */
    b = 0;
  }
  cond_if(b);
  free_expr(condexp);
  return s;
}


static void handle_ifeq(char *s)
{
  eol(ifexp(s,0));
}


static void handle_ifne(char *s)
{
  eol(ifexp(s,1));
}


static void handle_ifgt(char *s)
{
  eol(ifexp(s,2));
}


static void handle_ifge(char *s)
{
  eol(ifexp(s,3));
}


static void handle_iflt(char *s)
{
  eol(ifexp(s,4));
}


static void handle_ifle(char *s)
{
  eol(ifexp(s,5));
}


static void handle_else(char *s)
{
  eol(s);
  cond_skipelse();
}


static void handle_endif(char *s)
{
  eol(s);
  cond_endif();
}


/* Merlin-specific FIN handler that tolerates extra FIN statements.
   Original Merlin assembler appears to silently ignore FIN when there's
   no matching DO/ELSE block, which allows constructs like:
     do X
     else
       ...code...
       fin          ; closes do/else
       ...code...
       fin          ; extra fin, ignored in Merlin
   This is needed for Prince of Persia source (SPECIALK.S) */
static void handle_fin(char *s)
{
  eol(s);
  if (clev > 0)
    cond_endif();
  else
    syntax_error(18);  /* warning: FIN without DO */
}


static void handle_assert(char *s)
{
  char *expstr,*msgstr=NULL;
  size_t explen;
  expr *aexp;
  atom *a;

  expstr = skip(s);
  aexp = parse_expr(&s);
  explen = s - expstr;
  s = skip(s);
  if (*s == ',') {
    strbuf *buf;

    s = skip(s+1);
    if (buf = parse_name(0,&s))
      msgstr = mystrdup(buf->str);
  }

  a = new_assert_atom(aexp,cnvstr(expstr,explen),msgstr);
  add_atom(0,a);
}


static void handle_incdir(char *s)
{
  strbuf *name;

  if (name = parse_name(0,&s))
    new_include_path(name->str);
  eol(s);
}


static void handle_include(char *s)
{
  strbuf *name;
  char *resolved;

  if (name = parse_name(0,&s)) {
    eol(s);
    /* Merlin: Try to find file with .S extension if base name not found */
    resolved = locate_file_with_extensions(name->str);
    if (resolved) {
      include_source(resolved);
      myfree(resolved);
    }
    else {
      /* Fall back to standard behavior (will report error) */
      include_source(name->str);
    }
  }
}


/* Merlin ProDOS prefix mapping for USE directive */
/* Maps ProDOS-style prefixes like "4/" to actual directories */
static const char *merlin_prefix_4 = NULL;  /* Set via VASM_MERLIN_PREFIX_4 env var */

static void handle_use(char *s)
{
  strbuf *name;
  char *path;
  char fullpath[1024];

  if (name = parse_name(0,&s)) {
    eol(s);

    /* Merlin USE directive for macro libraries */
    /* ProDOS path format: "4/UTIL.MACS" or "4:UTIL.MACS" */
    path = name->str;

    /* Check for ProDOS prefix (digit followed by / or :) */
    if (isdigit((unsigned char)path[0]) && (path[1] == '/' || path[1] == ':')) {
      int prefix = path[0] - '0';
      const char *base_path = NULL;
      const char *file_part = path + 2;  /* Skip "4/" or "4:" */

      /* Look up prefix mapping */
      if (prefix == 4 && merlin_prefix_4 == NULL) {
        /* Try environment variable on first use */
        merlin_prefix_4 = getenv("VASM_MERLIN_PREFIX_4");
        if (merlin_prefix_4 == NULL) {
          /* Default: look in same directory as source file or current directory */
          merlin_prefix_4 = "./";
        }
      }

      if (prefix == 4)
        base_path = merlin_prefix_4;

      if (base_path != NULL) {
        /* Construct full path and try with common extensions */
        FILE *test;

        snprintf(fullpath, sizeof(fullpath), "%s%s", base_path, file_part);

        /* Try exact name first */
        test = fopen(fullpath, "r");
        if (test) {
          fclose(test);
          include_source(fullpath);
        } else {
          /* Try with .S extension */
          snprintf(fullpath, sizeof(fullpath), "%s%s.S", base_path, file_part);
          test = fopen(fullpath, "r");
          if (test) {
            fclose(test);
            include_source(fullpath);
          } else {
            /* Try with .s extension */
            snprintf(fullpath, sizeof(fullpath), "%s%s.s", base_path, file_part);
            include_source(fullpath);  /* Let include_source report error if not found */
          }
        }
      } else {
        general_error(37);  /* Unsupported prefix number */
      }
    } else {
      /* Not a ProDOS prefix path - use as-is */
      include_source(path);
    }
  }
}


static void handle_rel(char *s)
{
  /* Merlin REL directive - generate relocatable output */
  /* This would typically set flags for the output module */
  /* For now, just acknowledge and ignore */
  eol(s);
}


static void handle_dsk(char *s)
{
  /* Merlin DSK directive - specify output disk file name */
  /* Format: DSK filename */
  strbuf *name;

  if (name = parse_name(0,&s)) {
    /* Set output file name - for now just ignore */
    eol(s);
  }
}


static void handle_incbin(char *s)
{
  strbuf *name;
  long delta = 0;
  unsigned long nbbytes = 0;

  if (name = parse_name(0,&s)) {
    s = skip(s);
    if (*s == ',') {
      s = skip(s+1);
      delta = parse_constexpr(&s);
      if (delta < 0)
        delta = 0;
      s = skip(s);
      if (*s == ',') {
        s = skip(s+1);
        nbbytes = parse_constexpr(&s);
      }
    }
    eol(s);
    include_binary_file(name->str,delta,nbbytes);
  }
}


static void handle_rept(char *s)
{
  int cnt = (int)parse_constexpr(&s);
  char *itername = NULL;

  s = skip(s);
  if (!ISEOL(s) && *s==',') {
    strbuf *name;

    s = skip(s+1);
    if (name = parse_identifier(0,&s)) {
      if (name)
        itername = name->str;
    }
  }
  new_repeat(cnt<0?0:cnt,itername,NULL,
             dotdirs?drept_dirlist:rept_dirlist,
             dotdirs?dendr_dirlist:endr_dirlist);
  eol(s);
}


static void handle_endr(char *s)
{
  syntax_error(12,&endrname[1],&repeatname[1]);  /* unexpected endr without rept */
}


static void handle_macro(char *s)
{
  strbuf *name;

  if (name = parse_identifier(0,&s)) {
    s = skip(s);
    if (*s == ',') {  /* named macro arguments are given? */
      s++;
    }
    else {
      eol(s);
      s = NULL;
    }
    new_macro(name->str,dotdirs?dmacro_dirlist:macro_dirlist,
              dotdirs?dendm_dirlist:endm_dirlist,s);
  }
  else
    syntax_error(10);  /* identifier expected */
}


static void handle_endm(char *s)
{
  syntax_error(12,&endmname[1],&macroname[1]);  /* unexpected endm without macro */
}


static void handle_exitmacro(char *s)
{
  leave_macro();
  eol(s);
}


static void handle_defc(char *s)
{
  strbuf *name;

  s = skip(s);
  name = parse_identifier(0,&s);
  if (name != NULL) {
    s = skip(s);
    if (*s == '=') {
      s = skip(s+1);
      new_abs(name->str,parse_expr_tmplab(&s));
    }
  }
  else
    syntax_error(10);
}


static void handle_list(char *s)
{
  set_listing(1);
}

static void handle_nolist(char *s)
{
  del_last_listing();  /* hide directive in listing */
  set_listing(0);
}

static void handle_listttl(char *s)
{
  /* set listing file title */
}

static void handle_listsubttl(char *s)
{
  /* set listing file sub-title */
}

static void handle_listpage(char *s)
{
  /* new listing page */
}

static void handle_listspace(char *s)
{
  /* insert listing space */
}


static void handle_struct(char *s)
{
  strbuf *name;

  if (name = parse_identifier(0,&s)) {
    s = skip(s);
    eol(s);
    if (new_structure(name->str))
      current_section->flags |= LABELS_ARE_LOCAL;
  }
  else
    syntax_error(10);  /* identifier expected */
}


static void handle_endstruct(char *s)
{
  section *structsec = current_section;
  section *prevsec;
  symbol *szlabel;

  if (end_structure(&prevsec)) {
    /* create the structure name as label defining the structure size */
    structsec->flags &= ~LABELS_ARE_LOCAL;
    szlabel = new_labsym(0,structsec->name);
    /* end structure declaration by switching to previous section */
    set_section(prevsec);
    /* avoid that this label is moved into prevsec in set_section() */
    add_atom(structsec,new_label_atom(szlabel));
  }
  eol(s);
}


static void handle_inline(char *s)
{
  static int id;
  const char *last;

  if (inline_stack_index < INLSTACKSIZE) {
    sprintf(inl_lab_name,INLLABFMT,id);
    last = set_last_global_label(inl_lab_name);
    if (inline_stack_index == 0)
      saved_last_global_label = last;
    inline_stack[inline_stack_index++] = id++;
  }
  else
    syntax_error(16,INLSTACKSIZE);  /* maximum inline nesting depth exceeded */
}


static void handle_einline(char *s)
{
  if (inline_stack_index > 0 ) {
    if (--inline_stack_index == 0) {
      set_last_global_label(saved_last_global_label);
      saved_last_global_label = NULL;
    }
    else {
      sprintf(inl_lab_name,INLLABFMT,inline_stack[inline_stack_index-1]);
      set_last_global_label(inl_lab_name);
    }
  }
  else
    syntax_error(17);  /* einline without inline */
}


/* Merlin: No-op handler for editor directives that should be ignored */
static void handle_noop(char *s)
{
  /* Silently ignore the directive and its arguments */
}

/* Merlin: ENT - Entry point (export symbol) */
static void handle_ent(char *s)
{
  char *name;
  strbuf *symname;

  for (;;) {
    s = skip(s);
    if (ISEOL(s))
      break;

    symname = parse_identifier(0, &s);
    if (!symname) {
      syntax_error(10);  /* identifier expected */
      return;
    }

    /* Mark symbol as exported */
    {
      symbol *sym = new_import(symname->str);
      sym->flags |= EXPORT;
    }

    s = skip(s);
    if (*s == ',')
      s++;
    else if (!ISEOL(s)) {
      syntax_error(9);  /* comma expected */
      return;
    }
  }
}


/* Merlin: EXT - External symbol (import symbol) */
static void handle_ext(char *s)
{
  strbuf *symname;

  for (;;) {
    s = skip(s);
    if (ISEOL(s))
      break;

    symname = parse_identifier(0, &s);
    if (!symname) {
      syntax_error(10);  /* identifier expected */
      return;
    }

    /* Mark symbol as imported */
    new_import(symname->str);

    s = skip(s);
    if (*s == ',')
      s++;
    else if (!ISEOL(s)) {
      syntax_error(9);  /* comma expected */
      return;
    }
  }
}


/* Merlin: SAV - Save (set output filename) */
static void handle_sav(char *s)
{
  char *name;
  int len;

  s = skip(s);
  name = s;

  if (!ISIDSTART(*s)) {
    syntax_error(10);  /* identifier expected */
    return;
  }

  while (ISIDCHAR(*s))
    s++;

  len = s - name;

  /* Set output filename */
  if (len > 0 && len < sizeof(merlin_output_filename) - 1) {
    strncpy(merlin_output_filename, name, len);
    merlin_output_filename[len] = '\0';
  }

  eol(s);
}


/* Merlin: ERR - User-defined error message */
static void handle_err(char *s)
{
  char *msg_start;
  int msg_len;
  char msgbuf[256];

  s = skip(s);

  /* Check if it's a string literal */
  if (*s == '"' || *s == '\'') {
    char delim = *s++;
    msg_start = s;

    /* Find closing delimiter */
    while (*s && *s != delim)
      s++;

    msg_len = s - msg_start;

    if (*s == delim)
      s++;

    /* Copy message and display it */
    if (msg_len > 0 && msg_len < sizeof(msgbuf) - 1) {
      strncpy(msgbuf, msg_start, msg_len);
      msgbuf[msg_len] = '\0';
      general_error(19, msgbuf);  /* "fail: %s" */
    }
    else {
      syntax_error(43);  /* user-defined error */
    }
    return;
  }

  /* Check if it's a number (error code) */
  if (isdigit((unsigned char)*s)) {
    taddr errcode = parse_constexpr(&s);
    snprintf(msgbuf, sizeof(msgbuf), "error code %d", (int)errcode);
    general_error(19, msgbuf);  /* "fail: %s" */
    return;
  }

  /* No argument - just error */
  syntax_error(43);  /* user-defined error */
}


/* Merlin: TYP - ProDOS file type */
static void handle_typ(char *s)
{
  taddr filetype;

  s = skip(s);
  filetype = parse_constexpr(&s);

  /* Set ProDOS file type (0-255) */
  if (filetype >= 0 && filetype <= 255) {
    merlin_file_type = (int)filetype;
  }
  else {
    syntax_error(8);  /* invalid data operand */
  }

  eol(s);
}


/* Merlin: AUX - ProDOS auxiliary type */
static void handle_aux(char *s)
{
  taddr auxtype;

  s = skip(s);
  auxtype = parse_constexpr(&s);

  /* Set ProDOS auxiliary type (16-bit value) */
  if (auxtype >= 0 && auxtype <= 0xFFFF) {
    merlin_aux_type = (int)auxtype;
  }
  else {
    syntax_error(8);  /* invalid data operand */
  }

  eol(s);
}


/* Merlin: TR - Truncate addressing mode */
static void handle_tr(char *s)
{
  s = skip(s);

  /* TR ON - enable truncation to zero page when possible */
  /* TR OFF - disable truncation */
  /* TR ADR - truncate only for addressing modes */

  if (!strnicmp(s, "on", 2)) {
    /* Enable truncation */
    s += 2;
  }
  else if (!strnicmp(s, "off", 3)) {
    /* Disable truncation */
    s += 3;
  }
  else if (!strnicmp(s, "adr", 3)) {
    /* Truncate addressing modes only */
    s += 3;
  }

  /* For now, just accept - would need CPU module integration */
  eol(s);
}


/* Merlin: EXP - Expand macros in listing */
static void handle_exp(char *s)
{
  s = skip(s);

  /* EXP ON - expand macros in listing */
  /* EXP OFF - don't expand macros */
  /* EXP ONLY - only show macro expansion, not macro call */

  if (!strnicmp(s, "on", 2)) {
    set_listing(1);
    s += 2;
  }
  else if (!strnicmp(s, "off", 3)) {
    set_listing(0);
    s += 3;
  }
  else if (!strnicmp(s, "only", 4)) {
    set_listing(1);
    s += 4;
  }

  eol(s);
}


/* Merlin: OBJ - Object code listing control */
static void handle_obj(char *s)
{
  s = skip(s);

  /* OBJ ON - show object code in listing */
  /* OBJ OFF - hide object code in listing */

  if (!strnicmp(s, "on", 2)) {
    set_listing(1);
    s += 2;
  }
  else if (!strnicmp(s, "off", 3)) {
    set_listing(0);
    s += 3;
  }

  eol(s);
}


/* Merlin: CYC - Cycle counting */
static void handle_cyc(char *s)
{
  s = skip(s);

  /* CYC ON - enable cycle counting in listing */
  /* CYC OFF - disable cycle counting */

  if (!strnicmp(s, "on", 2)) {
    merlin_cycle_counting = 1;
    s += 2;
  }
  else if (!strnicmp(s, "off", 3)) {
    merlin_cycle_counting = 0;
    s += 3;
  }

  eol(s);
}


/* Merlin: KND - OMF segment kind */
static void handle_knd(char *s)
{
  taddr kind;

  s = skip(s);
  kind = parse_constexpr(&s);

  /* Set OMF segment kind (16-bit value) */
  /* Used for Apple IIgs OMF output format */
  if (current_section) {
    current_section->memattr = kind & 0xFFFF;
  }

  eol(s);
}


/* Merlin: ALI - OMF alignment */
static void handle_ali(char *s)
{
  s = skip(s);

  /* ALI BANK - align to 64K bank boundary */
  /* ALI PAGE - align to 256-byte page boundary */
  /* ALI NONE - no special alignment */

  if (!strnicmp(s, "bank", 4)) {
    /* Align to bank */
    current_section->align = 65536;
    s += 4;
  }
  else if (!strnicmp(s, "page", 4)) {
    /* Align to page */
    current_section->align = 256;
    s += 4;
  }
  else if (!strnicmp(s, "none", 4)) {
    /* No alignment */
    current_section->align = 1;
    s += 4;
  }

  eol(s);
}


/* Merlin: LNK - Linker script generation */
static void handle_lnk(char *s)
{
  strbuf *name;

  s = skip(s);

  /* LNK filename - generate linker script */
  /* Rarely used directive */

  if (name = parse_name(0, &s)) {
    /* Would generate linker script file */
    /* For now, just accept and ignore */
  }

  eol(s);
}


/* Merlin: CHK - Calculate and emit checksum byte */
static void handle_chk(char *s)
{
  atom *a;
  dblock *db;

  eol(s);

  /* Emit a single byte that will be patched during output
     with the EOR checksum of all bytes from section start to here */

  /* For now, emit a placeholder byte
     Full checksum calculation would require output module integration */
  db = new_dblock();
  db->size = 1;
  db->data = mymalloc(1);
  db->data[0] = 0;  /* Placeholder - would be calculated during output */
  a = new_data_atom(db, 1);
  add_atom(0, a);
}


/* Merlin: USR - User routine call for RW18 disk format */
/* Format: USR side,track,offset,length */
/* This stores metadata for crackle disk imaging compatibility */
static void handle_usr(char *s)
{
  expr *side_expr, *track_expr, *offset_expr, *len_expr;
  taddr side_val, track_val, offset_val;

  s = skip(s);
  if (ISEOL(s)) {
    syntax_error(16);  /* missing operand */
    return;
  }

  /* Parse side (e.g., $a9, $ad, $79) */
  side_expr = parse_expr(&s);
  if (side_expr == NULL || !eval_expr(side_expr, &side_val, NULL, 0)) {
    syntax_error(16);  /* missing operand */
    return;
  }
  free_expr(side_expr);

  s = skip(s);
  if (*s != ',') {
    syntax_error(16);  /* missing operand */
    return;
  }
  s = skip(s + 1);

  /* Parse track (0-34) */
  track_expr = parse_expr(&s);
  if (track_expr == NULL || !eval_expr(track_expr, &track_val, NULL, 0)) {
    syntax_error(16);  /* missing operand */
    return;
  }
  free_expr(track_expr);

  s = skip(s);
  if (*s != ',') {
    syntax_error(16);  /* missing operand */
    return;
  }
  s = skip(s + 1);

  /* Parse offset (intra-track offset, 0-4607) */
  offset_expr = parse_expr(&s);
  if (offset_expr == NULL || !eval_expr(offset_expr, &offset_val, NULL, 0)) {
    syntax_error(16);  /* missing operand */
    return;
  }
  free_expr(offset_expr);

  s = skip(s);
  if (*s != ',') {
    syntax_error(16);  /* missing operand */
    return;
  }
  s = skip(s + 1);

  /* Parse length expression (usually *-org) - we don't store it, length comes from binary size */
  len_expr = parse_expr(&s);
  if (len_expr != NULL)
    free_expr(len_expr);

  /* Store the USR metadata */
  merlin_usr_valid = 1;
  merlin_usr_side = (int)side_val;
  merlin_usr_track = (int)track_val;
  merlin_usr_offset = (int)offset_val;

  eol(s);
}


/* Merlin: DAT - Date/time stamp in assembly listing */
static void handle_dat(char *s)
{
  time_t now;
  char *timestamp;

  eol(s);

  /* Print date/time to listing */
  if (listena) {
    now = time(NULL);
    timestamp = ctime(&now);
    if (timestamp) {
      /* Remove trailing newline */
      char *nl = strchr(timestamp, '\n');
      if (nl) *nl = '\0';
      printf("; Assembly date: %s\n", timestamp);
    }
  }
}


/* Merlin: Handle PMC directive - explicit macro call */
static void handle_pmc(char *s)
{
  char *name_start;
  int name_len;
  macro *m;

  s = skip(s);
  name_start = s;

  /* Read macro name */
  if (!ISIDSTART(*s)) {
    syntax_error(10);  /* identifier expected */
    return;
  }
  while (ISIDCHAR(*s))
    s++;
  name_len = s - name_start;

  /* Look up macro */
  m = find_macro(name_start, name_len);
  if (m != NULL) {
    /* Found macro - execute it */
    if (!isspace((unsigned char)*s) && *s!='\0')
      syntax_error(2);  /* no space before operands */
    s = skip(s);

    execute_macro(name_start, name_len, NULL, NULL, 0, s);
  }
  else {
    /* Macro not found - error */
    syntax_error(42);  /* undefined macro */
  }
}

struct {
  const char *name;
  void (*func)(char *);
} directives[] = {
  "org",handle_org,
  "rorg",handle_rorg,
  "rend",handle_rend,
  "phase",handle_rorg,
  "dephase",handle_rend,
  "ph",handle_rorg,      /* Merlin PH - phase start (rorg) */
  "ep",handle_rend,      /* Merlin EP - end phase */
  "roffs",handle_roffs,
  "align",handle_align,
  "even",handle_even,
  "data",handle_secdata,
  /* "text" - Merlin TEXT directive for source listing path */
  "bss",handle_secbss,
#if defined(VASM_CPU_650X) || defined(VASM_CPU_Z80) || defined(VASM_CPU_6800)
  "abyte",handle_d8_mod,
#endif
  "asc",handle_ascii,
  "ascii",handle_ascii,
  "asciiz",handle_string,
  "string",handle_string,
  "str",handle_str,  /* GMGM */
  "defm",handle_text,
  "fcc",handle_text,
  "fcs",handle_fcs,
  "fcb",handle_d8,
  "byt",handle_d8,
  "byte",handle_d8,
  "db",handle_d8,
  "dfb",handle_d8,
  "defb",handle_d8,
  "byt",handle_d8,
  "fdb",handle_dblbyt,
  "ddb",handle_ddb,          /* Merlin DDB - big-endian word */
  "wrd",DATWORD,
  "wor",DATWORD,
  "word",DATWORD,
  "wrd",DATWORD,
  "dw",DATWORD,
  "dfw",DATWORD,
  "defw",DATWORD,
  "dl",DATLONG,
  "defl",DATLONG,
  "dd",DATDWRD,
#if !defined(VASM_CPU_6809)  /* clash with 6309 ADDR instruction */
  "addr",handle_taddr,
#endif
  "da",handle_taddr,
  "defp",handle_taddr,
  "adrl",handle_adrl,        /* Merlin ADRL - 32-bit address */
  "ds",handle_spc8,
  "dsb",handle_spc8,
  "fill",handle_spc8,
  "reserve",handle_spc8,
  "spc",handle_spc8,
  "defs",handle_spc8,
  "bsz",handle_spc8,
  "zmb",handle_spc8,
  "dc",handle_spc8,
  "blk",handle_spc8,
#if !defined(VASM_CPU_650X)
  "rmb",handle_spc8,
#endif
  "dsw",SPCWORD,
  "blkw",SPCWORD,
  "dsl",SPCLONG,
  "blkl",SPCLONG,
  "di8",handle_d8,
  "di16",handle_d16,
  "di24",handle_d24,
  "di32",handle_d32,
  "di64",handle_d64,
  "ds8",handle_spc8,
  "ds16",handle_spc16,
  "ds24",handle_spc24,
  "ds32",handle_spc32,
  "ds64",handle_spc64,
  "assert",handle_assert,
#if defined(VASM_CPU_TR3200) /* Clash with IFxx instructions of TR3200 cpu */
  "if_def",handle_ifd,
  "if_ndef",handle_ifnd,
  "if_blank",handle_ifblank,
  "if_nblank",handle_ifnblank,
  "if_eq",handle_ifeq,
  "if_ne",handle_ifne,
  "if_gt",handle_ifgt,
  "if_ge",handle_ifge,
  "if_lt",handle_iflt,
  "if_le",handle_ifle,
  "if_used",handle_ifused,
  "if_nused",handle_ifnused,
#else
  "ifdef",handle_ifd,
  "ifndef",handle_ifnd,
  "ifblank",handle_ifblank,
  "ifnblank",handle_ifnblank,
  "ifd",handle_ifd,
  "ifnd",handle_ifnd,
  "ifeq",handle_ifeq,
  "ifne",handle_ifne,
  "ifgt",handle_ifgt,
  "ifge",handle_ifge,
  "iflt",handle_iflt,
  "ifle",handle_ifle,
  "ifused",handle_ifused,
  "ifnused",handle_ifnused,
#endif
  "if",handle_ifne,
  "else",handle_else,
  "el",handle_else,
  "endif",handle_endif,
#if !defined(VASM_CPU_Z80) && !defined(VASM_CPU_6800) && !defined(VASM_CPU_SPC700)
  "ei",handle_endif,  /* clashes with cpu mnemonic */
#endif
  "fi",handle_endif,  /* GMGM */
  "incbin",handle_incbin,
  "mdat",handle_incbin,
  "incdir",handle_incdir,
  "include",handle_include,
  "put",handle_include,      /* Merlin PUT - include source file */
  "use",handle_use,          /* Merlin USE - include macro library */
  "rel",handle_rel,          /* Merlin REL - relocatable output */
  "dsk",handle_dsk,          /* Merlin DSK - output disk file name */
  "rept",handle_rept,
  "repeat",handle_rept,
  "lup",handle_rept,         /* Merlin LUP - loop directive */
  "endr",handle_endr,
  "endrep",handle_endr,
  "endrepeat",handle_endr,
#if !defined(VASM_CPU_UNSP)
  "mac",handle_macro, /* Clashes with unSP instruction */
#endif
  "ma",handle_macro,   /* Merlin MAC macro directive (alternate name) */
  "macro",handle_macro,
  "em",handle_endm,    /* Merlin <<< endmacro directive (alternate name) */
  "endm",handle_endm,
  "endmac",handle_endm,
  "endmacro",handle_endm,
  "end",handle_end,
  "exitmacro",handle_exitmacro,
  "fail",handle_fail,
  "section",handle_section,
  "dsect",handle_dsect,
  "dend",handle_dend,
  "dum",handle_dsect,    /* Merlin DUM - dummy section start */
  "dummy",handle_dsect,  /* Merlin DUM - dummy section start (long form) */
  "dend",handle_dend,    /* Merlin DEND - end dummy section */
  "ed",handle_dend,      /* Merlin DEND - end dummy (alias) */
  "binary",handle_incbin,
  "inb",handle_incbin,   /* Merlin PUTBIN - include binary (alias) */
  "defc",handle_defc,
  "xdef",handle_global,
  "xref",handle_global,
  "lib",handle_global,
  "xlib",handle_global,
  "global",handle_global,
  "extern",handle_global,
  "local",handle_local,
  "weak",handle_weak,
  "needs",handle_symdepend,
  "symdepend",handle_symdepend,
  "list",handle_list,
  "lst",handle_list,         /* Merlin LST - list control */
  "lstdo",handle_list,       /* Merlin LSTDO - list DO blocks */
  "nolist",handle_nolist,
  "struct",handle_struct,
  "structure",handle_struct,
  "endstruct",handle_endstruct,
  "endstructure",handle_endstruct,
  "inline",handle_inline,
  "einline",handle_einline,
  "nam",handle_listttl,
  "subttl",handle_listsubttl,
  "page",handle_listpage,
  "space",handle_listspace,
  /* Merlin-specific directive aliases */
  "or",handle_org,        /* .OR - origin (alias for org) */
  "bs",handle_spc8,       /* .BS - block storage (alias for ds) */
  "do",handle_ifne,       /* .DO - conditional start (like if) */
  "else",handle_else,     /* .ELSE - conditional else clause */
  "fin",handle_fin,       /* .FIN - Merlin tolerant conditional end */
  /* Merlin string directives */
  "asc",handle_as,        /* ASC - ASCII string */
  "dci",handle_at,        /* DCI - ASCII with high bit inverted on last char */
  "inv",handle_inv,       /* INV - all chars with high bit set */
  "fls",handle_fls,       /* FLS - flashing (alternating high bit) */
  "rev",handle_rev,       /* REV - reversed byte order */
  "str",handle_str,       /* STR - 1-byte length prefix */
  "strl",handle_strl,     /* STRL - 2-byte length prefix */
  "hex",handle_hs,        /* HEX - hex string */
  /* Alternative directive names */
  "as",handle_as,         /* AS - ASCII string (alternative name) */
  "az",handle_az,         /* AZ - ASCII zero-terminated */
  "at",handle_at,         /* AT - DCI (alternative name) */
  "hs",handle_hs,         /* HS - hex string (alternative name) */
  "op",handle_op,         /* .OP - select CPU type (6502/65C02/65816) */
  /* Merlin 65816 CPU mode directives */
  "mx",handle_mx,         /* MX - set M and X processor flags */
  "xc",handle_op,         /* XC - change CPU type (alias for OP) */
  "longa",handle_longa,   /* LONGA ON/OFF - 16-bit accumulator mode */
  "longi",handle_longi,   /* LONGI ON/OFF - 16-bit index register mode */
  /* Merlin macro invocation */
  "pmc",handle_pmc,       /* PMC - explicit macro call */
  /* Merlin linking directives */
  "ent",handle_ent,       /* ENT - Entry point (export symbol) */
  "ext",handle_ext,       /* EXT - External symbol (import symbol) */
  /* Merlin output control */
  "sav",handle_sav,       /* SAV - Save (set output filename) */
  "err",handle_err,       /* ERR - User-defined error message */
  "typ",handle_typ,       /* TYP - ProDOS file type */
  "aux",handle_aux,       /* AUX - ProDOS auxiliary type */
  /* Merlin addressing mode control */
  "tr",handle_tr,         /* TR - Truncate addressing mode */
  /* Merlin listing control */
  "exp",handle_exp,       /* EXP - Expand macros in listing */
  "obj",handle_obj,       /* OBJ - Object code listing control */
  "cyc",handle_cyc,       /* CYC - Cycle counting */
  /* Merlin OMF directives */
  "knd",handle_knd,       /* KND - OMF segment kind */
  "ali",handle_ali,       /* ALI - OMF alignment */
  "lnk",handle_lnk,       /* LNK - Linker script generation */
  /* Merlin assembly utility directives */
  "chk",handle_chk,       /* CHK - Checksum byte */
  "dat",handle_dat,       /* DAT - Date/time stamp */
  "usr",handle_usr        /* USR - ProDOS MLI call (no-op in cross-assembly) */
};

int dir_cnt = sizeof(directives) / sizeof(directives[0]);


/* checks for a valid directive, and return index when found, -1 otherwise */
static int check_directive(char **line)
{
  char *s,*name;
  hashdata data;

  s = skip(*line);

  /* Merlin: Special handling for <<< end macro delimiter */
  if (s[0] == '<' && s[1] == '<' && s[2] == '<') {
    name = "em";  /* treat <<< as EM directive (handle_endm) */
    *line = s + 3;
    if (find_name_nc(dirhash, name, &data)) {
      return data.idx;
    }
    return -1;
  }

  /* Merlin: Special handling for --^ end loop delimiter */
  if (s[0] == '-' && s[1] == '-' && s[2] == '^') {
    name = "endr";  /* treat --^ as ENDR directive (handle_endr) */
    *line = s + 3;
    if (find_name_nc(dirhash, name, &data)) {
      return data.idx;
    }
    return -1;
  }

  /* Handle . prefix for directives (optional in Merlin, for compatibility) */
  if (dotdirs && *s == '.') {
    name = s + 1;  /* skip the . */
    s++;
    if (!ISIDSTART(*s))
      return -1;
    s++;
    while (ISIDCHAR(*s))
      s++;
  }
  else {
    if (!ISIDSTART(*s))
      return -1;
    name = s++;
    while (ISIDCHAR(*s))
      s++;
  }

  if (!find_namelen_nc(dirhash,name,s-name,&data))
    return -1;
  *line = s;
  return data.idx;
}


/* Handles assembly directives; returns non-zero if the line
   was a directive. */
static int handle_directive(char *line)
{
  int idx = check_directive(&line);

  if (idx >= 0) {
    directives[idx].func(skip(line));
    return 1;
  }
  return 0;
}


static int oplen(char *e,char *s)
{
  while(s!=e&&isspace((unsigned char)e[-1]))
    e--;
  return e-s;
}


/* When a structure with this name exists, insert its atoms and either
   initialize with new values or accept its default values. */
static int execute_struct(char *name,int name_len,char *s)
{
  section *str;
  atom *p;

  str = find_structure(name,name_len);
  if (str == NULL)
    return 0;

  for (p=str->first; p; p=p->next) {
    atom *new;
    char *opp;
    int opl;

    if (p->type==DATA || p->type==SPACE || p->type==DATADEF) {
      opp = s = skip(s);
      s = skip_operand(0,s);
      opl = oplen(s,opp);

      if (opl > 0) {
        /* initialize this atom with a new expression */

        if (p->type == DATADEF) {
          /* parse a new data operand of the declared bitsize */
          operand *op;

          op = new_operand();
          if (parse_operand(opp,opl,op,
                            DATA_OPERAND(p->content.defb->bitsize))) {
            new = new_datadef_atom(p->content.defb->bitsize,op);
            new->align = p->align;
            add_atom(0,new);
          }
          else
            syntax_error(8);  /* invalid data operand */
        }
        else if (p->type == SPACE) {
          /* parse the fill expression for this space */
          new = clone_atom(p);
          new->content.sb = new_sblock(p->content.sb->space_exp,
                                       p->content.sb->size,
                                       parse_expr_tmplab(&opp));
          new->content.sb->space = p->content.sb->space;
          add_atom(0,new);
        }
        else {
          /* parse constant data - probably a string, or a single constant */
          dblock *db;

          db = new_dblock();
          db->size = p->content.db->size;
          db->data = db->size ? mycalloc(db->size) : NULL;
          if (db->data) {
            if (*opp=='\"' || *opp=='\'') {
              dblock *strdb;

              if (strdb = parse_string(&opp,*opp,8)) {
                if (strdb->size) {
                  if (strdb->size > db->size)
                    syntax_error(24,strdb->size-db->size); /* cut last chars */
                  memcpy(db->data,strdb->data,
                         strdb->size > db->size ? db->size : strdb->size);
                  myfree(strdb->data);
                }
              }
              myfree(strdb);
            }
            else {
              taddr val = parse_constexpr(&opp);
              void *p;

              if (db->size > sizeof(taddr) && BIGENDIAN)
                p = db->data + db->size - sizeof(taddr);
              else
                p = db->data;
              setval(BIGENDIAN,p,sizeof(taddr),val);
            }
          }
          add_atom(0,new_data_atom(db,p->align));
        }
      }
      else {
        /* empty: use default values from original atom */
        add_atom(0,clone_atom(p));
      }

      s = skip(s);
      if (*s == ',')
        s++;
    }
    else if (p->type == INSTRUCTION)
      syntax_error(23);  /* skipping instruction in struct init */

    /* other atoms are silently ignored */
  }

  eol(s);
  return 1;
}


static struct {
  const char *asn_name;
  int asn_len;
} symassigns[] = {
  { NULL,0 }, { "=",1 }, { "equ",3 }, { "eq",2 }, { "set",3 }
};
enum {
  ASN_NONE=0, ASN_EQ1, ASN_EQ2, ASN_EQ3, ASN_SET, ASN_NUM
};


static char *parse_label_field(char **start,int *asntype)
{
  char *s,*name;
  int spaced;  /* potential label is spaced and needs a ':' or '=' */
  int is_special_label = 0;  /* Flag for :LABEL or ]LABEL */

  s = *start;
  if (asntype)
    *asntype = ASN_NONE;

  if (isspace((unsigned char )*s)) {
    s = skip(s);
    spaced = 1;
  }
  else
    spaced = 0;

  /* Merlin: Check for local label :LABEL (alphanumeric or numeric) */
  /* Only treat as label if not spaced (i.e., at start of line) */
  /* Supports both :LOOP (alphanumeric) and :2 (numeric) local labels */
  if (!spaced && *s == ':' && (ISIDSTART(*(s+1)) || isdigit((unsigned char)*(s+1)))) {
    /* Merlin local label: :LOOP, :DONE, :1, :2, etc. */
    if (asntype) {
      char *namestart = s + 1;
      char *nameend = namestart;
      strbuf *buf;

      /* Parse alphanumeric or numeric identifier */
      while (ISIDCHAR(*nameend) || isdigit((unsigned char)*nameend))
        nameend++;

      /* Create local label scoped to last global label (or empty if none) */
      /* Merlin allows local labels without prior global label - use empty context */
      {
        const char *global_ctx = merlin_last_global_label ? merlin_last_global_label : "";
        buf = make_local_label(0, global_ctx, strlen(global_ctx),
                               namestart, nameend - namestart);
        name = buf->str;
        is_special_label = 1;  /* Mark as special label */
      }
      *start = skip(nameend);
    }
    else
      name = s;
  }
  /* Merlin uses :LABEL for local labels, not .N */
  /* Commented out - Merlin uses :LABEL for local labels instead */
  /*
  else if (!spaced && *s == '.' && isdigit((unsigned char)*(s+1))) {
    syntax_error(26);  // "invalid directive" or add custom error
    name = NULL;
    *start = s;
  }
  */
  else if (*s == ':') {
    /* anonymous label definition (original behavior) */
    if (asntype) {
      strbuf *buf;
      char num[16];

      buf = make_local_label(0,":",1,num,sprintf(num,"%u",++anon_labno));
      name = buf->str;
    }
    else
      name = s;
    *start = skip(s+1);
  }
  else {
    *start = s;

    /* Merlin: Check for variable label ]LABEL */
    /* Variable labels are mutable and generate new unique names on each definition */
    if (!spaced && *s == ']' && ISIDSTART(*(s+1))) {
      /* Merlin variable label: ]INDEX, ]TEMP, etc. */
      char *namestart = s;  /* Include the ] prefix */
      char *nameend = s + 1;

      /* Parse alphanumeric identifier */
      while (ISIDCHAR(*nameend))
        nameend++;

      /* Only prepare variable label definition if we're actually defining (asntype != NULL).
         When asntype is NULL, we're just scanning during a false conditional skip and should
         not modify variable label state. */
      if (asntype) {
        /* Prepare variable label definition - deferred until after expression is evaluated
           This allows ]VAR = ]VAR+1 to work correctly (referencing OLD value) */
        name = prepare_varlabel_definition(namestart, nameend - namestart);
        is_special_label = 1;  /* Mark as special label */
      }
      else {
        name = namestart;  /* Return raw name for skip processing */
      }
      s = nameend;  /* Move past the label name */
    }
    else {
      name = parse_symbol(&s);
    }

    if (name==NULL && *s==current_pc_char && !ISIDCHAR(*(s+1))) {
      name = current_pc_str;
      s++;
    }

    if (name) {
      /* we need anything to authentify a spaced symbol/label, like a ':', a '=',
         or an equate or set-directive */
      s = skip(s);

      /* Merlin: Check for label suffix ':' (but not :+, :-, :LABEL, or :N numeric local) */
      if (s[0]==':' && s[1]!='+' && s[1]!='-' && !ISIDSTART(s[1]) && !isdigit((unsigned char)s[1])) {
        s = skip(s+1);
        spaced = 0;
      }

      if (*s == '=') {
        spaced = 0;
        if (asntype)
          *asntype = ASN_EQ1;
      }
      else {
        /* check for equ or set directives */
        char *p = s;
        int i;

        if (!dotdirs || (dotdirs && *p++=='.')) {
          for (i=ASN_EQ2; i<ASN_NUM; i++) {
            if (!strnicmp(p,symassigns[i].asn_name,symassigns[i].asn_len)) {
              char *q = p + symassigns[i].asn_len;

              if (isspace((unsigned char)*q)) {
                q = skip(q);
                if (!ISEOL(q))
                  break;  /* directive with space and operand confirmed */
              }
            }
          }
        }
        else
          i = ASN_NUM;

        if (i < ASN_NUM) {
          /* assignment directive confirmed - remember it */
          if (!igntrail)
            spaced = 0;
          if (asntype)
            *asntype = i;
        }
      }

      if (spaced)
        name = NULL;
      else
        *start = s;
    }
  }

  return name;
}


#ifdef STATEMENT_DELIMITER
static char *read_next_statement(void)
{
  static char *s = NULL;
  char *line,c;

  if (s == NULL) {
    s = line = read_next_line();
    if (s == NULL)
      return NULL;  /* no more lines in source */
    /* skip label field and possible statement delimiters therein */
    (void)parse_label_field(&s,NULL);
  }
  else {
    /* make the new statement start with a blank - there is no label field */
    *s = ' ';
    line = s++;
  }

  /* find next statement delimiter in line buffer */
  for (;;) {
#ifdef VASM_CPU_Z80
    unsigned char lastuc;
#endif

    c = *s;
#ifdef VASM_CPU_Z80
    /* For the Z80 ignore ' behind a letter, as it may be a register */
    lastuc = toupper((unsigned char)*(s-1));
    if ((c=='\'' && (lastuc<'A' || lastuc>'Z')) || c=='\"') {
#else
    if (c=='\'' || c=='\"') {
#endif
      s = skip_string(s,c,NULL);
    }
    else if (c==STATEMENT_DELIMITER && s[1]!='-' && s[1]!='+') {
      *s = '\0';  /* terminate the statement here temporarily */
      break;
    }
    else if (c=='\0' || c==commentchar) {
      s = NULL;  /* ignore delimiters in rest of line */
      break;
    }
    else
      s++;
  }
  return line;
}
#endif


void parse(void)
{
  char *s,*line,*inst,*labname;
  char *ext[MAX_QUALIFIERS?MAX_QUALIFIERS:1];
  char *op[MAX_OPERANDS];
  int ext_len[MAX_QUALIFIERS?MAX_QUALIFIERS:1];
  int op_len[MAX_OPERANDS];
  int ext_cnt,op_cnt,inst_len,asn_type;
  instruction *ip;

#ifdef STATEMENT_DELIMITER
  while (line = read_next_statement()) {
#else
  while (line = read_next_line()) {
#endif
    /* Merlin: Preprocess <<< to EOM for core vasm compatibility */
    /* vasm's macro reader requires alphanumeric directive names */
    {
      char *p = line;
      while (*p) {
        /* Skip leading whitespace to find <<< at start of token */
        while (isspace((unsigned char)*p))
          p++;

        /* Check if we found <<< as a standalone token */
        if (p[0] == '<' && p[1] == '<' && p[2] == '<' &&
            (isspace((unsigned char)p[3]) || p[3] == '\0' || p[3] == ';' || p[3] == '*')) {
          /* Replace <<< with EOM (pad with space to keep line length) */
          p[0] = 'E';
          p[1] = 'O';
          p[2] = 'M';
          break;  /* Only replace first occurrence per line */
        }

        /* Skip to next potential token */
        if (*p && *p != ';' && *p != '*')
          p++;
        else
          break;  /* Hit comment */
      }
    }

    /* Merlin: * in column 1 is always a comment */
    if (parse_end || *line=='*')
      continue;


    if (!cond_state()) {
      /* skip source until ELSE or ENDIF */
      int idx;

      s = line;
      (void)parse_label_field(&s,NULL);
      idx = check_directive(&s);
      if (idx >= 0) {
        /* Track nested conditionals - check for IF-type and DO directives */
        if (!strncmp(directives[idx].name,"if",2) ||
            !strcmp(directives[idx].name,"do"))
          cond_skipif();
        else if (directives[idx].func == handle_else)
          cond_else();
        else if (directives[idx].func == handle_endif ||
                 directives[idx].func == handle_fin)
          cond_endif();
      }
      continue;
    }

    s = line;
    if (labname = parse_label_field(&s,&asn_type)) {
      /* we have found a global or local label, or current-pc character */
      symbol *label;

      if (asn_type) {
        /* proceed to operand */
        if (dotdirs && asn_type!=ASN_EQ1)
          s++;  /* skip the '.' unless we have a '=' */
        s = skip(s+symassigns[asn_type].asn_len);

        if (asn_type < ASN_SET) {
          /* EQU or '=' */
          if (*labname == current_pc_char) {
            handle_org(s);
            finalize_varlabel();  /* Finalize any pending variable label */
            continue;
          }
          else
            label = new_equate(labname,parse_expr_tmplab(&s));
        }
        else if (asn_type == ASN_SET) {
          /* SET allows redefinitions */
          if (*labname == current_pc_char)
            syntax_error(10);  /* identifier expected */
          else
            label = new_abs(labname,parse_expr_tmplab(&s));
        }
        else
          ierror(0);

        /* Finalize any pending variable label definition (after expression is evaluated) */
        finalize_varlabel();
      }
      else if (!strnicmp(s,macroname+!dotdirs,3+dotdirs) &&
               (isspace((unsigned char)*(s+3+dotdirs)) ||
                *(s+3+dotdirs)=='\0') ||
               !strnicmp(s,macroname+!dotdirs,5+dotdirs) &&
               (isspace((unsigned char)*(s+5+dotdirs)) ||
                *(s+5+dotdirs)=='\0')) {
        /* macro definition */
        char *params;
        strbuf *buf;

        params = skip(s + (tolower((unsigned char)*(s+3+dotdirs))=='r'?
                           5+dotdirs:3+dotdirs));
        s = line;
        if (!(buf = parse_identifier(0,&s)))
          ierror(0);
        new_macro(buf->str,dotdirs?dmacro_dirlist:macro_dirlist,
                  dotdirs?dendm_dirlist:endm_dirlist,
                  ISEOL(params)?NULL:params);
        continue;
      }
#ifdef PARSE_CPU_LABEL
      else if (!PARSE_CPU_LABEL(labname,&s)) {
#else
      else {
#endif
        /* it's just a label */
        int is_varlabel = (pending_varlabel != NULL);  /* Track before finalizing */

        label = new_labsym(current_section,labname);
        add_atom(0,new_label_atom(label));

        /* Finalize any pending variable label definition */
        finalize_varlabel();

        /* Merlin: Track global labels for :LABEL local label scoping */
        /* A global label doesn't start with a space or : (local labels) */
        /* and is not a variable label (which have internal names like unid_N) */
        if (labname[0] != ' ' && labname[0] != ':' && !is_varlabel) {
          /* This is a global label - update context for :LABEL scoping */
          if (merlin_last_global_label)
            myfree((void *)merlin_last_global_label);
          merlin_last_global_label = mystrdup(labname);
        }
      }

      /* Auto-export disabled in Merlin syntax */
    }
    if (ISEOL(s))
      continue;

    /* check for directives first */
    s = parse_cpu_special(s);
    if (ISEOL(s))
      continue;

    if (*s==current_pc_char && *(s+1)=='=') {
      /* "*=" org directive can stand together with a label: lab *= *+1 */
      handle_org(skip(s+2));
      continue;
    }

    if (handle_directive(s))
      continue;
    s = skip(s);
    if (ISEOL(s))
      continue;

    /* Merlin: Check for macro invocation with >>> prefix */
    if (*s == '>' && *(s+1) == '>' && *(s+2) == '>') {
      char *name_start;
      int name_len;
      macro *m;

      s += 3;  /* skip the >>> prefix */
      s = skip(s);  /* skip whitespace after >>> */
      name_start = s;

      /* Read macro name */
      if (!ISIDSTART(*s)) {
        syntax_error(10);  /* identifier expected after >>> */
        continue;
      }
      while (ISIDCHAR(*s))
        s++;
      name_len = s - name_start;

      /* Look up macro */
      m = find_macro(name_start, name_len);
      if (m != NULL) {
        /* Found macro - execute it */
        /* Merlin allows various separators (;,/ etc.) so don't check for space */
        s = skip(s);

        if (execute_macro(name_start, name_len, NULL, NULL, 0, s))
          continue;
      }
      else {
        /* Macro not found - error */
        syntax_error(42);  /* undefined macro */
        continue;
      }
    }

    /* read mnemonic name */
    inst = s;
    ext_cnt = 0;
    if (!ISIDSTART(*s)) {
      syntax_error(10);  /* identifier expected */
      continue;
    }
#if MAX_QUALIFIERS==0
    while (*s && !isspace((unsigned char)*s))
      s++;
    inst_len = s - inst;
#else
    s = parse_instruction(s,&inst_len,ext,ext_len,&ext_cnt);
#endif
    if (!isspace((unsigned char)*s) && *s!='\0')
      syntax_error(2);  /* no space before operands */
    s = skip(s);

    if (execute_macro(inst,inst_len,ext,ext_len,ext_cnt,s))
      continue;
    if (execute_struct(inst,inst_len,s))
      continue;

    /* read operands, terminated by comma or blank (unless in parentheses) */
    op_cnt = 0;
    while (!ISEOL(s) && op_cnt<MAX_OPERANDS) {
      op[op_cnt] = s;
      s = skip_operand(1,s);
      op_len[op_cnt] = oplen(s,op[op_cnt]);
#if !ALLOW_EMPTY_OPS
      if (op_len[op_cnt] <= 0)
        syntax_error(5);  /* missing operand */
      else
#endif
        op_cnt++;

      if (igntrail) {
        if (*s != ',')
          break;
        s++;
      }
      else {
        s = skip(s);
        if (OPERSEP_COMMA) {
          if (*s == ',')
            s = skip(s+1);
          else if (!(OPERSEP_BLANK))
            break;
        }
      }
    }
    eol(s);

    /* Merlin compatibility: REP and SEP only take immediate operands,
       so they don't require the # prefix. Add it if missing.
       Also track the M/X flag changes for 65816 16-bit mode. */
    if (op_cnt == 1 && inst_len == 3 &&
        (!strnicmp(inst, "rep", 3) || !strnicmp(inst, "sep", 3))) {
      int is_rep = !strnicmp(inst, "rep", 3);
      char *opval = op[0];
      taddr flags;

      if (*op[0] != '#') {
        /* Need to prefix operand with # - create modified operand string */
        static char immed_op[64];
        size_t olen = op_len[0];
        if (olen < sizeof(immed_op) - 2) {
          immed_op[0] = '#';
          memcpy(immed_op + 1, op[0], olen);
          immed_op[olen + 1] = '\0';
          op[0] = immed_op;
          op_len[0] = olen + 1;
        }
      }
      else {
        opval++;  /* skip # */
      }

      /* Try to parse the operand as a constant to track M/X flag changes.
         65816 P register: bit 5 = M (accumulator), bit 4 = X (index)
         REP clears bits (0 = 16-bit), SEP sets bits (1 = 8-bit) */
      if (cpu_type & WDC65816) {
        char *p = opval;
        int cur_asize, cur_xsize, new_asize, new_xsize;

        flags = parse_constexpr(&p);
        get_65816_sizes(&cur_asize, &cur_xsize);

        if (is_rep) {
          /* REP clears bits: if bit is in flags, mode becomes 16-bit */
          new_asize = (flags & 0x20) ? 16 : cur_asize;
          new_xsize = (flags & 0x10) ? 16 : cur_xsize;
        }
        else {
          /* SEP sets bits: if bit is in flags, mode becomes 8-bit */
          new_asize = (flags & 0x20) ? 8 : cur_asize;
          new_xsize = (flags & 0x10) ? 8 : cur_xsize;
        }
        set_65816_sizes(new_asize, new_xsize);
      }
    }

    ip = new_inst(inst,inst_len,op_cnt,op,op_len);

#if MAX_QUALIFIERS>0
    if (ip) {
      int i;

      for (i=0; i<ext_cnt; i++)
        ip->qualifiers[i] = cnvstr(ext[i],ext_len[i]);
      for(; i<MAX_QUALIFIERS; i++)
        ip->qualifiers[i] = NULL;
    }
#endif

    if (ip) {
#if MAX_OPERANDS>0
      if (!igntrail && ip->op[0]==NULL && op_cnt!=0)
        syntax_error(6);  /* mnemonic without operands has tokens in op.field */
#endif
      add_atom(0,new_inst_atom(ip));
    }
  }

  cond_check();
  if (dsect_active)
    syntax_error(23);  /* missing .ED */
}


/* parse next macro argument */
char *parse_macro_arg(struct macro *m,char *s,
                      struct namelen *param,struct namelen *arg)
{
  arg->len = 0;  /* cannot select specific named arguments */
  param->name = s;
  s = skip_operand(0,s);
  param->len = s - param->name;
  return s;
}


/* expands arguments and special escape codes into macro context */
int expand_macro(source *src,char **line,char *d,int dlen)
{
  int nc = 0;
  int n;
  char *s = *line;
  char *end;
  char escape_char;

  escape_char = *s++;  /* Get escape character (\\ or ]) */

  /* Merlin: Support both \ (vasm standard) and ] (Merlin standard) escapes */
  if (escape_char == ']') {
    /* Merlin macro parameter notation: ]0 (count), ]1-]8 (params), ]] (escape) */

    if (*s == ']') {
      /* ]]: escape to single ] */
      if (dlen >= 1) {
        *d++ = ']';
        s++;
        nc = 1;
      }
      else
        nc = -1;
    }
    else if (*s == '0') {
      /* ]0: insert parameter count (Merlin uses ]0, not ]#) */
      if (src && src->num_params >= 0) {
        if (dlen > 3) {
          nc = sprintf(d, "%d", src->num_params);
          s++;
        }
        else
          nc = -1;
      }
      else {
        nc = 0;  /* No parameters */
        s++;
      }
    }
    else if (isdigit((unsigned char)*s)) {
      /* ]1-]8: insert macro parameter (Merlin supports 1-8, not 1-9) */
      int pnum = *s - '1';  /* Convert ]1-]8 to 0-7 */
      if (pnum > 7) {
        syntax_error(29);  /* macro parameter out of range (]0-]8 only) */
        nc = 0;
      }
      else {
        nc = copy_macro_param(src, pnum, d, dlen);
      }
      s++;
    }
    else {
      /* Not a valid Merlin escape - output ] and continue */
      if (dlen >= 1) {
        *d = ']';
        nc = 1;
        s--;  /* Back up to reprocess this character */
      }
      else
        nc = -1;
    }

    if (nc >= 0)
      *line = s;

    return nc;
  }

  if (escape_char == '\\') {
    /* Standard vasm macro expansion */

    if (*s == '\\') {
      if (dlen >= 1) {
        *d++ = *s++;
        if (esc_sequences) {
          if (dlen >= 2) {
            *d++ = '\\';  /* make it a double \ again */
            nc = 2;
          }
          else
            nc = -1;
        }
        else
          nc = 1;
      }
      else
        nc = -1;
    }

    else if (*s == '@') {
      /* \@: insert a unique id */
      if (dlen > 7) {
        nc = sprintf(d,"_%06lu",src->id);
        s++;
      }
      else
        nc = -1;
    }
    else if (*s=='(' && *(s+1)==')') {
      /* \() is just skipped, useful to terminate named macro parameters */
      nc = 0;
      s += 2;
    }
    else if (*s == '<') {
      /* \<symbol> : insert absolute unsigned symbol value */
      char *name;
      symbol *sym;
      taddr val;

      s++;
      if (name = parse_symbol(&s)) {
        if ((sym = find_symbol(name)) && sym->type==EXPRESSION) {
          if (eval_expr(sym->expr,&val,NULL,0)) {
            if (dlen > 9)
              nc = sprintf(d,"%lu",(unsigned long)(uint32_t)val);
            else
              nc = -1;
          }
        }
        if (*s++ != '>') {
          syntax_error(11);  /* invalid numeric expansion */
          return 0;
        }
      }
      else {
        syntax_error(10);  /* identifier expected */
        return 0;
      }
    }
    else if (isdigit((unsigned char)*s)) {
      /* \1..\9,\0 : insert macro parameter 1..9,10 */
      nc = copy_macro_param(src,*s=='0'?0:*s-'1',d,dlen);
      s++;
    }
    else if ((end = skip_identifier(s)) != NULL) {
      if ((n = find_macarg_name(src,s,end-s)) >= 0) {
        /* \argname: insert named macro parameter n */
        nc = copy_macro_param(src,n,d,dlen);
        s = end;
      }
    }

    if (nc >= 0)
      *line = s;  /* update line pointer when expansion took place */
  }

  return nc;  /* number of chars written to line buffer, -1: out of space */
}


static int intel_suffix(char *s)
/* check for constants with h, d, o, q or b suffix */
{
  int base,lastbase;
  char c;

  base = 2;
  while (isxdigit((unsigned char)*s)) {
    lastbase = base;
    switch (base) {
      case 2:
        if (*s <= '1') break;
        base = 8;
      case 8:
        if (*s <= '7') break;
        base = 10;
      case 10:
        if (*s <= '9') break;
        base = 16;
    }
    s++;
  }

  c = tolower((unsigned char)*s);
  if (c == 'h')
    return 16;
  if ((c=='o' || c=='q') && base<=8)
    return 8;

  c = tolower((unsigned char)*(s-1));
  if (c=='d' && lastbase<=10)
    return 10;
  if (c=='b' && lastbase<=2)
    return 2;

  return 0;
}


char *const_prefix(char *s,int *base)
{
  if (isdigit((unsigned char)*s)) {
    if (!nointelsuffix && (*base = intel_suffix(s)))
      return s;
    if (!nocprefix) {
      if (*s == '0') {
        if (s[1]=='x' || s[1]=='X'){
          *base = 16;
          return s+2;
        }
        if (s[1]=='b' || s[1]=='B'){
          *base = 2;
          return s+2;
        }
        *base = 8;
        return s;
      }
      else if (s[1]=='#' && *s>='2' && *s<='9') {
        *base = *s & 0xf;
        return s+2;
      }
    }
    *base = 10;
    return s;
  }

  if (*s=='$') {
    if (isxdigit((unsigned char)s[1])) {
      *base = 16;
      return s+1;
    }
#if !defined(VASM_CPU_Z80)
    else if (isxdigit((unsigned char)s[2]) && s[1]=='-') {
      /* requires BROKEN_HEXCONST in expr.c */
      *base = 16;
      return s+2;
    }
    else {
      /* just skip the '$' and continue parsing, for example $'A' */
      *base = 0;
      return s+1;
    }
#endif
  }
#if defined(VASM_CPU_Z80)
  if ((*s=='&' || *s=='#') && isxdigit((unsigned char)s[1])) {
    *base = 16;
    return s+1;
  }
#endif
  if (*s=='@') {
#if defined(VASM_CPU_Z80)
    *base = 2;
#else
    *base = 8;
#endif
    return s+1;
  }
  if (*s == '%') {
    *base = 2;
    return s+1;
  }
  *base = 0;
  return s;
}


char *const_suffix(char *start,char *end)
{
  if (intel_suffix(start))
    return end+1;

  return end;
}


static char *skip_local(char *p)
{
  if (ISIDSTART(*p) || isdigit((unsigned char)*p)) {  /* may start with digit */
    p++;
    while (ISIDCHAR(*p))
      p++;
  }
  else
    p = NULL;

  return p;
}


strbuf *get_local_label(int n,char **start)
/* Merlin: Handle three-tier labels: global, :LABEL local, ]LABEL variable */
{
  char *s;
  strbuf *name;

  name = NULL;
  s = *start;

  /* Merlin: Check for local label reference :LABEL (alphanumeric or numeric) */
  if (*s == ':' && (ISIDSTART(*(s+1)) || isdigit((unsigned char)*(s+1)))) {
    char *namestart = s + 1;
    char *nameend = namestart;

    /* Parse alphanumeric or numeric identifier */
    while (ISIDCHAR(*nameend) || isdigit((unsigned char)*nameend))
      nameend++;

    /* Create local label scoped to last global label (or empty if none) */
    /* Merlin allows local labels without prior global label - use empty context */
    {
      const char *global_ctx = merlin_last_global_label ? merlin_last_global_label : "";
      name = make_local_label(n, global_ctx, strlen(global_ctx),
                             namestart, nameend - namestart);
    }
    *start = skip(nameend);
  }
  /* Merlin: Check for macro parameter ]0-]8 */
  else if (*s == ']' && isdigit((unsigned char)*(s+1))) {
    int pnum = *(s+1) - '0';
    char *p;

    s += 2;  /* skip ]digit */
    *start = skip(s);

    /* Check if we're in a macro context */
    if (cur_src && (cur_src->srcfile==NULL || cur_src->macro)) {
      if (pnum == 0) {
        /* ]0 = parameter count */
        char countstr[16];
        snprintf(countstr, sizeof(countstr), "%d", cur_src->num_params);
        name = make_local_label(n, "", 0, countstr, strlen(countstr));
      }
      else if (pnum >= 1 && pnum <= MAXMACPARAMS) {
        /* ]1-]8 = macro parameters */
        /* Get the actual parameter text and use it as the label */
        if (pnum <= cur_src->num_params && cur_src->param[pnum-1]) {
          p = cur_src->param[pnum-1];
          /* Skip any leading whitespace */
          while (isspace((unsigned char)*p)) p++;
          /* Copy parameter text up to whitespace or end */
          name = make_local_label(n, "", 0, p, strcspn(p, " \t\r\n,"));
        }
        else {
          syntax_error(28);  /* macro parameter out of range */
          return NULL;
        }
      }
    }
    else {
      syntax_error(29);  /* macro parameter outside macro context */
      return NULL;
    }
  }
  /* Merlin: Check for variable label reference ]LABEL */
  else if (*s == ']' && ISIDSTART(*(s+1))) {
    char *namestart = s;  /* Include the ] prefix in lookup */
    char *nameend = s + 1;
    char *unique;
    static strbuf varbuf[EXPBUFNO+1];
    char *p;
    int ulen;

    /* Parse alphanumeric identifier */
    while (ISIDCHAR(*nameend))
      nameend++;

    /* Look up the variable label in our hash table.
       For forward references, get_varlabel_unique_name creates a placeholder. */
    unique = get_varlabel_unique_name(namestart, nameend - namestart);
    /* Create strbuf with just the unique name (not a local label) */
    ulen = strlen(unique);
    p = strbuf_alloc(&varbuf[n], ulen+1);
    memcpy(p, unique, ulen);
    p[ulen] = '\0';
    varbuf[n].len = ulen;
    name = &varbuf[n];
    *start = skip(nameend);
  }
  /* Merlin uses :LABEL for local labels, not .N */
  /* Original anonymous label reference :+, :- */
  else if (*s == ':' && (*(s+1)=='+' || *(s+1)=='-')) {
    unsigned refno = (*(s+1)=='+')?anon_labno+1:anon_labno;
    char refnostr[16];

    s++;  /* skip : */
    while (*s=='+' || *s=='-') {
      if (*s++ == '+')
        refno++;  /* next anonymous label */
      else
        refno--;  /* previous anonymous label */
    }
    name = make_local_label(n,":",1,refnostr,sprintf(refnostr,"%u",refno));
    *start = skip(s);
  }

  return name;
}


int init_syntax(void)
{
  size_t i;
  hashdata data;

  /* Merlin: Directives have NO prefix (directives like ORG, MAC, DA have no dot) */
  dotdirs = 0;

  /* Merlin: Disable C-style octal prefix (leading 0 does NOT mean octal) */
  nocprefix = 1;

  /* Merlin: Enable tolerant parsing - allow informal comments after operands */
  allow_trailing_comments = 1;

  dirhash = new_hashtable(0x1000);
  for (i=0; i<dir_cnt; i++) {
    data.idx = i;
    add_hashentry(dirhash,directives[i].name,data,1);  /* case insensitive */
  }
  if (debug && dirhash->collisions)
    fprintf(stderr,"*** %d directive collisions!!\n",dirhash->collisions);

  cond_init();
  set_internal_abs(REPTNSYM,-1); /* reserve the REPTN symbol */
  current_pc_char = '*';
  current_pc_str[0] = current_pc_char;
  current_pc_str[1] = 0;
  charsperexp = 1;  /* allows 'x and "x expressions without 2nd quote-char */

  if (orgmode != ~0)
    set_section(new_org(orgmode));
  return 1;
}


int syntax_defsect(void)
{
  return 0;  /* defaults to .text */
}


int syntax_args(char *p)
{
  if (!strcmp(p,"-dotdir"))
    dotdirs = 1;
  else if (!strcmp(p,"-autoexp"))
    return 0;  /* Not supported in Merlin syntax */
  else if (!strncmp(p,"-org=",5))
    orgmode = atoi(p+5);
  else if (OPERSEP_COMMA && !strcmp(p,"-i"))
    igntrail = 1;
  else if (!strcmp(p,"-noc"))
    nocprefix = 1;
  else if (!strcmp(p,"-noi"))
    nointelsuffix = 1;
  else if (!strcmp(p,"-ast"))
    astcomment = 1;
  else if (!strcmp(p,"-ldots"))
    dot_idchar = 1;
  else if (!strcmp(p,"-sect"))
    sect_directives = 1;
  else
    return 0;

  return 1;
}


/* Accessor functions for Merlin directive metadata */
/* These allow output modules to query values set by TYP, AUX, SAV, CYC directives */

int get_merlin_file_type(void)
{
  return merlin_file_type;
}

int get_merlin_aux_type(void)
{
  return merlin_aux_type;
}

const char *get_merlin_output_filename(void)
{
  return merlin_output_filename[0] ? merlin_output_filename : NULL;
}

int get_merlin_cycle_counting(void)
{
  return merlin_cycle_counting;
}

/* USR directive accessor functions for RW18 disk format (crackle compatibility) */
int get_merlin_usr_valid(void)
{
  return merlin_usr_valid;
}

int get_merlin_usr_side(void)
{
  return merlin_usr_side;
}

int get_merlin_usr_track(void)
{
  return merlin_usr_track;
}

int get_merlin_usr_offset(void)
{
  return merlin_usr_offset;
}
