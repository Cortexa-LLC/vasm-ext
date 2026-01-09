/* syntax.h  syntax header file for vasm EDTASM syntax module */
/* (c) in 2002-2025 by Frank Wille */
/* EDTASM syntax adaptation (c) 2026 Bryan Woodruff, Cortexa LLC */

/* macros to recognize identifiers */
int isidchar(char);
int iscomment(char *);
#define ISIDSTART(x) ((x)=='_'||(x)=='@'||isalpha((unsigned char)(x)))
#define ISIDCHAR(x) isidchar(x)
#define ISBADID(p,l) ((l)==1&&(*(p)=='_'||*(p)=='@'))
#define ISEOL(p) (*(p)=='\0'||iscomment(p))

/* result of a boolean operation */
#define BOOLEAN(x) -(x)

/* we have a special skip() function for expressions, called exp_skip() */
char *exp_skip(char *);
#define EXPSKIP() s=exp_skip(s)

/* ignore operand field, when the instruction has no operands */
#define IGNORE_FIRST_EXTRA_OP 1

/* EDTASM supports up to 9 macro parameters */
#define MAXMACPARAMS 9
#define SKIP_MACRO_ARGNAME(p) (NULL)
