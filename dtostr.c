
// Precompile code for  dtostr

#define WANT_EXP_FORM   1
#define EXP_UPPERCASE   1
#define EXP_TABLE_SIZE       9
#define MAX_DIGITS          17
#if 1
#define _zero_or_inf_check(x) ( x == (x/4) )
#else
extern int _zero_or_inf_check(double x);
#endif

#define isnan(x) (x != x)

#include <float.h>
#include <limits.h>

/*
 * Set things up for the scaling power table.
 */

#if EXP_TABLE_SIZE < 6
#error EXP_TABLE_SIZE should be at least 6 to comply with standards
#endif

#define EXP_TABLE_MAX      (1U<<(EXP_TABLE_SIZE-1))

/*
 * Only bother checking if this is too small.
 * Throw in some play for denormals ( roughly O(-324) vs O(-307) on i386 ).
 */

#if (3+DBL_DIG-DBL_MIN_10_EXP)/2 > EXP_TABLE_MAX
#error larger EXP_TABLE_SIZE needed
#endif

/*
 * With 32 bit ints, we can get 9 digits per block.
 */
#define DIGITS_PER_BLOCK     9

#if (INT_MAX >> 30)
#define DIGIT_BLOCK_TYPE     int
#elif (LONG_MAX >> 30)
#define DIGIT_BLOCK_TYPE     long
#else
#error need at least 32 bit longs
#endif

/*
 * This is kind of a place-holder for LONG_DOUBLE support to show what I
 * think needs to be changed.  I haven't tried it though.  Changing this
 * from 3 to 4 and converting double to long double should work on i386.
 * DON'T FORGET to increase EXP_TABLE_SIZE and MAX_DIGITS.
 * DON'T FORGET the "larger EXP_TABLE_SIZE needed" check above.
 */
#define MAX_EXP_DIGITS 3


#define NUM_DIGIT_BLOCKS   ((MAX_DIGITS+DIGITS_PER_BLOCK-1)/DIGITS_PER_BLOCK)

static char infstr[] = " inf";/* save space for a - sign */
static char nanstr[] = "nan";

/* extra space for '-', '.', 'e+###', and nul */
static char buf[ 5 + MAX_EXP_DIGITS + NUM_DIGIT_BLOCKS * DIGITS_PER_BLOCK];
extern int npolygons;
/*****************************************************************************/

const char *_dtostr(double x)
{
  double exp_table[EXP_TABLE_SIZE];
  double p10;
  DIGIT_BLOCK_TYPE digit_block; /* int of at least 32 bits */
  int i, j;
  int exp, exp_neg;
  int negative = 0;
  char *pos;
  
  if (isnan(x)) {/* nan check */
    return nanstr;
  }

  if (x == 0) {/* handle 0 now to avoid false positive */
    exp = 0;/* with inf test, and to avoid scaling  */
    goto GENERATE_DIGITS;/* note: time vs space tradeoff */
  }
  
  if (x < 0) {/* convert negatives to positives */
    negative = 1;
    x = -x;
  }
  
  if (_zero_or_inf_check(x)) { /* must be inf since zero handled above */
    pos = infstr + 1;
    goto DO_SIGN;
  }

  /* need to build the scaling table */
  for (i = 0, p10 = 10 ; i < EXP_TABLE_SIZE ; i++) {
    exp_table[i] = p10;
    p10 *= p10;
  }
  
  exp_neg = 0;
  if (x < 1e8) {/* do we need to scale up or down? */
    exp_neg = 1;
  }

#if WANT_EXP_FORM
  exp = DIGITS_PER_BLOCK - 1;
#else
  exp = DIGITS_PER_BLOCK;
#endif
  
  i = EXP_TABLE_SIZE;
  j = EXP_TABLE_MAX;
  while ( i-- ) {/* scale x such that 1e8 <= x < 1e9 */
    if (exp_neg) {
      if (x * exp_table[i] < 1e9) {
	x *= exp_table[i];
	exp -= j;
      }
    } else {
      if (x / exp_table[i] >= 1e8) {
	x /= exp_table[i];
	exp += j;
      }
    }
    j >>= 1;
  }
  
 GENERATE_DIGITS:
  pos = buf + 1 + DIGITS_PER_BLOCK + 1; /* leave space for '.' and - */
  
  for (i = 0 ; i < NUM_DIGIT_BLOCKS ; ++i ) {
    digit_block = (int) x;
    x = (x - digit_block) * 1e9;
    for (j = 0 ; j < DIGITS_PER_BLOCK ; j++) {
      *--pos = '0' + (digit_block % 10);
      digit_block /= 10;
    }
    pos += (2*DIGITS_PER_BLOCK);
  }
  pos -= (DIGITS_PER_BLOCK*(NUM_DIGIT_BLOCKS+1))-MAX_DIGITS;
  
  /* start generating the exponent */
#if EXP_UPPERCASE
  *pos = 'E';
#else
  *pos = 'e';
#endif
  *++pos = '+';
  if (exp < 0) {
    *pos = '-';
    exp = -exp;
  }
  pos += 3;/* WARNING: Assumes max exp < 1000!!! */
  if (exp >= 100) {
    ++pos;
#if MAX_EXP_DIGITS > 4
#error need to modify exponent string generation code
#elif MAX_EXP_DIGITS > 3
    if (exp >= 1000) {/* WARNING: hasn't been checked */
      ++pos;/*    but should work */
    }
#endif
  }
  *pos = '\0';
  
  for (j = 0 ; (j < 2) || exp ; j++) { /* standard says at least 2 digits */
    *--pos = '0' + (exp % 10);
    exp /= 10;
  }
  
  /* insert the decimal point */
  pos = buf + 1;
  
#if WANT_EXP_FORM
  *pos = *(pos+1);
  *(pos+1) = '.';
#else
  *pos = '.';
#endif
  
 DO_SIGN:
  if (negative) {
    *--pos = '-';
  }
  
  return pos;
}

//End of code for dtostr
