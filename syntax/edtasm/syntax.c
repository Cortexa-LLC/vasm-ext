/* syntax.c  syntax module for vasm */
/* (c) in 2002-2025 by Frank Wille */
/* EDTASM syntax adaptation (c) 2026 Bryan Woodruff, Cortexa LLC*/

#include "vasm.h"

/* The syntax module parses the input (read_next_line), handles
   assembly-directives (section, data-storage etc.) and parses
   mnemonics. Assembly instructions are split up in mnemonic name,
   qualifiers and operands. new_inst returns a matching instruction,
   if one exists.
   Routines for creating sections and adding atoms to sections will
   be provided by the main module.
*/

const char *syntax_copyright="vasm EDTASM syntax module 1.0 (c) 2026 Bryan Woodruff, Cortexa LLC";
hashtable *dirhash;
char commentchar = ';';
int dotdirs;

static char code_name[] = "CODE";
static char data_name[] = "DATA";
static char bss_name[] = "BSS";
static char code_type[] = "acrx";
static char data_type[] = "adrw";
static char bss_type[] = "aurw";
static char rs_name[] = "__RS";
static char so_name[] = "__SO";
static char fo_name[] = "__FO";
static char line_name[] = "__LINE__";

static struct namelen macro_dirlist[] = {
  { 5,"macro" }, { 0,0 }
};
static struct namelen endm_dirlist[] = {
  { 4,"endm" }, { 0,0 }
};
static struct namelen rept_dirlist[] = {
  { 4,"rept" }, { 0,0 }
};
static struct namelen endr_dirlist[] = {
  { 4,"endr" }, { 0,0 }
};
static struct namelen erem_dirlist[] = {
  { 4,"erem" }, { 0,0 }
};

/* options */
static int allow_spaces;
static int check_comm;
static int dot_idchar;
static char local_char = '.';

/* OPT directive options */
static int opt_mc = 1;    /* List macro calls */
static int opt_md = 1;    /* List macro definitions */
static int opt_mex = 0;   /* List macro expansions */
static int opt_list = 1;  /* Listing enabled */

/* (currently two-byte only) padding value for CNOPs */
#ifdef VASM_CPU_M68K
static taddr cnop_pad = 0x4e71;
#else
static taddr cnop_pad = 0;
#endif

/* directive flags */
#define DIRF_TYPEMASK 0xf
enum {  /* directive types */
  DT_NONE=0,DT_IF,DT_ELSE,DT_ELIF,DT_ENDIF
};
#define DIRF_DEVPAC (1<<4)
#define DIRF_PHXASS (1<<5)

/* unique macro IDs */
#define IDSTACKSIZE 100
static unsigned long id_stack[IDSTACKSIZE];
static int id_stack_index;

/* isolated local labels block */
#define INLSTACKSIZE 100
#define INLLABFMT "=%06d"
static int inline_stack[INLSTACKSIZE];
static int inline_stack_index;
static const char *saved_last_global_label;
static char inl_lab_name[8];
static int local_id;

static int parse_end = 0;
static expr *carg1;


char *skip(char *s)
{
  while (isspace((unsigned char )*s))
    s++;
  return s;
}


int iscomment(char *s)
{
  if (*s == commentchar)
    return 1;

  /* EDTASM: asterisk in column 1 */
  if (cur_src && s == cur_src->text && *s == '*')
    return 1;

  return 0;
}


/* issue a warning for comments introduced by blanks in the operand field */
static void comment_check(char *s)
{
  if (isspace((unsigned char)*s)) {
    s = skip(s + 1);
    if (!ISEOL(s))
      syntax_error(18);  /* check comment warning */
  }
}


/* check for end of line, issue error, if not */
void eol(char *s)
{
  if (allow_spaces) {
    s = skip(s);
    if (!ISEOL(s))
      syntax_error(6);
  }
  else {
    if (!ISEOL(s) && !isspace((unsigned char)*s))
      syntax_error(6);
  }
}


int isidchar(char c)
{
  if (isalnum((unsigned char)c) || c=='_' || c=='@' || c=='$')
    return 1;
  if (dot_idchar && c=='.')
    return 1;
  return 0;
}


#ifdef VASM_CPU_M68K
char *chkidend(char *start,char *end)
{
  if (dot_idchar && (end-start)>2 && *(end-2)=='.') {
    char c = tolower((unsigned char)*(end-1));

    if (c=='b' || c=='w' || c=='l')
      return end - 2;	/* .b/.w/.l extension is not part of identifier */
  }
  return end;
}
#endif


char *exp_skip(char *s)
{
  if (isspace((unsigned char)*s)) {
    *s = '\0';  /* rest of operand is ignored */
  }
  return s;
}


char *skip_operand(char *s)
{
  int par_cnt = 0;
  char c;

  for (;;) {
    s = exp_skip(s);
    c = *s;

    if (START_PARENTH(c)) {
      par_cnt++;
    }
    else if (END_PARENTH(c)) {
      if (par_cnt>0)
        par_cnt--;
      else
        syntax_error(3);  /* too many closing parentheses */
    }
    else if (c=='\'' || c=='\"') {
      /* Check for EDTASM single-char constant: #'x without closing quote */
      if (c=='\'' && s > cur_src->text && *(s-1)=='#') {
        /* Check if next char is followed by whitespace/comment (not a closing quote) */
        if (*(s+1) && *(s+2) &&
            (isspace((unsigned char)*(s+2)) || *(s+2)==';' || iscomment(s+2))) {
          /* Single-char constant without closing quote: #'x */
          s++;  /* Skip the char */
        }
        else {
          s = skip_string(s,c,NULL) - 1;
        }
      }
      else {
        s = skip_string(s,c,NULL) - 1;
      }
    }
    else if (!c || (par_cnt==0 && (c==',' || iscomment(s))))
      break;

    s++;
  }

  if (par_cnt != 0)
    syntax_error(4);  /* missing closing parentheses */
  return s;
}


/* make the given struct- or frame-offset symbol dividable my the next
   multiple of "align" (must be a power of 2!) */
static void setoffset_align(char *symname,int dir,utaddr align)
{
  symbol *sym;
  expr *new;

  sym = internal_abs(symname);
  --align;  /* @@@ check it */
  new = make_expr(BAND,
                  make_expr(dir>0?ADD:SUB,sym->expr,number_expr(align)),
                  number_expr(~align));
  simplify_expr(new);
  sym->expr = new;
}


/* assign value of current struct- or frame-offset symbol to an abs-symbol,
   or just increment/decrement when equname is NULL */
static symbol *new_setoffset_size(char *equname,char *symname,
                                  char **s,int dir,taddr size)
{
  symbol *sym,*equsym;
  expr *new,*old;

  /* get current offset symbol expression, then increment or decrement it */
  sym = internal_abs(symname);

  if (!ISEOL(*s)) {
    /* Make a new expression out of the parsed expression multiplied by size
       and add to or subtract it from the current symbol's expression. */
    new = make_expr(MUL,parse_expr_tmplab(s),number_expr(size));
    simplify_expr(new);
    old = sym->expr;
    new = make_expr(dir>0?ADD:SUB,old,new);
  }
  else {
    new = old = sym->expr;
  }

  /* assign expression to equ-symbol and change exp. of the offset-symbol */
  if (equname)
    equsym = new_equate(equname,dir>0 ? copy_tree(old) : copy_tree(new));
  else
    equsym = NULL;

  simplify_expr(new);
  sym->expr = new;
  return equsym;
}


