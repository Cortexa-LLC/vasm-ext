  "syntax error",ERROR,
  "invalid extension",ERROR,
  "no space before operands",WARNING,
  "too many closing parentheses",WARNING,
  "missing closing parentheses",WARNING,
  "missing operand",ERROR,                                           /* 5 */
  "garbage at end of line",WARNING,
  "%c expected",WARNING,
  "invalid data operand",ERROR,
  ", expected",WARNING,
  "identifier expected",ERROR,                                       /* 10 */
  "invalid local label number (must be 0-255)",ERROR,
  "local label without global label context",ERROR,
  "invalid private label format",ERROR,
  "hex string must contain even number of hex digits",ERROR,
  "invalid hex digit in .HS directive",ERROR,                        /* 15 */
  ".DO without .FIN",ERROR,
  ".ELSE without .DO",ERROR,
  ".FIN without .DO",ERROR,
  "too many nested .DO/.FIN (maximum 63 levels)",ERROR,
  ".EP without .PH",ERROR,                                           /* 20 */
  ".ED without .DUMMY",ERROR,
  "missing .EP",ERROR,
  "missing .ED",ERROR,
  "invalid CPU type for .OP directive",ERROR,
  "invalid directive",ERROR,                                         /* 25 */
  "label <%s> has already been defined",WARNING,
  "alignment too big",WARNING,
  "macro parameter out of range (]1-]9 only)",ERROR,
  "invalid macro parameter reference",ERROR,
  "missing closing delimiter for string",ERROR,                      /* 30 */
  ".EM without .MA",ERROR,
  "missing .EM",ERROR,
  "invalid .LIST option",WARNING,
  "invalid block size",ERROR,
  ".DUMMY section already active",ERROR,                             /* 35 */
  "phase already active",ERROR,
  "period '.' not allowed in identifier (reserved for directives/local labels)",ERROR,
  "underscore '_' alone is not a valid identifier",ERROR,
  "invalid expression in .DA directive",ERROR,
  "invalid binary digit (must be 0 or 1)",ERROR,                     /* 40 */
  "undefined macro",ERROR,
  ".TA directive ignored in cross-assembly (no effect)",WARNING,
