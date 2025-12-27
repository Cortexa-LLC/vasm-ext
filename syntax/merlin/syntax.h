/* syntax.h  syntax header file for vasm (Merlin syntax) */
/* (c) in 2002,2012,2014,2017,2020,2025 by Frank Wille */
/* Merlin syntax module adaptation (c) 2025 Bryan Woodruff, Cortexa LLC */

/* macros to recognize identifiers */
/* NOTE: Merlin directives have no prefix, so '.' not needed in ISIDSTART */
/* Tilde (~) allowed for wrapper macros like ~CtlStartUp */
/* Bracket (]) allowed for variable labels like ]COUNT */
int isidchar(char);
#define ISIDSTART(x) ((x)=='_'||(x)=='~'||(x)==']'||isalpha((unsigned char)(x)))
#define ISIDCHAR(x) isidchar(x)
#define ISBADID(p,l) ((l)==1&&(*(p)=='_'))
#define ISEOL(p) (*(p)=='\0' || *(p)==commentchar)

/* result of a boolean operation */
#define BOOLEAN(x) -(x)

/* we have a special skip() function for expressions, called exp_skip() */
char *exp_skip(char *);
#define EXPSKIP() s=exp_skip(s)

/* support for broken negative hex-constants: $-hex */
#define BROKEN_HEXCONST

/* Merlin uses period (.) as bit-OR operator (e.g., RdGrp.Inc = RdGrp | Inc) */
#define DOT_AS_BITOR

/* Merlin uses non-alphanumeric delimiters: <<< (end macro), --^ (end loop) */
#define SYNTAX_MERLIN_DELIMITERS

/* operator separation characters */
#ifndef OPERSEP_COMMA
#define OPERSEP_COMMA 1
#endif
#ifndef OPERSEP_BLANK
#define OPERSEP_BLANK 0
#endif

/* ignore operand field, when the instruction has no operands */
extern int igntrail;
#define IGNORE_FIRST_EXTRA_OP (igntrail)

/* Enable optional # prefix in data directives (dfb #EXPR) for Merlin compatibility */
#define SYNTAX_SUPPORTS_SCASM_OPS

/* symbol which contains the current rept-endr iteration count */
#define REPTNSYM "__RPTCNT"

/* overwrite macro defaults */
/* Merlin supports ]0 (count) through ]8 parameters */
#define MAXMACPARAMS 8
char *my_skip_macro_arg(char *);
#define SKIP_MACRO_ARGNAME(p) my_skip_macro_arg(p)

/* Merlin uses semicolon (;) as parameter separator, not comma */
/* Also allow comma and other separators for compatibility */
char *merlin_macro_param_sep(char *);
#define MACRO_PARAM_SEP(p) merlin_macro_param_sep(p)

/* cpu-specific data sizes */
#if defined(VASM_CPU_650X) || defined(VASM_CPU_SPC700)
#define DATWORD handle_d16
#define SPCWORD handle_spc16
#define DATLONG handle_d24
#define SPCLONG handle_spc24
#define DATDWRD handle_d32
#define SPCDWRD handle_spc32
#else
#define DATWORD handle_d16
#define SPCWORD handle_spc16
#define DATLONG handle_d32
#define SPCLONG handle_spc32
#define DATDWRD handle_d32
#define SPCDWRD handle_spc32
#endif

/* Merlin directive metadata accessor functions */
int get_merlin_file_type(void);
int get_merlin_aux_type(void);
const char *get_merlin_output_filename(void);
int get_merlin_cycle_counting(void);

/* USR directive accessor functions for RW18 disk format (crackle compatibility) */
int get_merlin_usr_valid(void);
int get_merlin_usr_side(void);
int get_merlin_usr_track(void);
int get_merlin_usr_offset(void);