/* assign value of current struct- or frame-offset symbol to an abs-symbol,
   determine operation size from directive extension first */
static symbol *new_setoffset(char *equname,char **s,char *symname,int dir)
{
  taddr size = 1;
  char *start = *s;
  char ext;

  /* get extension character and proceed to operand */
  if (*(start+2) == '.') {
    ext = tolower((unsigned char)*(start+3));
    *s = skip(start+4);
    switch (ext) {
      case 'b':
        break;
      case 'w':
        size = 2;
        break;
      case 'l':
      case 's':
        size = 4;
        break;
      case 'q':
      case 'd':
        size = 8;
        break;
      case 'x':
        size = 12;
        break;
      default:
        syntax_error(1);  /* invalid extension */
        break;
    }
  }
  else {
    size = 2;  /* defaults to 'w' extension when missing */
    *s = skip(start+2);
  }

  return new_setoffset_size(equname,symname,s,dir,size);
}


static atom *do_space(int size,expr *cnt,expr *fill)
{
  atom *a;

  a = new_space_atom(cnt,size>>3,fill);
  a->align = 1;
  add_atom(0,a);
  return a;
}


static void handle_space(char *s,int size)
{
  do_space(size,parse_expr_tmplab(&s),0);
}


static void handle_uspace(char *s,int size)
{
  atom *a = do_space(size,parse_expr_tmplab(&s),0);
  a->content.sb->flags |= SPC_UNINITIALIZED;
}


static void handle_dbss(char *s,int size)
{
  atom *a = do_space(size,parse_expr_tmplab(&s),0);
  a->content.sb->flags |= SPC_DATABSS;
}


static char *read_sec_attr(char *attr,char *s,uint32_t *mem)
{
  char *type = s;

  if (!(s = skip_identifier(s))) {
    syntax_error(10);  /* identifier expected */
    return NULL;
  }

  if ((s-type==3 || s-type==5) && !strnicmp(type,"bss",3))
    strcpy(attr,bss_type);
  else if ((s-type==4 || s-type==6) && !strnicmp(type,"data",4))
    strcpy(attr,data_type);
  else if ((s-type==4 || s-type==6) &&
           (!strnicmp(type,"code",4) || !strnicmp(type,"text",4)))
    strcpy(attr,code_type);
  else {
    syntax_error(13);  /* illegal section type */
    return NULL;
  }

  if (s-type==5 || s-type==6) {
    if (*(s-2) == '_') {
      switch (tolower((unsigned char)*(s-1))) {
        case 'c':
          *mem = 2;  /* AmigaDOS MEMF_CHIP */
          break;
        case 'f':
          *mem = 4;  /* AmigaDOS MEMF_FAST */
          break;
        case 'p':
          break;
        default:
          syntax_error(13);
          return NULL;
      }
    }
    else {
      syntax_error(13);  /* illegal section type */
      return NULL;
    }
  }

  s = skip(s);
  if (*s == ',') {
    /* read optional memory type */
    taddr mc;

    s = skip(s+1);
    type = s;

    /* check for "chip" or "fast" memory type (AmigaDOS) */
    if (s = skip_identifier(s)) {
      if (s-type==4 && !strnicmp(type,"chip",4)) {
        *mem = 2;  /* AmigaDOS MEMF_CHIP */
        return skip(s);
      }
      else if (s-type==4 && !strnicmp(type,"fast",4)) {
        *mem = 4;  /* AmigaDOS MEMF_FAST */
        return skip(s);
      }
    }

    /* try to read a numerical memory type constant */
    s = type;
    mc = parse_constexpr(&type);
    if (type>s && mc!=0)
      *mem = (uint32_t)mc;
    else
      syntax_error(15);  /* illegal memory type */
    s = skip(type);
  }

  return s;
}


static section *edtasmsection(section *sec,uint32_t mem)
/* EDTASM-syntax specific section initializations on a new section */
{
  try_end_rorg();  /* end a potential ORG block */

  /* set optional memory attributes */
  if (sec->memattr!=0 && mem!=(uint32_t)sec->memattr)
    syntax_error(27,sec->name);  /* modified memory attributes for section */
  else
    sec->memattr = mem;

  return sec;
}


static void handle_section(char *s)
{
  char attr[32];
  char *name;
  strbuf *buf;
  uint32_t mem = 0;

  /* read section name */
  if (buf = parse_name(0,&s))
    name = buf->str;
  else
    return;

  if (*s == ',') {
    /* read section type and memory attributes */
    s = read_sec_attr(attr,skip(s+1),&mem);
  }
  else if (unnamed_sections) {
    /* only name is given - guess type from the name (i.e. name is type) */
    if (!stricmp(name,"data")) {
      strcpy(attr,data_type);
      name = data_name;
    }
    else if (!stricmp(name,"bss")) {
      strcpy(attr,bss_type);
      name = bss_name;
    }
    else if (!stricmp(name,"code") || !stricmp(name,"text")) {
      strcpy(attr,code_type);
      name = code_name;
    }
    else {
      syntax_error(13);  /* illegal section type */
      s = NULL;
    }
  }
  else {
    /* otherwise a missing section type defaults to CODE */
    strcpy(attr,code_type);
  }
  if (s)
    set_section(edtasmsection(new_section(name,attr,1),mem));
}


static void handle_offset(char *s)
{
  taddr offs;

  if (!ISEOL(s))
    offs = parse_constexpr(&s);
  else
    offs = -1;  /* use last offset */

  try_end_rorg();
  switch_offset_section(NULL,offs);
}


static void nameattrsection(char *secname,char *sectype,uint32_t mem)
/* switch to a section called secname, with attributes sectype+addattr */
{
  set_section(edtasmsection(new_section(secname,sectype,1),mem));
}

static void handle_csec(char *s)
{
  nameattrsection(code_name,code_type,0);
}

static void handle_dsec(char *s)
{
  nameattrsection(data_name,data_type,0);
}

static void handle_bss(char *s)
{
  nameattrsection(bss_name,bss_type,0);
}

static void handle_codec(char *s)
{
  nameattrsection("CODE_C",code_type,2);  /* AmigaDOS MEMF_CHIP */
}

static void handle_codef(char *s)
{
  nameattrsection("CODE_F",code_type,4);  /* AmigaDOS MEMF_FAST */
}

static void handle_datac(char *s)
{
  nameattrsection("DATA_C",data_type,2);  /* AmigaDOS MEMF_CHIP */
}

static void handle_dataf(char *s)
{
  nameattrsection("DATA_F",data_type,4);  /* AmigaDOS MEMF_FAST */
}

static void handle_bssc(char *s)
{
  nameattrsection("BSS_C",bss_type,2);  /* AmigaDOS MEMF_CHIP */
}

static void handle_bssf(char *s)
{
  nameattrsection("BSS_F",bss_type,4);  /* AmigaDOS MEMF_FAST */
}


