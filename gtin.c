/*
 * PostgreSQL type definitions for GTINs.
 *
 */

#include <string.h>
#include "postgres.h"
#include "fmgr.h"
#include "utils/builtins.h"

#define MAXGTINLEN 19
#define PG_ARGS fcinfo // Might need to change if fmgr changes its name
#define GET_TEXT_STR(textp) DatumGetCString( \
    DirectFunctionCall1( textout, PointerGetDatum( textp ) ) \
)

/*
 * Forward declarations.
 *
 */

Datum isa_gtin_int64(PG_FUNCTION_ARGS);
Datum isa_gtin_text(PG_FUNCTION_ARGS);
int   isa_gtin(char *str);

Datum gtin_in (PG_FUNCTION_ARGS);
Datum gtin_out (PG_FUNCTION_ARGS);

Datum str_to_gtin (char *str);
Datum gtin_to_text (PG_FUNCTION_ARGS);
Datum text_to_gtin (PG_FUNCTION_ARGS);

Datum gtin_eq (PG_FUNCTION_ARGS);
Datum gtin_ne (PG_FUNCTION_ARGS);
Datum gtin_lt (PG_FUNCTION_ARGS);
Datum gtin_le (PG_FUNCTION_ARGS);
Datum gtin_gt (PG_FUNCTION_ARGS);
Datum gtin_ge (PG_FUNCTION_ARGS);

Datum gtin_cmp (PG_FUNCTION_ARGS);
int gtin_str_cmp (PG_FUNCTION_ARGS);

/*
 * isa_gtin()
 *
 * This function does the work of validating a GTIN. It is called by gtin_in()
 * and isa_gtin_text().
 *
 */

int isa_gtin(char *str) {
    int len = strlen(str);
    int index;
    int sum = 0;
    
    // Return false if there are no digits.
    if (len <= 0)
      return 0;

    for (index = len-2; index >= 0; index -=2 ) {
        // Return false if it's not a digit.
        if (str[index] < '0' || str[index] > '9')
            return 0;
        sum += (str[index] - '0');
    }

    sum *= 3;

    for (index = len -1; index >= 0; index -=2 ) {
        // Return false if it's not a digit.
        if (str[index] < '0' || str[index] > '9')
            return 0;
        sum += (str[index] - '0');
    }

    return sum % 10 == 0;
}

/*
 * isa_gtin_text()
 *
 * Validates GTINs passed as PostgreSQL text.
 *
 */

PG_FUNCTION_INFO_V1(isa_gtin_text);

Datum isa_gtin_text(PG_FUNCTION_ARGS) {
  PG_RETURN_BOOL( isa_gtin( GET_TEXT_STR( PG_GETARG_TEXT_P(0) ) ) );
}

/*
 * Name: str_to_gtin()
 *
 * Converts a character string to a GTIN. Called by text_to_gtin() and
 * gtin_in().
 *
 */

Datum str_to_gtin (char *str) {
    char *result = (char *) palloc( MAXGTINLEN );

    // Strip out leading 0s.
    while (*str == '0') ++str; // XXX Memory leak??

    if (!isa_gtin(str)) {
        ereport( ERROR, (
            errcode(ERRCODE_INVALID_PARAMETER_VALUE),
            errmsg("\"%s\" is not a valid GTIN", str)
        ));
    }

    memset( result, 0, MAXGTINLEN );
    result = (char *) str;
    PG_RETURN_POINTER( result );
}

/*
 * Name: gtin_in()
 *
 * Converts a gtin from an external for to an internal form.
 *
 */

PG_FUNCTION_INFO_V1(gtin_in);
Datum gtin_in (PG_FUNCTION_ARGS) {
  return( str_to_gtin( PG_GETARG_CSTRING(0) ) );
}

/*
 * Name: gtin_out()
 *
 * Converts the internal form of a GTIN to an external cstring.
 *
 */

PG_FUNCTION_INFO_V1(gtin_out);

Datum gtin_out (PG_FUNCTION_ARGS) {
    char * src    = (char *) PG_GETARG_POINTER(0);
    char * result = (char *) palloc( strlen(src) + 1 );
    strcpy( result, src );
    PG_RETURN_CSTRING( result );
}

/*
 *		==============
 *		CAST FUNCTIONS
 *		==============
 */

/*
 * Name: gtin_to_text()
 *
 * Converts a gtin to text.
 *
 */

PG_FUNCTION_INFO_V1(gtin_to_text);

Datum gtin_to_text(PG_FUNCTION_ARGS) {
    char * src    = (char *) PG_GETARG_POINTER(0);
	int    len    = strlen( src );
	text * result = (text *) palloc(VARHDRSZ + len);

	VARATT_SIZEP( result ) = VARHDRSZ + len;
    memmove( VARDATA( result ), src, len );
    PG_RETURN_TEXT_P( result );
}

/*
 * Name: text_to_gtin()
 *
 * Converts text to a gtin.
 *
 */

PG_FUNCTION_INFO_V1(text_to_gtin);

Datum text_to_gtin(PG_FUNCTION_ARGS) {
    return str_to_gtin( GET_TEXT_STR( PG_GETARG_TEXT_P(0) ) );
}

/*
 *		==================
 *		OPERATOR FUNCTIONS
 *		==================
 */

PG_FUNCTION_INFO_V1(gtin_eq);

Datum gtin_eq (PG_FUNCTION_ARGS) {
    PG_RETURN_BOOL( gtin_str_cmp( PG_ARGS ) == 0 );
}

PG_FUNCTION_INFO_V1(gtin_ne);

Datum gtin_ne (PG_FUNCTION_ARGS) {
    PG_RETURN_BOOL( gtin_str_cmp( PG_ARGS ) != 0 );
}

PG_FUNCTION_INFO_V1(gtin_lt);

Datum gtin_lt (PG_FUNCTION_ARGS) {
    PG_RETURN_BOOL( gtin_str_cmp( PG_ARGS ) < 0 );
}

PG_FUNCTION_INFO_V1(gtin_le);

Datum gtin_le (PG_FUNCTION_ARGS) {
    PG_RETURN_BOOL( gtin_str_cmp( PG_ARGS ) <= 0 );
}

PG_FUNCTION_INFO_V1(gtin_gt);

Datum gtin_gt (PG_FUNCTION_ARGS) {
    PG_RETURN_BOOL( gtin_str_cmp( PG_ARGS ) > 0 );
}

PG_FUNCTION_INFO_V1(gtin_ge);

Datum gtin_ge (PG_FUNCTION_ARGS) {
    PG_RETURN_BOOL( gtin_str_cmp( PG_ARGS ) >= 0 );
}

PG_FUNCTION_INFO_V1(gtin_cmp);

Datum gtin_cmp (PG_FUNCTION_ARGS) {
    PG_RETURN_INT32( gtin_str_cmp( PG_ARGS ) );
}

int gtin_str_cmp (PG_FUNCTION_ARGS) {
    char * left    = (char *) PG_GETARG_POINTER(0);
    char * right   = (char *) PG_GETARG_POINTER(1);
    return strcmp( left, right );
}
