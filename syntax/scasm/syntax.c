/* syntax.c  SCASM 3.0 syntax module for vasm */
/* (c) in 2002-2025 by Frank Wille */
/* SCASM 3.0 syntax adaptation (c) 2025 Bryan Woodruff, Cortexa LLC*/

#include "vasm.h"

/* The SCASM syntax module parses S-C Macro Assembler (SCASM) syntax
   as used in the A2osX project and other Apple II 6502/65816 code.

   Key features:
   - Comments: * in column 1, ; inline
   - Three-tier labels: global, private (:prefix), local (.prefix numeric)
   - Directives start with . (e.g., .OR, .DA, .MA)
   - Macros: >NAME or _NAME invocation, ]1-]9 parameters
   - Flexible string delimiters
*/

const char *syntax_copyright="vasm SCASM 3.0 syntax module 1.0 (c) 2025 Bryan Woodruff, Cortexa LLC";
hashtable *dirhash;
int dotdirs;

static char textname[]=".text",textattr[]="acrx";
static char dataname[]=".data",dataattr[]="adrw";
static char rodataname[]=".rodata",rodataattr[]="adr";
static char bssname[]=".bss",bssattr[]="aurw";
static char zeroname[]=".zero",zeroattr[]="aurwz";

char commentchar=';';  /* SCASM also allows * in column 1 (handled in parse loop) */

/* SCASM macro directives: .MA and .EM */
static char macroname[] = ".ma";
static char endmname[] = ".em";
static char endrname[] = ".endr";  /* Not standard SCASM but kept for compatibility */
static char reptname[] = ".rept";  /* Not standard SCASM but kept for compatibility */
static char repeatname[] = ".repeat";
static struct namelen macro_dirlist[] = {
  { 2,&macroname[1] }, { 0,0 }  /* ma */
};
static struct namelen endm_dirlist[] = {
  { 2,&endmname[1] }, { 4,"endm" }, { 6,"endmac" }, { 8,"endmacro" }, { 0,0 }
};
static struct namelen rept_dirlist[] = {
  { 4,&reptname[1] }, { 6,&repeatname[1] }, { 2,"lu" }, { 0,0 }
};
static struct namelen endr_dirlist[] = {
  { 4,&endrname[1] }, { 6,&endrname[1] }, { 9,&endrname[1] }, { 4,"endu" }, { 0,0 }
};
static struct namelen dmacro_dirlist[] = {
  { 3,&macroname[0] }, { 0,0 }  /* .ma */
};
static struct namelen dendm_dirlist[] = {
  { 3,&endmname[0] }, { 5,".endm" }, { 7,".endmac" }, { 9,".endmacro" }, { 0,0 }
};
static struct namelen drept_dirlist[] = {
  { 5,&reptname[0] }, { 7,&repeatname[0] }, { 3,".lu" }, { 0,0 }
};
static struct namelen dendr_dirlist[] = {
  { 5,&endrname[0] }, { 7,&endrname[0] }, { 10,&endrname[0] }, { 5,".endu" }, { 0,0 }
};

static char local_modif_name[] = "_";  /* ._ for abyte directive */
static char current_pc_str[2];

static int autoexport,parse_end,nocprefix,nointelsuffix;
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

/* SCASM three-tier label system */
static const char *scmasm_last_global_label = NULL;  /* Last global label for .N local labels */
static int scmasm_private_context = 0;                /* Context counter for :N private labels */

/* SCASM .AC compression state */
static char ac_table1[16];   /* Table 1: single nibble encoding (nibbles 1-15) */
static char ac_table2[16];   /* Table 2: two nibble encoding (0, then 1-15) */
static char ac_table3[16];   /* Table 3: three nibble encoding (0, 0, then 0-15) */
static int ac_table1_len;    /* Length of table 1 */
static int ac_table2_len;    /* Length of table 2 */
static int ac_table3_len;    /* Length of table 3 */
static int ac_nibble_flag;   /* 0 = even position (high nibble), 1 = odd (low nibble) */
static unsigned char ac_pending_nibble; /* Saved high nibble when at odd position */

int igntrail;  /* ignore everything after a blank in the operand field */