static void handle_org(char *s)
{
  if (*s == '*') {    /*  "* = * + <expr>" reserves bytes */
    s = skip(s+1);
    if (*s == '+')
      handle_uspace(skip(s+1),8);
    else
      syntax_error(7);  /* syntax error */
  }
  else {
    if (current_section!=NULL &&
        (!(current_section->flags & ABSOLUTE) ||
          (current_section->flags & IN_RORG)))
      start_rorg(parse_constexpr(&s));
    else
      set_section(new_org(parse_constexpr(&s)));
  }
}


static void handle_rorg(char *s)
{
  expr *offs = parse_expr_tmplab(&s);
  expr *fill;

  s = skip(s);
  if (*s == ',') {
    /* may be followed by an optional fill-value */
    s = skip(s+1);
    fill = parse_expr_tmplab(&s);
  }
  else
    fill = NULL;
  add_atom(0,new_roffs_atom(offs,fill));
}


static void do_bind(char *s,unsigned bind)
{
  symbol *sym;
  strbuf *name;

  do {
    s = skip(s);
    if (!(name = parse_identifier(0,&s))) {
      syntax_error(10);  /* identifier expected */
      return;
    }
    sym = new_import(name->str);
    if ((sym->flags & (EXPORT|WEAK|NEAR)) != 0 &&
        (sym->flags & (EXPORT|WEAK|NEAR)) != (bind & (EXPORT|WEAK|NEAR))) {
      general_error(62,sym->name,get_bind_name(sym)); /* binding already set */
    }
    else {
      sym->flags |= bind;
      if ((bind & XREF)!=0 && sym->type!=IMPORT)
        general_error(85,sym->name);  /* xref must not be defined already */
    }
    s = skip(s);
  }
  while (*s++ == ',');
}


static void handle_xref(char *s)
{
  do_bind(s,EXPORT|XREF);
}


static void handle_xdef(char *s)
{
  do_bind(s,EXPORT|XDEF);
}


static void handle_global(char *s)
{
  do_bind(s,EXPORT);
}


static void handle_weak(char *s)
{
  do_bind(s,WEAK);
}


static void handle_nref(char *s)
{
  do_bind(s,EXPORT|XREF|NEAR);
}


static void handle_comm(char *s)
{
  strbuf *name = parse_identifier(0,&s);
  symbol *sym;
  taddr sz = 4;

  if (name == NULL) {
    syntax_error(10);  /* identifier expected */
    return;
  }
  sym = new_import(name->str);
  sym->flags |= COMMON;

  s = skip(s);
  if (*s == ',') {
    s = skip(s+1);
    sz = parse_constexpr(&s);
  }
  else
    syntax_error(9);  /* , expected */

  sym->size = number_expr(sz);
  sym->align = 4;
}


static void handle_data(char *s,int size)
{
  /* size is negative for floating point data! */
  for (;;) {
    char *opstart = s;
    operand *op;
    dblock *db = NULL;

    if (OPSZ_BITS(size)==8 && (*s=='\"' || *s=='\'')) {
      if (db = parse_string(&opstart,*s,8)) {
        add_atom(0,new_data_atom(db,1));
        s = opstart;
      }
    }
    if (!db) {
      op = new_operand();
      s = skip_operand(s);
      if (parse_operand(opstart,s-opstart,op,DATA_OPERAND(size))) {
        atom *a;

        a = new_datadef_atom(OPSZ_BITS(size),op);
        a->align = 1;
        add_atom(0,a);
      }
      else
        syntax_error(8);  /* invalid data operand */
    }

    s = skip(s);
    if (*s == ',')
      s = skip(s+1);
    else {
      eol(s);
      break;
    }
  }
}


static void handle_d8(char *s)
{
  handle_data(s,8);
}


static void handle_d16(char *s)
{
  handle_data(s,16);
}


static void handle_d32(char *s)
{
  handle_data(s,32);
}


static void handle_d64(char *s)
{
  handle_data(s,64);
}


#if FLOAT_PARSER
static void handle_f32(char *s)
{
  handle_data(s,OPSZ_FLOAT|32);
}


static void handle_f64(char *s)
{
  handle_data(s,OPSZ_FLOAT|64);
}


static void handle_f96(char *s)
{
  handle_data(s,OPSZ_FLOAT|96);
}


static void handle_fpd(char *s)
{
  handle_data(s,OPSZ_FLOAT|97);  /* packed decimal */
}
#endif


static void do_alignment(taddr align,taddr offset,size_t pad,expr *fill)
{
  atom *a = new_space_atom(number_expr(offset),pad,fill);

  a->align = align;
  add_atom(0,a);
}


static void handle_cnop(char *s)
{
  taddr offset;
  taddr align=1;

  offset = parse_constexpr(&s);
  s = skip(s);
  if (*s == ',') {
    s = skip(s+1);
    align = parse_constexpr(&s);
  }
  else
    syntax_error(9);  /* , expected */

  /* Simple alignment for EDTASM */
  do_alignment(align,offset,1,NULL);
}


static void handle_align(char *s)
{
  do_alignment(1<<parse_constexpr(&s),0,1,NULL);
}


static void handle_even(char *s)
{
  do_alignment(2,0,1,NULL);
}


static void handle_odd(char *s)
{
  do_alignment(2,1,1,NULL);
}


static void handle_block(char *s,int size)
{
  expr *cnt,*fill=0;

  cnt = parse_expr_tmplab(&s);
  s = skip(s);
  if (*s == ',') {
    s = skip(s+1);
    fill = parse_expr_tmplab(&s);
  }
  do_space(size,cnt,fill);
}


static void handle_xspc8(char *s)
{
  handle_dbss(s,8);
}


static void handle_xspc16(char *s)
{
  handle_dbss(s,16);
}


static void handle_xspc32(char *s)
{
  handle_dbss(s,32);
}


static void handle_xspc64(char *s)
{
  handle_dbss(s,64);
}


static void handle_xspc96(char *s)
{
  handle_dbss(s,96);
}


static void handle_spc8(char *s)
{
  handle_space(s,8);
}


static void handle_spc16(char *s)
{
  handle_space(s,16);
}


static void handle_spc32(char *s)
{
  handle_space(s,32);
}


static void handle_spc64(char *s)
{
  handle_space(s,64);
}


static void handle_spc96(char *s)
{
  handle_space(s,96);
}


static void handle_blk8(char *s)
{
  handle_block(s,8);
}


static void handle_blk16(char *s)
{
  handle_block(s,16);
}


static void handle_blk32(char *s)
{
  handle_block(s,32);
}


static void handle_blk64(char *s)
{
  handle_block(s,64);
}


static void handle_blk96(char *s)
{
  handle_block(s,96);
}


#ifdef VASM_CPU_M68K
static void handle_reldata(char *s,int size)
{
  for (;;) {
    char *opstart = s;
    operand *op;

    op = new_operand();
    s = skip_operand(s);
    if (parse_operand(opstart,s-opstart,op,DATA_OPERAND(size))) {
      if (op->value[0]) {
        expr *tmplab,*new;
        atom *a;

        tmplab = new_expr();
        tmplab->type = SYM;
        tmplab->c.sym = new_tmplabel(0);
        add_atom(0,new_label_atom(tmplab->c.sym));
        /* subtract the current pc value from all data expressions */
        new = make_expr(SUB,op->value[0],tmplab);
        simplify_expr(new);
        op->value[0] = new;
        a = new_datadef_atom(OPSZ_BITS(size),op);
        if (!align_data)
          a->align = 1;
        add_atom(0,a);
      }
      else
        ierror(0);
    }
    else
      syntax_error(8);  /* invalid data operand */
    s = skip(s);
    if (*s == ',')
      s = skip(s+1);
    else
      break;
  }
}


static void handle_reldata8(char *s)
{
  handle_reldata(s,8);
}


static void handle_reldata16(char *s)
{
  handle_reldata(s,16);
}


static void handle_reldata32(char *s)
{
  handle_reldata(s,32);
}
#endif


/* EDTASM-specific directive handlers */

static void handle_fcc(char *s)
{
  char delimiter;
  char *start;
  dblock *db;

  s = skip(s);
  if (!*s || ISEOL(s)) {
    syntax_error(5);  /* missing operand */
    return;
  }

  delimiter = *s++;
  start = s;

  /* Find closing delimiter - no escape sequences in EDTASM */
  while (*s && *s != delimiter) {
    s++;
  }

  if (*s != delimiter) {
    syntax_error(21,delimiter);  /* missing delimiter */
    return;
  }

  /* Copy string literally - backslash is just a regular character */
  db = new_dblock();
  db->size = s - start;
  db->data = mymalloc(db->size);
  memcpy(db->data, start, db->size);
  add_atom(0, new_data_atom(db, 1));

  s++;  /* Skip closing delimiter */
  eol(s);
}


static void handle_rmb(char *s)
{
  handle_space(s, 8);
}


static void handle_fcs(char *s)
{
  /* FCS - Form Constant String (high bit set on last char)
     Same as FCC but sets bit 7 on the last character */
  char delimiter;
  char *start;
  dblock *db;
  int len;

  s = skip(s);
  if (!*s || ISEOL(s)) {
    syntax_error(5);  /* missing operand */
    return;
  }

  delimiter = *s++;
  start = s;

  /* Find closing delimiter - no escape sequences in EDTASM */
  while (*s && *s != delimiter) {
    s++;
  }

  if (*s != delimiter) {
    syntax_error(21,delimiter);  /* missing delimiter */
    return;
  }

  /* Copy string literally and set high bit on last character */
  len = s - start;
  db = new_dblock();
  db->size = len;
  db->data = mymalloc(db->size);
  memcpy(db->data, start, len);
  if (len > 0)
    db->data[len - 1] |= 0x80;  /* Set bit 7 on last character */
  add_atom(0, new_data_atom(db, 1));

  s++;  /* Skip closing delimiter */
  eol(s);
}


static void handle_mod(char *s)
{
  /* MOD modsize,modname,type,attr,start,memsiz
     OS-9 module header directive - for now, just skip parameters
     Proper implementation would require OS-9 module format support */
  /* TODO: Implement OS-9 module header generation */
  /* Skip all parameters until end of line */
  while (!ISEOL(s))
    s++;
}


static void handle_cond(char *s)
{
  expr *condexp = parse_expr_tmplab(&s);
  taddr val;

  if (eval_expr(condexp, &val, NULL, 0)) {
    cond_if(val != 0);
  } else {
    general_error(30);  /* expression must be constant */
    cond_if(0);
  }
  free_expr(condexp);
  eol(s);
}


static void handle_opt(char *s)
{
  char *name;

  for (;;) {
    s = skip(s);
    if (ISEOL(s))
      break;

    name = s;
    while (isalpha((unsigned char)*s))
      s++;

    if (s > name) {
      int len = s - name;

      if (len == 2 && !strnicmp(name, "mc", 2))
        opt_mc = 1;
      else if (len == 4 && !strnicmp(name, "nomc", 4))
        opt_mc = 0;
      else if (len == 2 && !strnicmp(name, "md", 2))
        opt_md = 1;
      else if (len == 4 && !strnicmp(name, "nomd", 4))
        opt_md = 0;
      else if (len == 3 && !strnicmp(name, "mex", 3))
        opt_mex = 1;
      else if (len == 5 && !strnicmp(name, "nomex", 5))
        opt_mex = 0;
      else if (len == 1 && !strnicmp(name, "l", 1))
        opt_list = 1;
      else if (len == 3 && !strnicmp(name, "nol", 3))
        opt_list = 0;
      else
        syntax_error(11);  /* directive has no effect */
    }

    s = skip(s);
    if (*s == ',')
      s++;
    else if (!ISEOL(s))
      syntax_error(9);  /* , expected */
  }
}


static void handle_end(char *s)
{
  parse_end = 1;
}


static void handle_fail(char *s)   
{ 
  add_or_save_atom(new_assert_atom(NULL,NULL,mystrdup(s)));
}


static void handle_assert(char *s)
{
  char *expstr,*msgstr;
  size_t explen;
  expr *aexp;

  msgstr = NULL;
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

  add_or_save_atom(new_assert_atom(aexp,cnvstr(expstr,explen),msgstr));
}