int isidchar(char c)
{
  /* SCASM: periods ARE allowed in identifiers (e.g., VERSION.HI, VERSION.LO) */
  /* Directives are distinguished by context (after whitespace or at line start) */
  /* Underscores and periods allowed in identifiers */
  if (isalnum((unsigned char)c) || c=='_' || c=='.')
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
    /* SCASM: With igntrail enabled, trailing content is always treated as comments.
       Don't warn about "garbage at end of line" since it's valid SCASM syntax. */
    return;
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
  else if (isspace((unsigned char)*s) || *s==commentchar) {
    /* SCASM igntrail mode: Whitespace terminates expression parsing.
       This matches SCASM's behavior where space is an expression terminator
       (see OPERATOR.CHARS which includes space as a terminator). */
    *s = '\0';  /* terminate expression at whitespace */
  }
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


/* Called when a macro is about to be executed.
   SCASM: Increment private label context so that each macro invocation
   gets a unique scope for its :N private labels. */
void my_exec_macro(source *src)
{
  scmasm_private_context++;
}


#define handle_data(a,b) handle_data_mod(a,b,NULL)


/* Helper function: Apply high bit based on delimiter
   SCASM rule: delimiters < apostrophe (0x27) set high bit */
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
      char delim = *s;
      db = parse_string(&opstart,*s,size);
      s = opstart;
      if (db) {
        apply_delimiter_highbit(db, delim);  /* Apply delimiter rule */
      }
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
      /* SCASM igntrail: treat trailing content as comment */
      if (!igntrail) {
        syntax_error(9);  /* , expected */
        return;
      }
      break;
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
  taddr val;

  cnt = parse_expr_tmplab(&s);
  s = skip(s);
  if (*s == ',') {
    s = skip(s+1);
    fill = parse_expr_tmplab(&s);
  }

  /* SCASM: Check if space count can be evaluated and is valid */
  /* If negative or zero, skip space allocation (common with .BS $ADDR-*) */
  if (cnt && eval_expr(cnt,&val,current_section,current_section->pc)) {
    if (val <= 0) {
      /* Space is negative or zero - skip allocation silently */
      /* This happens when .BS $TARGET-* and PC is already past TARGET */
      eol(s);
      return;
    }
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
  /* SCASM .DA directive: process each expression in comma-separated list.
     Each expression can have its own prefix:
     #expr = low byte only (1 byte)
     /expr = high byte only (1 byte)
     expr  = full address (bytespertaddr bytes, typically 2)
  */
#if BITSPERBYTE == 8
  s = skip(s);
  if (ISEOL(s)) {
    handle_fixedspc(s,bytespertaddr);
    return;
  }

  for (;;) {
    char *opstart;
    operand *op;
    int size;

    s = skip(s);
    if (ISEOL(s))
      break;

    /* Check for # or / prefix on this expression */
    if (*s == '#') {
      s++;  /* skip the # */
      size = 8;  /* low byte only */
    }
    else if (*s == '/') {
      /* Keep / for expression parser (high byte operator) */
      size = 8;  /* high byte only */
    }
    else {
      size = bytespertaddr * BITSPERBYTE;  /* full address */
    }

    opstart = s;
    op = new_operand();
    s = skip_operand(0, s);
    if (parse_operand(opstart, s - opstart, op, DATA_OPERAND(size))) {
      atom *a = new_datadef_atom(OPSZ_BITS(size), op);
      a->align = 1;
      add_atom(0, a);
    }
    else {
      syntax_error(8);  /* invalid data operand */
      return;
    }

    s = skip(s);
    if (*s == ',') {
      s++;
    }
    else if (*s == commentchar || ISEOL(s)) {
      break;
    }
    else {
      /* SCASM igntrail: treat trailing content as comment */
      if (!igntrail) {
        syntax_error(9);  /* , expected */
        return;
      }
      break;
    }
  }

  eol(s);
#else
  handle_data(s,bytespertaddr*BITSPERBYTE);
#endif
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


/* SCASM .PG directive - page eject for listing output only.
   In the original S-C Assembler, .PG outputs a form feed ($0C) to the
   listing and increments the page counter. It does NOT affect code
   placement or memory alignment. This is a no-op for code generation. */
static void handle_pg(char *s)
{
  eol(s);
}


static void handle_even(char *s)
{
  do_alignment(2,number_expr(0));
  eol(s);
}


static void handle_spc8(char *s)
{
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


static void handle_spc64(char *s)
{
  handle_space(s,64);
}


static void handle_ascii(char *s)
{
  handle_data(s,8);
}


/* SCASM .HS directive - hex string */
static void handle_hs(char *s)
{
  char *p;
  unsigned char *hexdata;
  int len = 0, maxlen = 256;
  dblock *db;
  int hex_digit_count = 0;
  int first_byte_done = 0;

  hexdata = mymalloc(maxlen);
  s = skip(s);

  /* SCASM: Count total hex digits first to handle odd-length padding */
  p = s;
  while (*p && !ISEOL(p)) {
    while (*p && (isspace((unsigned char)*p) || *p == '.' || *p == ','))
      p++;
    if (ISEOL(p) || !isxdigit((unsigned char)*p))
      break;
    hex_digit_count++;
    p++;
  }

  /* If odd number of digits, emit first byte with leading zero */
  if (hex_digit_count % 2 == 1) {
    int first_nibble;

    /* Skip whitespace to get to first hex digit */
    while (*s && (isspace((unsigned char)*s) || *s == '.' || *s == ','))
      s++;

    /* Get first nibble */
    first_nibble = isdigit((unsigned char)*s) ? *s - '0' :
                   tolower((unsigned char)*s) - 'a' + 10;
    s++;

    /* Emit byte with leading zero: 0x0N */
    hexdata[len++] = first_nibble;
    first_byte_done = 1;
  }

  while (*s && !ISEOL(s)) {
    int nibble1, nibble2;

    /* Skip whitespace and optional periods (SCASM allows periods as separators) */
    while (*s && (isspace((unsigned char)*s) || *s == '.' || *s == ','))
      s++;

    if (ISEOL(s))
      break;

    /* SCASM: Check if next character is hex digit - if not, stop parsing */
    /* This allows inline comments like ".HS 2C    SKIP TWO BYTES" */
    if (!isxdigit((unsigned char)*s)) {
      /* Not a hex digit - treat rest of line as comment */
      break;
    }

    /* Get first hex digit */
    nibble1 = isdigit((unsigned char)*s) ? *s - '0' :
              tolower((unsigned char)*s) - 'a' + 10;
    s++;

    /* Skip separators between nibbles */
    while (*s && (isspace((unsigned char)*s) || *s == '.' || *s == ','))
      s++;

    /* Get second hex digit - should always exist after odd padding above */
    if (isxdigit((unsigned char)*s)) {
      nibble2 = isdigit((unsigned char)*s) ? *s - '0' :
                tolower((unsigned char)*s) - 'a' + 10;
      s++;
    } else {
      /* This shouldn't happen after proper odd-padding, but handle it */
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

  /* SCASM: Don't check eol() when igntrail is enabled */
  /* We may have stopped parsing hex at a comment */
  if (!igntrail)
    eol(s);
}


static void handle_string(char *s)
{
  handle_data(s,8);
  add_atom(0,new_space_atom(number_expr(1),1,0));  /* terminating zero */
}


/* SCASM .AS directive - ASCII string with flexible delimiters */
static void handle_as(char *s)
{
  dblock *db;
  char *opstart;

  for (;;) {
    int force_highbit = 0;
    opstart = s;
    s = skip(s);
    if (ISEOL(s))
      break;

    /* SCASM: Check for optional '-' flag (FORCE high-bit setting on all chars) */
    if (*s == '-') {
      force_highbit = 1;
      s++;
      opstart = s;  /* update opstart past the flag */
    }

    /* Parse string with any non-whitespace delimiter */
    if (*s && !isspace((unsigned char)*s)) {
      char delim = *s;
      db = parse_string(&s,delim,8);

      /* parse_string returns NULL for single-character strings.
         Handle this case explicitly. */
      if (!db) {
        char *p = opstart;
        p = skip(p);
        if (*p == delim) {
          p++;  /* skip opening delimiter */
          if (*p && !ISEOL(p) && p[1] == delim) {
            /* Single character between delimiters */
            db = new_dblock();
            db->size = 1;
            db->data = mymalloc(1);
            db->data[0] = *p;
            s = p + 2;  /* skip char and closing delimiter */
          }
        }
      }

      if (db) {
        if (force_highbit) {
          /* Force high bit on all characters regardless of delimiter */
          size_t i;
          for (i = 0; i < db->size; i++) {
            db->data[i] |= 0x80;
          }
        } else {
          apply_delimiter_highbit(db, delim);
        }
        add_atom(0,new_data_atom(db,1));
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
      if (!igntrail) {
        syntax_error(9);  /* , expected */
        return;
      }
      break;  /* Exit loop when igntrail is true and trailing content present */
    }
  }
  eol(s);
}


/* SCASM .AC directive - ASCII Compression

   This implements SCASM's nibble-based text compression:
   - .AC 0              Initialize compression state (reset nibble position)
   - .AC 1"chars"       Define table 1 (single nibble encoding, nibbles 1-15)
   - .AC 2"chars"       Define table 2 (two nibble encoding: 0, then 1-15)
   - .AC 3"chars"       Define table 3 (three nibble encoding: 0, 0, then 0-15)
   - .AC /message%/     Compress message using defined tables

   Compression scheme:
   - Characters in table 1 at index i emit nibble (i+1)
   - Characters in table 2 at index i emit nibbles 0, (i+1)
   - Characters in table 3 at index i emit nibbles 0, 0, i
   - % (end marker) must be last char in table 1, emits nibble 15
   - Nibbles are packed two per byte (high nibble first)
   - At end of message, any pending nibble is flushed with 0 padding
*/
static void handle_ac(char *s)
{
  dblock *db;
  unsigned char *outbuf;
  int outlen = 0, outmax = 256;
  int table_num = -1;  /* -1 = compress mode, 0-3 = table definition/init */
  char delim;
  size_t i;

  s = skip(s);
  if (ISEOL(s))
    return;

  /* Check for numeric prefix (table definition or init) */
  if (isdigit((unsigned char)*s)) {
    table_num = *s - '0';
    s++;
    s = skip(s);

    /* .AC 0 - Initialize compression state
       Note: SCASM igntrail mode allows inline comments after operands */
    if (table_num == 0) {
      ac_nibble_flag = 0;
      ac_pending_nibble = 0;
      eol(s);
      return;
    }

    /* .AC 1/2/3"..." - Define compression table */
    if (table_num >= 1 && table_num <= 3 && *s && !isspace((unsigned char)*s)) {
      delim = *s;
      db = parse_string(&s, delim, 8);
      if (db && db->size > 0) {
        int maxlen = (table_num == 3) ? 16 : 15;  /* Table 3 can have 16 entries */
        int copylen = (db->size > maxlen) ? maxlen : db->size;

        switch (table_num) {
          case 1:
            memcpy(ac_table1, db->data, copylen);
            ac_table1_len = copylen;
            break;
          case 2:
            memcpy(ac_table2, db->data, copylen);
            ac_table2_len = copylen;
            break;
          case 3:
            memcpy(ac_table3, db->data, copylen);
            ac_table3_len = copylen;
            break;
        }
      }
      eol(s);
      return;
    }
  }

  /* Compression mode: .AC /message/ or .AC "message" */
  if (!*s || isspace((unsigned char)*s)) {
    if (!ISEOL(s))
      syntax_error(30);  /* missing closing delimiter for string */
    return;
  }

  delim = *s;
  db = parse_string(&s, delim, 8);

  /* parse_string returns NULL for single-character strings.
     For .AC compression, we need to handle single characters too. */
  if (!db) {
    char *p = s + 1;  /* point past opening delimiter */
    /* Check for single character followed by closing delimiter */
    if (*p && p[1] == delim) {
      db = new_dblock();
      db->size = 1;
      db->data = mymalloc(1);
      db->data[0] = *p;
      s = p + 2;  /* skip char and closing delimiter */
    } else {
      eol(s);
      return;
    }
  }
  if (db->size == 0) {
    eol(s);
    return;
  }

  /* Allocate output buffer for compressed data */
  outbuf = mymalloc(outmax);
  outlen = 0;

  /* Helper macro to emit a nibble */
  #define EMIT_NIBBLE(nib) do { \
    if (ac_nibble_flag == 0) { \
      ac_pending_nibble = (nib) & 0x0F; \
      ac_nibble_flag = 1; \
    } else { \
      if (outlen >= outmax) { \
        outmax *= 2; \
        outbuf = myrealloc(outbuf, outmax); \
      } \
      outbuf[outlen++] = (ac_pending_nibble << 4) | ((nib) & 0x0F); \
      ac_nibble_flag = 0; \
    } \
  } while(0)

  /* Process each character in the input string */
  for (i = 0; i < db->size; i++) {
    unsigned char ch = db->data[i];
    int found = 0;
    int j;

    /* Search table 1 (single nibble: index+1) */
    for (j = 0; j < ac_table1_len && !found; j++) {
      if ((unsigned char)ac_table1[j] == ch) {
        EMIT_NIBBLE(j + 1);
        found = 1;
      }
    }

    /* Search table 2 (two nibbles: 0, index+1) */
    for (j = 0; j < ac_table2_len && !found; j++) {
      if ((unsigned char)ac_table2[j] == ch) {
        EMIT_NIBBLE(0);
        EMIT_NIBBLE(j + 1);
        found = 1;
      }
    }

    /* Search table 3 (three nibbles: 0, 0, index+1)
       The runtime THIRD.TABLE has a placeholder at index 0 (typically '.'),
       so actual characters start at index 1. This matches the structure
       of tables 1 and 2 where index 0 is reserved. */
    for (j = 0; j < ac_table3_len && !found; j++) {
      if ((unsigned char)ac_table3[j] == ch) {
        EMIT_NIBBLE(0);
        EMIT_NIBBLE(0);
        EMIT_NIBBLE(j + 1);  /* Use j+1 to match THIRD.TABLE structure */
        found = 1;
      }
    }

    /* If character not found in tables and it's a digit, use as blank count.
       This allows digits in tables to be encoded as characters (main assembler),
       while digits NOT in tables become space counts (SCI module).
       Digit N means output N spaces (0 = 10 spaces). */
    if (!found && ch >= '0' && ch <= '9') {
      int blank_count = (ch == '0') ? 10 : (ch - '0');
      /* Find space in table 1 */
      int space_idx = -1;
      for (j = 0; j < ac_table1_len; j++) {
        if (ac_table1[j] == ' ') {
          space_idx = j + 1;
          break;
        }
      }
      if (space_idx > 0) {
        for (j = 0; j < blank_count; j++) {
          EMIT_NIBBLE(space_idx);
        }
        found = 1;
      }
    }

    if (!found) {
      /* Character not in any table - output error and skip */
      general_error(38, ch);  /* illegal reloc */
    }
  }

  /* Do NOT flush pending nibble here - nibble state persists across
     .AC directives. Messages are packed continuously, sharing bytes
     at boundaries. The decompressor scans nibbles, not bytes. */

  #undef EMIT_NIBBLE

  /* Output the compressed data (complete bytes only) */
  if (outlen > 0) {
    dblock *out_db = new_dblock();
    out_db->size = outlen;
    out_db->data = mymalloc(outlen);
    memcpy(out_db->data, outbuf, outlen);
    add_atom(0, new_data_atom(out_db, 1));
  }

  myfree(outbuf);
  eol(s);
}


/* SCASM .AZ directive - ASCII zero-terminated with flexible delimiters */
static void handle_az(char *s)
{
  dblock *db;
  char *opstart;

  for (;;) {
    opstart = s;
    s = skip(s);
    if (ISEOL(s))
      break;

    /* Parse string with any non-whitespace delimiter */
    if (*s && !isspace((unsigned char)*s)) {
      char delim = *s;
      db = parse_string(&s,delim,8);

      /* parse_string returns NULL for single-character strings.
         Handle this case explicitly. */
      if (!db) {
        char *p = opstart;
        p = skip(p);
        if (*p == delim) {
          p++;  /* skip opening delimiter */
          if (*p && !ISEOL(p) && p[1] == delim) {
            /* Single character between delimiters */
            db = new_dblock();
            db->size = 1;
            db->data = mymalloc(1);
            db->data[0] = *p;
            s = p + 2;  /* skip char and closing delimiter */
          }
        }
      }

      if (db) {
        apply_delimiter_highbit(db, delim);
        add_atom(0,new_data_atom(db,1));
        /* Add zero terminator as separate space atom */
        add_atom(0,new_space_atom(number_expr(1),1,0));
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
      if (!igntrail) {
        syntax_error(9);  /* , expected */
        return;
      }
      break;  /* Exit loop when igntrail is true and trailing content present */
    }
  }
  eol(s);
}


/* SCASM .AT directive - ASCII with high bit inverted on last character
   High bit handling depends on delimiter:
   - Delimiter < $27 (like '-'): Set high bit on all chars, then toggle last
   - Delimiter >= $27 (like '/', '"'): No high bit on chars, then toggle last
   This matches S-C Assembler behavior for command tables vs display strings. */
static void handle_at(char *s)
{
  char *opstart;
  dblock *db;

  for (;;) {
    opstart = s;
    s = skip(s);

    if (ISEOL(s))
      break;

    /* Parse string with any non-whitespace delimiter */
    if (*s && !isspace((unsigned char)*s)) {
      char delim = *s;
      db = parse_string(&s,delim,8);

      /* parse_string returns NULL for single-character strings.
         Handle this case explicitly for .AT directive. */
      if (!db) {
        char *p = opstart;
        p = skip(p);
        if (*p == delim) {
          p++;  /* skip opening delimiter */
          if (*p && !ISEOL(p) && p[1] == delim) {
            /* Single character between delimiters */
            db = new_dblock();
            db->size = 1;
            db->data = mymalloc(1);
            db->data[0] = *p;
            s = p + 2;  /* skip char and closing delimiter */
          }
        }
      }

      if (db && db->size > 0) {
        /* Apply delimiter rule first (sets high bit if delim < $27) */
        apply_delimiter_highbit(db, delim);
        /* Then toggle high bit on last character for end-of-string marker */
        db->data[db->size - 1] ^= 0x80;
        add_atom(0,new_data_atom(db,1));
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
      if (!igntrail) {
        syntax_error(9);  /* , expected */
        return;
      }
      break;  /* Exit loop when igntrail is true and trailing content present */
    }
  }
  eol(s);
}


/* SCASM 3.1 .CS directive - C-string with escape sequences */
static void handle_cs(char *s)
{
  dblock *db;
  unsigned char *data;
  int len = 0, maxlen = 256;
  char delim;

  data = mymalloc(maxlen);
  s = skip(s);

  for (;;) {
    s = skip(s);
    if (ISEOL(s))
      break;

    delim = *s++;
    if (ISEOL(s)) {
      syntax_error(30);
      myfree(data);
      return;
    }

    while (*s && *s != delim) {
      unsigned char c;

      if (*s == '\\' && s[1]) {
        s++;
        switch (*s) {
          case 'a': c = 0x07; break;
          case 'b': c = 0x08; break;
          case 'e': c = 0x1B; break;
          case 'f': c = 0x0C; break;
          case 'n': c = 0x0A; break;
          case 'r': c = 0x0D; break;
          case 't': c = 0x09; break;
          case 'v': c = 0x0B; break;
          case '0': c = 0x00; break;
          case '\\': c = '\\'; break;
          case '"': c = '"'; break;
          case '\'': c = '\''; break;
          case 'x':
            if (isxdigit((unsigned char)s[1]) && isxdigit((unsigned char)s[2])) {
              int hi = isdigit((unsigned char)s[1]) ? s[1]-'0' : (toupper((unsigned char)s[1])-'A'+10);
              int lo = isdigit((unsigned char)s[2]) ? s[2]-'0' : (toupper((unsigned char)s[2])-'A'+10);
              c = (hi << 4) | lo;
              s += 2;
            } else {
              c = 'x';
            }
            break;
          default:
            c = *s;
            break;
        }
        s++;
      } else {
        c = *s++;
      }

      if (len >= maxlen) {
        maxlen *= 2;
        data = myrealloc(data, maxlen);
      }
      data[len++] = c;
    }

    if (*s == delim)
      s++;

    s = skip(s);
    if (*s == ',')
      s++;
    else
      break;
  }

  if (len > 0) {
    db = new_dblock();
    db->size = len;
    db->data = mymalloc(len);
    memcpy(db->data, data, len);
    add_atom(0, new_data_atom(db, 1));
  }
  myfree(data);
  eol(s);
}


/* SCASM 3.1 .CZ directive - C-string zero terminated */
static void handle_cz(char *s)
{
  handle_cs(s);
  add_atom(0, new_space_atom(number_expr(1), 1, 0));
}


/* SCASM 3.1 .HX directive - Hex nibble storage */
static void handle_hx(char *s)
{
  dblock *db;
  unsigned char *data;
  int len = 0, maxlen = 256;
  int val;

  data = mymalloc(maxlen);
  s = skip(s);

  while (*s && !ISEOL(s)) {
    while (*s && (isspace((unsigned char)*s) || *s == '.' || *s == ','))
      s++;

    if (ISEOL(s) || !isxdigit((unsigned char)*s))
      break;

    if (isdigit((unsigned char)*s))
      val = *s - '0';
    else
      val = toupper((unsigned char)*s) - 'A' + 10;

    if (len >= maxlen) {
      maxlen *= 2;
      data = myrealloc(data, maxlen);
    }
    data[len++] = (unsigned char)val;
    s++;
  }

  if (len > 0) {
    db = new_dblock();
    db->size = len;
    db->data = mymalloc(len);
    memcpy(db->data, data, len);
    add_atom(0, new_data_atom(db, 1));
  }
  myfree(data);
  eol(s);
}


/* SCASM 3.1 .PS directive - Pascal string (length-prefixed) */
static void handle_ps(char *s)
{
  dblock *db;
  unsigned char *data;
  int len = 1, maxlen = 256;
  char delim;

  data = mymalloc(maxlen);
  s = skip(s);

  for (;;) {
    s = skip(s);
    if (ISEOL(s))
      break;

    delim = *s++;
    if (ISEOL(s)) {
      syntax_error(30);
      myfree(data);
      return;
    }

    while (*s && *s != delim) {
      if (len >= maxlen) {
        maxlen *= 2;
        data = myrealloc(data, maxlen);
      }
      data[len++] = *s++;
    }

    if (*s == delim)
      s++;

    s = skip(s);
    if (*s == ',')
      s++;
    else
      break;
  }

  if (len > 1) {
    int str_len = len - 1;
    if (str_len > 255)
      str_len = 255;
    data[0] = (unsigned char)str_len;

    db = new_dblock();
    db->size = len;
    db->data = mymalloc(len);
    memcpy(db->data, data, len);
    add_atom(0, new_data_atom(db, 1));
  }
  myfree(data);
  eol(s);
}


/* SCASM .TA directive - Target Address
   In native SCASM, this sets where object code is written in memory,
   separate from the origin address used for label calculations.
   In cross-assembly, this has no effect since we write to files. */
static void handle_ta(char *s)
{
  expr *e;
  s = skip(s);
  if (!ISEOL(s)) {
    e = parse_expr(&s);
    if (e) {
      /* Warn user that .TA has no effect in cross-assembly */
      syntax_error(42);  /* .TA directive ignored in cross-assembly */
      free_expr(e);
    }
  }
  eol(s);
}


static void handle_op(char *s)
{
  /* SCASM .OP directive - change CPU type */
  char *cpuname;

  /* Skip whitespace */
  s = skip(s);
  cpuname = s;

  /* Call CPU module's set_cpu_type() directly */
  if (!set_cpu_type(cpuname)) {
    syntax_error(24);  /* invalid CPU type for .OP directive */
  }
}


static void handle_str(char *s)  /* GMGM entire handle_str function */
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
    expr *orgexp;
    taddr val;

#if !defined(VASM_CPU_Z80)
    if (*s == '#')
      s = skip(s+1);  /* some strange assemblers allow ORG #<addr> */
#endif

    /* SCASM: Parse expression, then try to evaluate it
       This allows symbols defined with .EQ * to work in .OR directive */
    orgexp = parse_expr_tmplab(&s);

    if (orgexp && eval_expr(orgexp,&val,NULL,0)) {
      /* Expression evaluates to constant value */
      if (dsect_active)
        switch_offset_section(NULL,val);
      else
        set_section(new_org(val));
      free_expr(orgexp);
    }
    else if (orgexp) {
      /* Expression cannot be evaluated yet - pass to vasm for resolution */
      syntax_error(31);  /* expression must be constant */
      free_expr(orgexp);
    }
    else {
      syntax_error(10);  /* identifier expected */
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
  if (!dsect_active) {
    last_alloc_sect = current_section;
    dsect_active = 1;
  }
  else
    syntax_error(35);  /* .DUMMY section already active */

  switch_offset_section(NULL,dsect_offs);
  eol(s);
}


static void handle_dend(char *s)
{
  if (dsect_active) {
    dsect_offs = current_section->pc;
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


/* SCASM: Try to find file with various source extensions */
static char *try_source_extensions(const char *basename)
{
  static const char *exts[] = { "", ".s", ".S", ".asm", ".ASM", NULL };
  char *testname;
  FILE *f;
  int i;

  for (i = 0; exts[i] != NULL; i++) {
    size_t len = strlen(basename) + strlen(exts[i]) + 1;
    testname = mymalloc(len);
    strcpy(testname, basename);
    strcat(testname, exts[i]);

    /* Try to open the file to see if it exists - use simple fopen */
    /* This checks current directory only, but include_source will */
    /* check include paths, so it's ok if we don't find it here */
    if (f = fopen(testname, "r")) {
      fclose(f);
      return testname;  /* Found it! */
    }
    myfree(testname);
  }

  return NULL;  /* Not found with any extension */
}


static void handle_include(char *s)
{
  strbuf *name;
  char *found_filename;

  if (name = parse_name(0,&s)) {
    eol(s);

    /* SCASM: Try to find file with source extensions (.s, .S, .asm, .ASM)
       using include paths */
    found_filename = locate_file_with_extensions(name->str);

    if (found_filename) {
      include_source(found_filename);
      myfree(found_filename);
    }
    else {
      /* Not found - let include_source handle the error */
      include_source(name->str);
    }
  }
}


static void handle_incbin(char *s)
{
  strbuf *name;
  long delta = 0;
  unsigned long nbbytes = 0;
  char *filename;
  char *found_filename;

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

    filename = name->str;

    /* SCASM: Try to find file with source extensions (.s, .S, .asm, .ASM)
       using include paths. If found, include as source. */
    found_filename = locate_file_with_extensions(filename);

    if (found_filename) {
      /* Found a source file - include it as source, not binary */
      include_source(found_filename);
      myfree(found_filename);
    }
    else {
      /* Not found as source - try as binary include */
      include_binary_file(filename, delta, nbbytes);
    }
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
  /* .LIST - Enable listing with optional parameters
     SCASM format: .list [xoff] [moff] [symoff] ...
     Parameters: xoff (cross-ref off), moff (macro off), symoff (symbol off), etc.
     For now, we just enable listing and ignore the parameters */
  set_listing(1);
  /* Consume any parameters on the line */
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

static void handle_lif(char *s)
{
  /* .LIF - List IF (toggle listing control) - SCASM directive */
  /* No-op: vasm's -L option controls listing globally */
}

static void handle_ti(char *s)
{
  /* .TI - Title (set listing title with column,text format) - SCASM directive */
  /* Format: .TI column,title text
     Example: .TI 76,S-C MACRO ASSEMBLER (ProDOS) 3.0 */
  /* No-op: title is for listing output which vasm handles differently */
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


/* SCASM: Global variables for A2osX build metadata */
static char *scmasm_target_file = NULL;   /* .TF directive - binary output path */
static char *scmasm_source_file = NULL;   /* TEXT directive - source listing path */

/* SCASM: No-op handler for directives that should be ignored */
static void handle_noop(char *s)
{
  /* Silently ignore the directive and its arguments */
}

/* SCASM: Handle .TF directive - extract target binary path */
static void handle_tf(char *s)
{
  char *start, *end;

  s = skip(s);
  if (ISEOL(s))
    return;

  start = s;
  /* Read until whitespace or end of line */
  while (*s && !isspace((unsigned char)*s))
    s++;
  end = s;

  /* Store the target file path */
  if (scmasm_target_file)
    myfree(scmasm_target_file);
  scmasm_target_file = cnvstr(start, end - start);

  eol(s);
}

/* SCASM: Handle AUTO directive - parse but ignore line numbering */
static void handle_auto(char *s)
{
  /* AUTO <start>,<increment> - just consume the arguments */
  s = skip(s);
  if (!ISEOL(s)) {
    /* Skip first number */
    while (isdigit((unsigned char)*s))
      s++;
    s = skip(s);
    /* Skip comma if present */
    if (*s == ',') {
      s = skip(s + 1);
      /* Skip second number */
      while (isdigit((unsigned char)*s))
        s++;
    }
  }
  eol(s);
}

/* SCASM: Handle TEXT directive - extract source listing path */
static void handle_text_directive(char *s)
{
  char *start, *end;

  s = skip(s);
  if (ISEOL(s))
    return;

  start = s;
  /* Read until whitespace or end of line */
  while (*s && !isspace((unsigned char)*s))
    s++;
  end = s;

  /* Store the source file path */
  if (scmasm_source_file)
    myfree(scmasm_source_file);
  scmasm_source_file = cnvstr(start, end - start);

  eol(s);
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
  "ph",handle_rorg,      /* SCASM: .PH - phase start */
  "ep",handle_rend,      /* SCASM: .EP - end phase */
  "roffs",handle_roffs,
  "align",handle_align,
  "even",handle_even,
  "pg",handle_pg,        /* SCASM: .PG - align to 256-byte page boundary */
  "data",handle_secdata,
  /* "text" removed - conflicts with SCASM TEXT directive for source listing */
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
  "in",handle_include,   /* SCASM: .IN - include file (2-char alias) */
  "rept",handle_rept,
  "repeat",handle_rept,
  "lu",handle_rept,      /* SCASM: .LU - loop (2-char alias) */
  "endr",handle_endr,
  "endrep",handle_endr,
  "endrepeat",handle_endr,
  "endu",handle_endr,    /* SCASM: .ENDU - end loop (4-char alias) */
#if !defined(VASM_CPU_UNSP)
  "mac",handle_macro, /* Clashes with unSP instruction */
#endif
  "ma",handle_macro,   /* SCASM macro directive */
  "macro",handle_macro,
  "em",handle_endm,    /* SCASM endmacro directive */
  "endm",handle_endm,
  "endmac",handle_endm,
  "endmacro",handle_endm,
  "end",handle_end,
  "exitmacro",handle_exitmacro,
  "fail",handle_fail,
  "section",handle_section,
  "dsect",handle_dsect,
  "dend",handle_dend,
  "dummy",handle_dsect,  /* SCASM: .DUMMY - dummy section start */
  "ed",handle_dend,      /* SCASM: .ED - end dummy */
  "binary",handle_incbin,
  "inb",handle_include,  /* SCASM: .INB - include source file (same as .IN) */
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
  /* SCASM-specific directive aliases */
  "or",handle_org,        /* .OR - origin (alias for org) */
  "hs",handle_hs,         /* .HS - hex string */
  "bs",handle_spc8,       /* .BS - block storage (alias for ds) */
  "do",handle_ifne,       /* .DO - conditional start (like if) */
  "else",handle_else,     /* .ELSE - conditional else clause */
  "fin",handle_endif,     /* .FIN - conditional end (alias for endif) */
  "as",handle_as,         /* .AS - ASCII string with flexible delimiters */
  "ac",handle_ac,         /* .AC - ASCII string with optional numeric prefix */
  "az",handle_az,         /* .AZ - ASCII zero-terminated with flexible delimiters */
  "at",handle_at,         /* .AT - ASCII with high bit set on last char */
  "cs",handle_cs,         /* .CS - C-string with escape sequences (SCASM 3.1) */
  "cz",handle_cz,         /* .CZ - C-string zero-terminated (SCASM 3.1) */
  "hx",handle_hx,         /* .HX - Hex nibble storage (SCASM 3.1) */
  "ps",handle_ps,         /* .PS - Pascal string (length-prefixed) (SCASM 3.1) */
  "ta",handle_ta,         /* .TA - Target Address (ignored in cross-assembly) */
  "op",handle_op,         /* .OP - select CPU type (6502/65C02/65816) */
  /* SCASM editor directives */
  "new",handle_noop,      /* NEW - SCASM editor command (clear buffer - ignored) */
  "auto",handle_auto,     /* AUTO - Automatic line numbering (consumed) */
  "tf",handle_tf,         /* .TF - Target file metadata (extracted for build) */
  "text",handle_text_directive,  /* TEXT - Source listing path (extracted) */
  /* SCASM listing control directives */
  "lif",handle_lif,       /* .LIF - List IF (toggle listing control) */
  "ti",handle_ti          /* .TI - Title (set listing title) */
};

int dir_cnt = sizeof(directives) / sizeof(directives[0]);


/* checks for a valid directive, and return index when found, -1 otherwise */
static int check_directive(char **line)
{
  char *s,*name;
  hashdata data;

  s = skip(*line);

  /* SCASM: Handle . prefix for directives */
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
  { NULL,0 }, { "=",1 }, { "equ",3 }, { "eq",2 }, { "se",2 }, { "set",3 }
};
enum {
  ASN_NONE=0, ASN_EQ1, ASN_EQ2, ASN_EQ3, ASN_SE, ASN_SET, ASN_NUM
};


static char *parse_label_field(char **start,int *asntype)
{
  char *s,*name;
  int spaced;  /* potential label is spaced and needs a ':' or '=' */
  int is_scmasm_special = 0;  /* Flag for :N or .N labels */

  s = *start;
  if (asntype)
    *asntype = ASN_NONE;

  if (isspace((unsigned char )*s)) {
    s = skip(s);
    spaced = 1;
  }
  else
    spaced = 0;

  /* SCASM: Check for private label :N (where N is a digit) */
  /* Only treat as label if not spaced (i.e., at start of line) */
  if (!spaced && *s == ':' && isdigit((unsigned char)*(s+1))) {
    /* SCASM private label: :1, :2, etc. */
    if (asntype) {
      char labnum[32];
      char *numstart = s + 1;
      char *numend = numstart;
      strbuf *buf;

      while (isdigit((unsigned char)*numend))
        numend++;

      /* Create unique private label name using context */
      sprintf(labnum, "_%d_%.*s", scmasm_private_context,
              (int)(numend - numstart), numstart);
      buf = make_local_label(0, ":", 1, labnum, strlen(labnum));
      name = buf->str;
      is_scmasm_special = 1;  /* Mark as special label */
      *start = skip(numend);
    }
    else
      name = s;
  }
  /* SCASM: If line starts with '.' but not followed by a digit, it's a directive */
  /* not a label - return NULL */
  else if (!spaced && *s == '.' && !isdigit((unsigned char)*(s+1))) {
    /* This is a directive (e.g., .OR, .DA), not a label */
    return NULL;
  }
  /* SCASM: Check for local label .N (where N is 1-255) */
  /* Only treat as label if not spaced (i.e., at start of line) */
  else if (!spaced && *s == '.' && isdigit((unsigned char)*(s+1))) {
    /* SCASM local label: .1, .2, ..., .255 */
    if (asntype) {
      char *numstart = s + 1;
      char *numend = numstart;
      int labnum;
      strbuf *buf;

      while (isdigit((unsigned char)*numend))
        numend++;

      labnum = atoi(numstart);
      if (labnum < 0 || labnum > 255) {
        syntax_error(11);  /* invalid local label number (must be 0-255) */
        name = NULL;
      }
      else if (scmasm_last_global_label == NULL) {
        syntax_error(12);  /* local label without global label context */
        name = NULL;
      }
      else {
        /* Create local label scoped to last global label */
        buf = make_local_label(0, scmasm_last_global_label,
                               strlen(scmasm_last_global_label),
                               numstart, numend - numstart);
        name = buf->str;
        is_scmasm_special = 1;  /* Mark as special label */
      }
      *start = skip(numend);
    }
    else
      name = s;
  }
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
    name = parse_symbol(&s);

    if (name==NULL && *s==current_pc_char && !ISIDCHAR(*(s+1))) {
      name = current_pc_str;
      s++;
    }

    if (name) {
      /* we need anything to authentify a spaced symbol/label, like a ':', a '=',
         or an equate or set-directive */
      s = skip(s);

      /* SCASM: Also exclude :N private labels from label suffix check */
      if (s[0]==':' && s[1]!='+' && s[1]!='-' && !isdigit((unsigned char)s[1])) {
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
    /* SCASM: Skip optional line numbers at start of line (e.g., "1000  .OR $2000") */
    /* Line numbers are all digits followed by whitespace */
    s = line;
    if (isdigit((unsigned char)*s)) {
      while (isdigit((unsigned char)*s))
        s++;
      /* Only skip if followed by whitespace or comment */
      if (isspace((unsigned char)*s)) {
        /* Skip line number, but preserve one space if there was whitespace */
        /* This ensures that "1000 .OR" becomes " .OR" (spaced, no label) */
        /* not ".OR" (unspaced, would be treated as label) */
        s++;  /* skip first whitespace character */
        line = s;  /* keep rest of line including any additional whitespace */
      }
      else if (*s == '*' || *s == '\0') {
        /* Line number followed by comment or EOL */
        line = skip(s);
      }
      else {
        /* Not a line number (e.g., a label like "1000LABEL"), restore */
        s = line;
      }
    }

    /* SCASM: * in column 1 (after line number) is always a comment */
    if (parse_end || *line=='*')
      continue;

    /* SCASM: Check for bare editor directives (NEW, AUTO, TEXT) at start of line.
       IMPORTANT: Only treat these as editor commands when NOT followed by an
       instruction. If followed by an instruction (after whitespace), they are
       label definitions like "AUTO   JSR something". */
    s = line;
    if (*s && !isspace((unsigned char)*s)) {
      char *rest;
      int is_editor_cmd = 0;

      if (!strnicmp(s, "NEW", 3) && (isspace((unsigned char)s[3]) || s[3]=='\0')) {
        /* NEW alone on line = editor command */
        rest = skip(s+3);
        if (ISEOL(rest) || *rest == '*' || *rest == commentchar) {
          handle_noop(s+3);
          is_editor_cmd = 1;
        }
      }
      else if (!strnicmp(s, "AUTO", 4) && (isspace((unsigned char)s[4]) || s[4]=='\0')) {
        /* AUTO followed by digits or EOL = editor command */
        /* AUTO followed by letters (instruction) = label definition */
        rest = skip(s+4);
        if (ISEOL(rest) || *rest == '*' || *rest == commentchar ||
            isdigit((unsigned char)*rest)) {
          handle_auto(s+4);
          is_editor_cmd = 1;
        }
      }
      else if (!strnicmp(s, "TEXT", 4) && (isspace((unsigned char)s[4]) || s[4]=='\0')) {
        /* TEXT followed by path or EOL = editor command */
        /* TEXT followed by instruction = label definition */
        rest = skip(s+4);
        /* Editor TEXT command takes a filename, which typically starts with / or letter */
        /* But "TEXT   LDX" has LDX as an instruction. Distinguish by checking if it's
           a known instruction or if there's more after it suggesting it's code */
        if (ISEOL(rest) || *rest == '*' || *rest == commentchar) {
          handle_text_directive(s+4);
          is_editor_cmd = 1;
        }
        /* If rest starts with a letter and is short (filename), could be either.
           Let's be conservative: only treat as editor cmd if EOL/comment follows. */
      }

      if (is_editor_cmd)
        continue;
    }

    if (!cond_state()) {
      /* skip source until ELSE or ENDIF */
      int idx;

      s = line;
      (void)parse_label_field(&s,NULL);
      idx = check_directive(&s);
      if (idx >= 0) {
        /* Check for nested conditionals - need to track nesting depth.
           SCASM uses .DO for conditionals, not just if* directives. */
        if (!strncmp(directives[idx].name,"if",2) ||
            !strcmp(directives[idx].name,"do"))
          cond_skipif();
        else if (directives[idx].func == handle_else)
          cond_else();
        else if (directives[idx].func == handle_endif)
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

        if (asn_type < ASN_SE) {
          /* EQU or '=' */
          if (*labname == current_pc_char) {
            handle_org(s);
            continue;
          }
          else {
            /* SCASM: Support multi-byte .EQ (e.g., .EQ $36,37 or .EQ $BEF2,3,4) */
            /* Parse first value - this becomes the symbol value */
            expr *first_expr = parse_expr_tmplab(&s);
            label = new_equate(labname,first_expr);

            /* Check for commas indicating additional bytes (for documentation) */
            /* Additional bytes are implicitly at symbol+1, symbol+2, etc. */
            s = skip(s);
            while (*s == ',') {
              s = skip(s+1);
              /* Parse and discard additional values - they're for documentation only */
              parse_expr_tmplab(&s);
              s = skip(s);
            }
            eol(s);
            continue;
          }
        }
        else if (asn_type >= ASN_SE) {
          /* SE and SET allow redefinitions */
          if (*labname == current_pc_char)
            syntax_error(10);  /* identifier expected */
          else
            label = new_abs(labname,parse_expr_tmplab(&s));
        }
        else
          ierror(0);
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
        label = new_labsym(current_section,labname);
        add_atom(0,new_label_atom(label));

        /* SCASM: Track global labels for local label scoping */
        /* A global label doesn't start with a space (local labels created by make_local_label start with space) */
        if (labname[0] != ' ' && labname[0] != ':' && labname[0] != '.') {
          /* This is a global label - update context */
          if (scmasm_last_global_label)
            myfree((void *)scmasm_last_global_label);
          scmasm_last_global_label = mystrdup(labname);
          scmasm_private_context++;  /* New major scope for private labels */
        }
      }

      if (autoexport && !is_local_symbol_name(labname))
        label->flags |= EXPORT;
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

    /* SCASM: Check for macro invocation with > or _ prefix */
    if (*s == '>' || *s == '_') {
      char *macro_name;
      char *name_start;
      int name_len;
      macro *m;

      s++;  /* skip the > or _ prefix */
      name_start = s;

      /* Read macro name */
      if (!ISIDSTART(*s)) {
        syntax_error(10);  /* identifier expected */
        continue;
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

        if (execute_macro(name_start, name_len, NULL, NULL, 0, s))
          continue;
      }
      else {
        /* Macro not found - error */
        syntax_error(41);  /* undefined macro */
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
    int skip_operands = 0;  /* Flag: set if we detect multiple spaces (= comment) */

    if (!isspace((unsigned char)*s) && *s!='\0')
      syntax_error(2);  /* no space before operands */

    /* SCASM igntrail: One whitespace allowed before operand, more = comment.
       If there's whitespace, skip only ONE whitespace char max. Multiple = comment */
    if (igntrail && isspace((unsigned char)*s)) {
      s++;  /* Skip the ONE allowed whitespace character */
      /* If there's another whitespace, it's a comment - stop parsing operands */
      if (isspace((unsigned char)*s)) {
        s = skip(s);  /* Skip rest of whitespace */
        skip_operands = 1;  /* Set flag to skip operand parsing */
      }
    }
    else {
      s = skip(s);  /* Normal mode - skip all whitespace */
    }

    if (execute_macro(inst,inst_len,ext,ext_len,ext_cnt,s))
      continue;
    if (execute_struct(inst,inst_len,s))
      continue;

    /* read operands, terminated by comma or blank (unless in parentheses) */
    op_cnt = 0;

    if (!skip_operands) {  /* Only parse operands if we're not in a comment line */
    while (!ISEOL(s) && op_cnt<MAX_OPERANDS) {
      char *saved_s = s;  /* Save position for potential space check */
      op[op_cnt] = s;
      s = skip_operand(1,s);

      op_len[op_cnt] = oplen(s,op[op_cnt]);

      /* In igntrail mode, SCASM's expression parser stops at spaces (they're in OPERATOR.CHARS).
         So we need to stop at the first space, even if skip_operand returned further.
         This prevents "M-MMM00000" from being parsed as "M - MMM00000" */
      if (igntrail && op_len[op_cnt] > 0) {
        char *op_str = op[op_cnt];
        int i;
        for (i = 0; i < op_len[op_cnt]; i++) {
          if (isspace((unsigned char)op_str[i])) {
            /* Found a space - truncate operand here */
            op_len[op_cnt] = i;
            break;
          }
        }
      }

#if !ALLOW_EMPTY_OPS
      if (op_len[op_cnt] <= 0)
        syntax_error(5);  /* missing operand */
      else
#endif
        op_cnt++;

      if (igntrail) {
        /* SCASM: Whitespace terminates operand parsing.
           If we just hit whitespace, everything after is a comment. */
        if (isspace((unsigned char)*s) || *s == '\0' || *s == commentchar) {
          /* Operand parsing was terminated by whitespace or EOL - done */
          break;
        }
        /* Check for comma - but only if there's actually an operand after it */
        if (*s == ',') {
          char *after_comma = skip(s+1);  /* skip whitespace after comma */
          if (*after_comma && !isspace((unsigned char)*after_comma) && *after_comma != commentchar) {
            /* There's something after the comma - it might be a real operand */
            s = after_comma;
          }
          else {
            /* Nothing substantial after comma - it's part of the comment */
            break;
          }
        }
        else {
          /* No comma - done with operands */
          break;
        }
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
    }  /* Close if (!skip_operands) block */
    eol(s);

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

  /* SCASM: Output build metadata if extracted from source (.TF/.SF directives) */
  if (scmasm_target_file || scmasm_source_file) {
    fprintf(stderr, "\n");
    if (scmasm_target_file)
      fprintf(stderr, "SCASM Target Binary: %s\n", scmasm_target_file);
    if (scmasm_source_file)
      fprintf(stderr, "SCASM Source Listing: %s\n", scmasm_source_file);
  }
}


/* parse next macro argument */
char *parse_macro_arg(struct macro *m,char *s,
                      struct namelen *param,struct namelen *arg)
{
  arg->len = 0;  /* cannot select specific named arguments */

  /* SCASM: If argument starts with a quote, strip the quotes.
     This allows passing string content to macros like:
       >QT LABEL,"MESSAGE TEXT%"
     where ]2 should be MESSAGE TEXT% without quotes. */
  if (*s == '"' || *s == '\'') {
    char quote = *s;
    s++;  /* skip opening quote */
    param->name = s;
    /* Find closing quote */
    while (*s && *s != quote) {
      if (*s == '\\' && *(s+1))
        s++;  /* skip escaped character */
      s++;
    }
    param->len = s - param->name;
    if (*s == quote)
      s++;  /* skip closing quote */
    return s;
  }

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

  /* SCASM: Support both \ (vasm standard) and ] (SCASM standard) escapes */
  if (escape_char == ']') {
    /* SCASM macro parameter notation: ]1-]9, ]#, ]] */

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
    else if (*s == '#') {
      /* ]#: insert parameter count */
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
      /* ]1..]9, ]0: insert macro parameter */
      nc = copy_macro_param(src, *s=='0'? 9 : *s-'1', d, dlen);
      s++;
    }
    else {
      /* Not a valid SCASM escape - output ] and continue */
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
/* SCASM: Handle three-tier labels: global, :N private, .N local */
{
  char *s;
  strbuf *name;

  name = NULL;
  s = *start;

  /* SCASM: Check for private label reference :N */
  if (*s == ':' && isdigit((unsigned char)*(s+1))) {
    char labnum[32];
    char *numstart = s + 1;
    char *numend = numstart;

    while (isdigit((unsigned char)*numend))
      numend++;

    /* Create unique private label name using current context */
    sprintf(labnum, "_%d_%.*s", scmasm_private_context,
            (int)(numend - numstart), numstart);
    name = make_local_label(n, ":", 1, labnum, strlen(labnum));

    /* SCASM igntrail: Don't skip whitespace - let exp_skip() handle it */
    if (igntrail)
      *start = numend;
    else
      *start = skip(numend);
  }
  /* SCASM: Check for local label reference .N */
  else if (*s == '.' && isdigit((unsigned char)*(s+1))) {
    char *numstart = s + 1;
    char *numend = numstart;
    int labnum;

    while (isdigit((unsigned char)*numend))
      numend++;

    labnum = atoi(numstart);
    if (labnum < 0 || labnum > 255) {
      syntax_error(11);  /* invalid local label number (must be 0-255) */
      *start = numend;
      return NULL;
    }

    if (scmasm_last_global_label == NULL) {
      syntax_error(12);  /* local label without global label context */
      *start = numend;
      return NULL;
    }

    /* Create local label scoped to last global label */
    name = make_local_label(n, scmasm_last_global_label,
                           strlen(scmasm_last_global_label),
                           numstart, numend - numstart);

    /* SCASM igntrail: Don't skip whitespace - let exp_skip() handle it */
    if (igntrail)
      *start = numend;
    else
      *start = skip(numend);
  }
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

    /* SCASM igntrail: Don't skip whitespace - let exp_skip() handle it */
    if (igntrail)
      *start = s;
    else
      *start = skip(s);
  }

  return name;
}


int init_syntax(void)
{
  size_t i;
  hashdata data;

  /* SCASM: All directives start with '.' */
  dotdirs = 1;

  /* SCASM: Ignore trailing content after operands (allows inline comments without ';') */
  igntrail = 1;

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
    autoexport = 1;
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


/* SCASM: Get A2osX metadata extracted from source */
const char *get_a2osx_target_file(void)
{
  return scmasm_target_file;
}

const char *get_a2osx_source_listing(void)
{
  return scmasm_source_file;
}