static void handle_idnt(char *s)
{
  strbuf *name;

  if (name = parse_name(0,&s))
    setfilename(mystrdup(name->str));
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


static void handle_edtasm_title(char *s)
{
  /* EDTASM TITLE directive - just ignored for compatibility */
  eol(s);
}


static void handle_plen(char *s)
{
  int plen = (int)parse_constexpr(&s);

  listlinesperpage = plen > 12 ? plen : 12;
}


static void handle_edtasm_page(char *s)
{
  /* EDTASM PAGE directive - just ignored for compatibility */
  eol(s);
}


static void handle_nopage(char *s)
{
  listformfeed = 0;
}


static void handle_output(char *s)
{
  strbuf *buf;

  if (buf = parse_name(0,&s)) {
    if (*(buf->str)=='.') {
      char *p;
      int outlen;

      if (!outname)
        outname = inname ? inname : "a";
      if (p = strrchr(outname,'.'))
        outlen = p - outname;
      else
        outlen = strlen(outname);
      p = mymalloc(outlen+buf->len+1);
      memcpy(p,outname,outlen);
      strcpy(p+outlen,buf->str);
      outname = p;
    }
    else if (!outname)
      outname = mystrdup(buf->str);
  }
}


static void handle_dsource(char *s)
{
  strbuf *name;

  if (name = parse_name(0,&s))
    setdebugname(mystrdup(name->str));
}


static void handle_debug(char *s)
{
  atom *a = new_srcline_atom((int)parse_constexpr(&s));

  add_atom(0,a);
}


static void handle_msource(char *s)
{
  if (!strnicmp(s,"on",2))
    msource_disable = 0;
  else if (!strnicmp(s,"off",3))
    msource_disable = 1;
  else
    msource_disable = atoi(s) == 0;
}


static void handle_vdebug(char *s)
{
  atom *a = new_atom(VASMDEBUG,0);

  add_atom(0,a);
}


static void handle_incdir(char *s)
{
  strbuf *name;

  while (name = parse_name(0,&s)) {
    new_include_path(name->str);
    if (*s != ',') {
      return;
    }
    s = skip(s+1);
  }
  syntax_error(5);
}


static void handle_include(char *s)
{
  strbuf *name;

  if (name = parse_name(0,&s)) {
    include_source(name->str);
  }
}


static void handle_incbin(char *s)
{
  strbuf *name;
  taddr offs = 0;
  taddr length = 0;

  if (name = parse_name(0,&s)) {
    s = skip(s);
    if (*s == ',') {
      /* We have an offset */
      s = skip(s + 1);
      offs = parse_constexpr(&s);
      s = skip(s);
      if (*s == ',') {
        /* We have a length */
        s = skip(s + 1);
        length = parse_constexpr(&s);
      }
    }
    include_binary_file(name->str,offs,length);
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
  new_repeat(cnt<0?0:cnt,itername,NULL,rept_dirlist,endr_dirlist);
}


static void handle_endr(char *s)
{
  syntax_error(12,"endr","rept");  /* unexpected endr without rept */
}


static void handle_macro(char *s)
{
  strbuf *name;

  if (name = parse_identifier(0,&s))
    new_macro(name->str,macro_dirlist,endm_dirlist,NULL);
  else
    syntax_error(10);  /* identifier expected */
}


static void handle_endm(char *s)
{
  syntax_error(12,"endm","macro");  /* unexpected endm without macro */
}


static void handle_mexit(char *s)
{
  leave_macro();
}


#if STRUCT
static void handle_struct(char *s)
{
  strbuf *name;

  if (name = parse_identifier(0,&s)) {
    s = skip(s);
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
}
#endif


static void handle_rem(char *s)
{
  new_repeat(0,NULL,NULL,NULL,erem_dirlist);
}


static void handle_erem(char *s)
{
  syntax_error(12,"erem","rem");  /* unexpected erem without rem */
}


static void handle_ifb(char *s)
{
  cond_if(ISEOL(s));
}

static void handle_ifnb(char *s)
{
  cond_if(!ISEOL(s));
}

static void ifc(char *s,int b)
{
  strbuf *str1,*str2;
  int result;

  str1 = parse_name(0,&s);
  if (str1!=NULL && *s==',') {
    s = skip(s+1);
    if (str2 = parse_name(1,&s)) {
      result = strcmp(str1->str,str2->str) == 0;
      cond_if(result == b);
      return;
    }
  }
  syntax_error(5);  /* missing operand */
}

static void handle_ifc(char *s)
{
  ifc(s,1);
}

static void handle_ifnc(char *s)
{
  ifc(s,0);
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
}

static void handle_ifd(char *s)
{
  ifdef(s,1);
}

static void handle_ifnd(char *s)
{
  ifdef(s,0);
}

static void ifmacro(char *s,int b)
{
  char *name = s;
  int result;

  if (s = skip_identifier(s)) {
    result = find_macro(name,s-name) != NULL;
    cond_if(result == b);
  }
  else
    syntax_error(10);  /*identifier expected */
}

static void handle_ifmacrod(char *s)
{
  ifmacro(s,1);
}

static void handle_ifmacrond(char *s)
{
  ifmacro(s,0);
}

static int eval_ifexp(char **s,int c)
{
  expr *condexp = parse_expr_tmplab(s);
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
  free_expr(condexp);
  return b;
}

static void ifexp(char *s,int c)
{
  cond_if(eval_ifexp(&s,c));
}

/* Move line_ptr to the end of the string if the parsing should stop,
   otherwise move line_ptr after the iif directive and the expression
   so the parsing can continue and return the new line_ptr.
   The string is never modified. */
static char *handle_iif(char *line_ptr)
{
  if (strnicmp(line_ptr,"iif",3) == 0 &&
      isspace((unsigned char)line_ptr[3])) {
    char *expr_copy,*expr_end;
    int condition;
    size_t expr_len;

    line_ptr += 3;

    /* Move the line ptr to the beginning of the iif expression. */
    line_ptr = skip(line_ptr);

    /* As eval_ifexp() may modify the input string, duplicate
       it for the case when the parsing should continue. */
    expr_copy = mystrdup(line_ptr);
    expr_end = expr_copy;
    condition = eval_ifexp(&expr_end,1);
    expr_len = expr_end - expr_copy;
    myfree(expr_copy);

    if (condition) {
      /* Parsing should continue after the expression, from the next field. */
      line_ptr += expr_len;
      line_ptr = skip(line_ptr);
    } else {
      /* Parsing should stop, move ptr to the end of the line. */
      line_ptr += strlen(line_ptr);
    }
  }
  return line_ptr;
}

static void handle_ifeq(char *s)
{
  ifexp(s,0);
}

static void handle_ifne(char *s)
{
  ifexp(s,1);
}

static void handle_ifgt(char *s)
{
  ifexp(s,2);
}

static void handle_ifge(char *s)
{
  ifexp(s,3);
}

static void handle_iflt(char *s)
{
  ifexp(s,4);
}

static void handle_ifle(char *s)
{
  ifexp(s,5);
}

static void handle_ifp1(char *s)
{
  cond_if(1);        /* vasm parses only once, so we assume true */
  syntax_error(25);  /* and warn about it */
}

static void handle_ifp2(char *s)
{
  cond_if(0);        /* vasm parses only once, so we assume false */
  syntax_error(26);  /* and warn about it */
}

static void handle_else(char *s)
{
  cond_skipelse();
}

static void handle_elseif(char *s)
{
  cond_skipelse();
}

static void handle_endif(char *s)
{
  cond_endif();
}


static void handle_rsreset(char *s)
{
  new_abs(rs_name,number_expr(0));
}

/* EDTASM helper: convert '.' to '*' for current PC symbol
   Both '.' and '*' represent current PC in EDTASM */
static char *convert_dot_to_star(char *s)
{
  static char linebuf[4096];
  char *d = linebuf;
  char *start = s;

  while (*s && !ISEOL(s) && (d - linebuf) < 4095) {
    if (*s == '.' && (s == start || !ISIDCHAR(*(s-1))) && !ISIDCHAR(*(s+1))) {
      /* Standalone dot - convert to asterisk */
      *d++ = '*';
      s++;
    }
    else {
      *d++ = *s++;
    }
  }
  *d = '\0';
  return linebuf;
}

/* EDTASM helper: convert #'x to ASCII value
   EDTASM uses #'x (without closing quote) for character literals */
static char *convert_char_literals(char *s)
{
  static char linebuf2[4096];
  char *d = linebuf2;

  /* Process the line character by character, but skip over quoted strings
     to avoid treating comment characters inside strings as end-of-line */
  while (*s && (d - linebuf2) < 4090) {
    /* Check for start of quoted string - skip over it without processing */
    if (*s == '"' || *s == '\'') {
      char delimiter = *s;
      *d++ = *s++;  /* Copy opening delimiter */

      /* Copy string contents until closing delimiter */
      while (*s && *s != delimiter && (d - linebuf2) < 4090) {
        *d++ = *s++;
      }

      /* Copy closing delimiter if present */
      if (*s == delimiter && (d - linebuf2) < 4090) {
        *d++ = *s++;
      }
      continue;
    }

    /* Check for end of line (excluding comment chars inside strings) */
    if (*s == '\0' || iscomment(s))
      break;

    /* Process #'x character literals */
    if (*s == '#' && *(s+1) == '\'') {
      /* Found #' - convert character literal to #$xx */
      char ch = *(s+2);
      if (ch) {
        /* Check for EDTASM style #'x (no closing quote) */
        if (*(s+3) != '\'' &&
            (isspace((unsigned char)*(s+3)) || *(s+3) == ',' ||
             *(s+3) == ';' || iscomment(s+3) || *(s+3) == ')' || *(s+3) == '\0')) {
          /* EDTASM single character literal: convert #'x to #$xx */
          sprintf(d, "#$%02x", (unsigned char)ch);
          d += 4;
          s += 3;
        }
        /* Check for standard quoted literal #'x' */
        else if (*(s+3) == '\'') {
          /* Standard quoted literal like #'#' - convert to #$xx */
          sprintf(d, "#$%02x", (unsigned char)ch);
          d += 4;
          s += 4;  /* Skip #'x' */
        }
        else {
          /* Not a valid character literal - copy as-is */
          *d++ = *s++;
        }
      }
      else {
        /* Empty character literal - copy as-is */
        *d++ = *s++;
      }
    }
    else {
      *d++ = *s++;
    }
  }
  *d = '\0';

  return linebuf2;
}

static void handle_rsset(char *s)
{
  s = convert_dot_to_star(s);
  new_abs(rs_name,parse_expr_tmplab(&s));
}

static void handle_rseven(char *s)
{
  setoffset_align(rs_name,1,2);
}

static void handle_clrfo(char *s)
{
  new_abs(fo_name,number_expr(0));
}

static void handle_setfo(char *s)
{
  new_abs(fo_name,parse_expr_tmplab(&s));
}

static void handle_rs8(char *s)
{
  new_setoffset_size(NULL,rs_name,&s,1,1);
}

static void handle_rs16(char *s)
{
  new_setoffset_size(NULL,rs_name,&s,1,2);
}

static void handle_rs32(char *s)
{
  new_setoffset_size(NULL,rs_name,&s,1,4);
}

static void handle_rs64(char *s)
{
  new_setoffset_size(NULL,rs_name,&s,1,8);
}

static void handle_rs96(char *s)
{
  new_setoffset_size(NULL,rs_name,&s,1,12);
}

static void handle_fo8(char *s)
{
  new_setoffset_size(NULL,fo_name,&s,-1,1);
}

static void handle_fo16(char *s)
{
  new_setoffset_size(NULL,fo_name,&s,-1,2);
}

static void handle_fo32(char *s)
{
  new_setoffset_size(NULL,fo_name,&s,-1,4);
}

static void handle_fo64(char *s)
{
  new_setoffset_size(NULL,fo_name,&s,-1,8);
}

static void handle_fo96(char *s)
{
  new_setoffset_size(NULL,fo_name,&s,-1,12);
}

static void handle_cargs(char *s)
{
  char *name;
  expr *offs;
  taddr size;

  if (*s == '#') {
    /* offset given */
    ++s;
    offs = parse_expr_tmplab(&s);
    s = skip(s);
    if (*s != ',')
      syntax_error(9);  /* , expected */
    else
      s = skip(s+1);
  }
  else
    offs = number_expr(bytespertaddr);  /* default offset */

  for (;;) {

    if (!(name = parse_symbol(&s))) {
      syntax_error(10);  /* identifier expected */
      break;
    }

    if (!check_symbol(name)) {
      /* define new stack offset symbol */
      new_abs(name,copy_tree(offs));
    }

    /* increment offset by given size */
    if (*s == '.') {
      ++s;
      switch (tolower((unsigned char)*s)) {
        case 'b':
        case 'w':
          size = 2;
          ++s;
          break;
        case 'l':
          size = 4;
          ++s;
          break;
        default:
          size = 2;
          syntax_error(1);  /* invalid extension */
          break;
      }
    }
    else
      size = 2;

    s = skip(s);
    if (*s != ',')  /* define another offset symbol? */
      break;

    offs = make_expr(ADD,offs,number_expr(size));
    simplify_expr(offs);
    s = skip(s+1);
  }

  /* offset expression was copied, so we can free it now */
  if (offs)
    free_expr(offs);
}

static void handle_printt(char *s)
{
  strbuf *txt;

  while (txt = parse_name(0,&s)) {
    add_or_save_atom(new_text_atom(mystrdup(txt->str)));
    s = skip(s);
    if (*s != ',')
      break;
    add_or_save_atom(new_text_atom(NULL));  /* new line */
    s = skip(s+1);
  }
  add_or_save_atom(new_text_atom(NULL));  /* new line */
}

static void handle_printv(char *s)
{
  expr *x;

  for (;;) {
    x = parse_expr(&s);
    add_or_save_atom(new_text_atom("$"));
    add_or_save_atom(new_expr_atom(x,PEXP_HEX,32));
    add_or_save_atom(new_text_atom(" "));
    add_or_save_atom(new_expr_atom(x,PEXP_SDEC,32));
    add_or_save_atom(new_text_atom(" \""));
    add_or_save_atom(new_expr_atom(x,PEXP_ASC,32));
    add_or_save_atom(new_text_atom("\" %"));
    add_or_save_atom(new_expr_atom(x,PEXP_BIN,32));
    add_or_save_atom(new_text_atom(NULL));  /* new line */
    s = skip(s);
    if (*s != ',')
      break;
    s = skip(s+1);
  }    
}

static void handle_echo(char *s)
{
  for (;;) {
    if (*s=='\"' || *s=='\'') {
      strbuf *txt = parse_name(0,&s);
      if (txt)
        add_or_save_atom(new_text_atom(mystrdup(txt->str)));
    }
    else {
      add_or_save_atom(new_expr_atom(parse_expr(&s),PEXP_SDEC,32));
    }
    s = skip(s);
    if (*s != ',')
      break;
    s = skip(s+1);
  }
  add_or_save_atom(new_text_atom(NULL));  /* new line */
}

static void handle_showoffset(char *s)
{
  strbuf *txt;

  if (txt = parse_name(0,&s))
    add_or_save_atom(new_text_atom(mystrdup(txt->str)));
  add_or_save_atom(new_text_atom(" "));
  add_or_save_atom(new_expr_atom(curpc_expr(),PEXP_HEX,32));
  add_or_save_atom(new_text_atom(NULL));  /* new line */
}

static void handle_dummy_expr(char *s)
{
  parse_expr(&s);
  syntax_error(11);  /* directive has no effect */
}

static void handle_dummy_cexpr(char *s)
{
  parse_constexpr(&s);
  syntax_error(11);  /* directive has no effect */
}

static void handle_noop(char *s)
{
  syntax_error(11);  /* directive has no effect */
}

static void handle_comment(char *s)
{
  /* handle Atari-specific "COMMENT HEAD=<expr>" to define the tos-flags */
  if (!strnicmp(s,"HEAD=",5)) {
    s += 5;
    new_abs(" TOSFLAGS",parse_expr_tmplab(&s));
  }
  /* otherwise it's just a comment to be ignored */
}

static void handle_local(char *s)
{
  sprintf(inl_lab_name,INLLABFMT,local_id++);
  set_last_global_label(mystrdup(inl_lab_name));
}

static void handle_inline(char *s)
{
  const char *last;

  if (inline_stack_index < INLSTACKSIZE) {
    sprintf(inl_lab_name,INLLABFMT,local_id);
    last = set_last_global_label(inl_lab_name);
    if (inline_stack_index == 0)
      saved_last_global_label = last;
    inline_stack[inline_stack_index++] = local_id++;
  }
  else
    syntax_error(22,INLSTACKSIZE);  /* maximum inline nesting depth exceeded */
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
    syntax_error(20);  /* einline without inline */
}

static void handle_pushsect(char *s)
{
  push_section();
  eol(s);
}

static void handle_popsect(char *s)
{
  pop_section();
  eol(s);
}


/* EDTASM directive table */
struct {
  const char *name;
  unsigned flags;
  void (*func)(char *);
} directives[] = {
  /* Basic directives */
  "org",0,handle_org,
  "end",0,handle_end,
  "equ",0,handle_dummy_expr,  /* handled in parse() */
  "set",0,handle_dummy_expr,  /* handled in parse() */

  /* EDTASM data directives */
  "fcb",0,handle_d8,
  "fdb",0,handle_d16,
  "fcc",0,handle_fcc,
  "fcs",0,handle_fcs,
  "rmb",0,handle_rmb,

  /* Sections */
  "section",0,handle_section,
  "mod",0,handle_mod,          /* OS-9 module header (stub) */

  /* Conditional assembly */
  "if",DT_IF,handle_cond,         /* IF expr - same as COND */
  "cond",DT_IF,handle_cond,
  "ifeq",DT_IF,handle_ifeq,
  "ifne",DT_IF,handle_ifne,
  "ifgt",DT_IF,handle_ifgt,
  "ifge",DT_IF,handle_ifge,
  "iflt",DT_IF,handle_iflt,
  "ifle",DT_IF,handle_ifle,
  "else",DT_ELSE,handle_else,
  "endc",DT_ENDIF,handle_endif,
  "endif",DT_ENDIF,handle_endif,

  /* Macros */
  "macro",0,handle_macro,
  "endm",0,handle_endm,

  /* Include */
  "include",0,handle_include,

  /* OS-9 Module */
  "emod",0,handle_end,   /* EMOD (end module) is same as END */

  /* Listing control */
  "opt",0,handle_opt,
  "page",0,handle_edtasm_page,
  "title",0,handle_edtasm_title,
  "list",0,handle_list,
  "nolist",0,handle_nolist,
};


size_t dir_cnt = sizeof(directives) / sizeof(directives[0]);


/* checks for a valid directive, and return index when found, -1 otherwise */
static int check_directive(char **line)
{
  char *s,*name;
  hashdata data;

  s = skip(*line);
  if (!ISIDSTART(*s))
    return -1;
  name = s++;
  while (ISIDCHAR(*s) || *s=='.')
    s++;
  if (!find_namelen_nc(dirhash,name,s-name,&data))
    return -1;
  *line = s;
  return data.idx;
}

/* Handles assembly directives;
   returns non-zero if the parsing of the line should stop. */
static int handle_directive(char *line)
{
  int idx = check_directive(&line);

  if (idx >= 0) {
    directives[idx].func(skip(line));
    return 1;
  }
  return 0;
}


static int offs_directive(char *s,char *name)
{
  int len = strlen(name);
  char *d = s + len;

  return !strnicmp(s,name,len) &&
         ((isspace((unsigned char)*d) || ISEOL(d)) ||
          (*d=='.' && (isspace((unsigned char)*(d+2))||ISEOL(d+2))));
}


#if FLOAT_PARSER
static symbol *fequate(char *labname,char **s)
{
  char x = tolower((unsigned char)**s);

  if (x=='s' || x=='d' || x=='x' || x=='p') {
    *s = skip(*s + 1);
    return new_equate(labname,parse_expr_float(s));
  }
  syntax_error(1);  /* illegal extension */
  return NULL;
}
#endif


static char *skip_local(char *p)
{
  char *s;

  if (ISIDSTART(*p) || isdigit((unsigned char)*p)) {  /* may start with digit */
    s = p++;
    while (ISIDCHAR(*p))
      p++;
    p = CHKIDEND(s,p);
  }
  else
    p = NULL;

  return p;
}


#if STRUCT
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
      s = skip_operand(s);
      opl = s - opp;

      if (opl > 0) {
        /* initialize this atom with a new expression */

        if (p->type == DATADEF) {
          /* parse a new data operand of the declared bitsize */
          operand *op;

          op = new_operand();
          if (parse_operand(opp,opl,op,
                            DATA_OPERAND(p->content.defb->bitsize))) {
            new = new_datadef_atom(p->content.defb->bitsize,op);
            new->align = 1;
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
                myfree(strdb);
              }
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

  return 1;
}
#endif


void parse(void)
{
  char *s,*line,*inst,*labname;
  char *ext[MAX_QUALIFIERS?MAX_QUALIFIERS:1];
  char *op[MAX_OPERANDS];
  int ext_len[MAX_QUALIFIERS?MAX_QUALIFIERS:1];
  int op_len[MAX_OPERANDS];
  int ext_cnt,op_cnt,inst_len;
  instruction *ip;

  while (line = read_next_line()) {
    if (parse_end)
      continue;
    /* Preprocess EDTASM-specific syntax: convert #'x to #$xx */
    /* Only preprocess if not inside macro (macro expansions are in cur_src->text) */
    if (!cur_src->macro) {
      line = convert_char_literals(line);
    }
    s = line;

    if (!cond_state()) {
      /* skip source until ELSE or ENDIF */
      int idx;

      /* skip label, when present */
      if (labname = parse_labeldef(&s,0)) {
        if (*s == ':')
          s++;  /* skip double-colon */
      }
      /* advance to directive */
      s = skip(s);
      idx = check_directive(&s);
      if (idx >= 0) {
        switch (directives[idx].flags & DIRF_TYPEMASK) {
          case DT_IF:
            cond_skipif();
            break;
          case DT_ELSE:
            cond_else();
            break;
          case DT_ELIF:
            s = skip(s);
            cond_elseif(eval_ifexp(&s,1));
            break;
          case DT_ENDIF:
            cond_endif();
            break;
        }
      }
      continue;
    }

    if (labname = parse_labeldef(&s,0)) {
      /* we have found a global or local label */
      uint32_t symflags = 0;
      symbol *label;

      if (*s == ':') {
        /* double colon automatically declares label as exported */
        symflags |= EXPORT;
        s++;
      }

      s = handle_iif(skip(s));

      if (!strnicmp(s,"equ",3) && isspace((unsigned char)*(s+3))) {
        s = skip(s+3);
        s = convert_dot_to_star(s);
        label = new_equate(labname,parse_expr_tmplab(&s));
        label->flags |= symflags;
      }
      else if (*s=='=') {
        s = skip(s+1);
        s = convert_dot_to_star(s);
        label = new_equate(labname,parse_expr_tmplab(&s));
        label->flags |= symflags;
      }
      else if (!strnicmp(s,"set",3) && isspace((unsigned char)*(s+3))) {
        /* SET allows redefinitions */
        s = skip(s+3);
        s = convert_dot_to_star(s);
        label = new_abs(labname,parse_expr_tmplab(&s));
      }
      else if (!strnicmp(s,"ttl",3) && isspace((unsigned char)*(s+3))) {
        s = skip(s+3);
        setfilename(labname);
      }
      else if (!strnicmp(s,"macro",5) &&
               (isspace((unsigned char)*(s+5)) || *(s+5)=='\0'
                || *(s+5)==commentchar)) {
        /* reread original label field as macro name, no local macros */
        strbuf *buf;

        s = line;
        if (!(buf = parse_identifier(0,&s)))
          ierror(0);
        new_macro(buf->str,macro_dirlist,endm_dirlist,NULL);
        continue;
      }
#ifdef PARSE_CPU_LABEL
      else if (!PARSE_CPU_LABEL(labname,&s)) {
#else
      else {
#endif
        label = new_labsym(0,labname);
        label->flags |= symflags;
        add_atom(0,new_label_atom(label));
      }
    }

    /* check for directives first */
    s = skip(s);
    if (*s=='\0' || *s=='*' || *s==commentchar)
      continue;

    s = handle_iif(s);

    s = parse_cpu_special(s);
    if (ISEOL(s))
      continue;

    if (handle_directive(s))
      continue;

    s = skip(s);
    if (ISEOL(s))
      continue;

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
#if STRUCT
    if (execute_struct(inst,inst_len,s))
      continue;
#endif

    /* read operands, terminated by comma (unless in parentheses)  */
    op_cnt = 0;
    while (!ISEOL(s) && op_cnt<MAX_OPERANDS) {
      op[op_cnt] = s;
      s = skip_operand(s);
      op_len[op_cnt] = s - op[op_cnt];
      op_cnt++;

      if (allow_spaces) {
        s = skip(s);
        if (*s != ',')
          break;
        else
          s = skip(s+1);
      }
      else {
        if (*s != ',') {
          if (check_comm)
            comment_check(s);
          break;
        }
        s++;
      }
    }
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
      if (allow_spaces && ip->op[0]==NULL && op_cnt!=0)
        syntax_error(6);  /* mnemonic without operands has tokens in op.field */
#endif
      add_atom(0,new_inst_atom(ip));
    }
  }

  cond_check();  /* check for open conditional blocks */
}


/* parse next macro argument */
char *parse_macro_arg(struct macro *m,char *s,
                      struct namelen *param,struct namelen *arg)
{
  arg->len = 0;  /* no argument reference by keyword */
  param->name = s;

  if (*s == '<') {
    /* macro argument enclosed in < ... > */
    param->name++;
    while (*++s != '\0') {
      if (*s == '>') {
        if (*(s+1) == '>') {
          /* convert ">>" into a single ">" by shifting the whole line buffer */
          char *p;

          for (p=s+1; *p!='\0'; p++)
            *(p-1) = *p;
          *(p-1) = '\0';
        }
        else {
          param->len = s - param->name;
          return s + 1;
        }
      }
      else if (*s=='\'' || *s=='\"')
        s = skip_string(s,*s,NULL) - 1;
    }
    param->len = s - param->name;
  }
  else {
    s = skip_operand(s);
    param->len = trim(s) - param->name;
  }

  return s;
}


/* expands arguments and special escape codes into macro context */
int expand_macro(source *src,char **line,char *d,int dlen)
{
  int nc = 0;
  char *s = *line;

  if (*s++ == '\\' && *s == '\\') {
    /* EDTASM requires double backslash for macro expansion */
    s++;  /* Skip second backslash */

    if (*s >= '1' && *s <= '9') {
      /* \\1 through \\9 : insert macro parameter 1..9 */
      int pnum = *s++ - '0';
      if (src->num_params > 0 && pnum <= src->num_params) {
        if (src->param[pnum-1] && src->param_len[pnum-1]) {
          if (dlen >= src->param_len[pnum-1]) {
            memcpy(d, src->param[pnum-1], src->param_len[pnum-1]);
            nc = src->param_len[pnum-1];
          }
          else
            nc = -1;
        }
      }
    }

    else if (*s == '@') {
      /* \\@ : insert a unique id "_nnnnnn" */
      s++;
      if (dlen > 7) {
        nc = sprintf(d, "_%06lu", src->id);
      }
      else
        nc = -1;
    }

    else if (*s == '.') {
      /* \\.label : create local label within macro */
      char *labname;
      s++;
      labname = s;
      while (ISIDCHAR(*s))
        s++;

      if (s > labname && src->macro) {
        strbuf *name = make_local_label(0, src->macro->name,
                                       strlen(src->macro->name),
                                       labname, s - labname);
        if (name && dlen >= strlen(name->str)) {
          strcpy(d, name->str);
          nc = strlen(name->str);
        }
        else
          nc = -1;
      }
    }

    if (nc >= dlen)
      nc = -1;
    else if (nc >= 0)
      *line = s;  /* update line pointer when expansion took place */
  }

  return nc;  /* number of chars written to line buffer, -1: out of space */
}


char *const_prefix(char *s,int *base)
{
  if (isdigit((unsigned char)*s)) {
    *base = 10;
    return s;
  }
  if (*s == '$') {
    *base = 16;
    return s+1;
  }
  if (*s=='@' && isdigit((unsigned char)*(s+1))) {
    *base = 8;
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
  return end;
}


strbuf *get_local_label(int n,char **start)
/* Motorola local labels start with a '.' or end with '$': "1234$", ".1" */
{
  char *s,*p;
  strbuf *name;

  name = NULL;
  s = *start;
  p = skip_local(s);

  if (p!=NULL && *p=='\\' && ISIDSTART(*s) && *s!=local_char && *(p-1)!='$') {
    /* skip local part of global\local label */
    s = p + 1;
    if (p = skip_local(s)) {
      name = make_local_label(n,*start,(s-1)-*start,s,p-s);
      *start = skip(p);
    }
    else
      return NULL;
  }
  else if (p!=NULL && p>(s+1)) {  /* identifier with at least 2 characters */
    if (*s == local_char) {
      /* .label */
      name = make_local_label(n,NULL,0,s,p-s);
      *start = skip(p);
    }
    else if (*(p-1) == '$') {
      /* label$ */
      name = make_local_label(n,NULL,0,s,p-s);
      *start = skip(p);
    }
  }

  return name;
}


int init_syntax(void)
{
  size_t i;
  hashdata data;

  /* Note: nocase is set via syntax_args() or defaults to 0 (case-sensitive)
     Use -nocase flag to enable case-insensitive mode per EDTASM+ spec */

  dirhash = new_hashtable(0x1800);
  for (i=0; i<dir_cnt; i++) {
    data.idx = i;
    add_hashentry(dirhash,directives[i].name,data,1);  /* directives always case-insensitive */
  }
  if (debug && dirhash->collisions)
    fprintf(stderr,"*** %d directive collisions!!\n",dirhash->collisions);

  cond_init();
  current_pc_char = '*';  /* Asterisk is current PC (also comment in column 1) */
  maxmacparams = 9;       /* EDTASM supports up to 9 macro parameters */
  dot_idchar = 1;         /* Allow dots in identifiers (HB.ADDR, etc.) */

  return 1;
}


int syntax_defsect(void)
{
  defsectname = code_name;
  defsecttype = code_type;
  return 1;
}


int syntax_args(char *p)
{
  if (!strcmp(p,"-nocase")) {
    /* Enable case-insensitive mode per EDTASM+ spec
       Default is case-sensitive for compatibility with existing code */
    nocase = 1;
    nocase_macros = 1;
    return 1;
  }
  return 0;
}
