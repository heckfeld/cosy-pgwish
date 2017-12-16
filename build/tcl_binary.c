/* package which supports a safe, yet fast encoding for binary data
   in tcl.

   the basic idea is that the only really dangerous character in
   binary data streams (the null) is remapped to a rarely used
   character code.  that character code is remapped using a quoting
   convention.  the result is a string which should only be slightly
   larger in size over the original binary data and which can be
   encoded very quickly.  furthermore, strings encoded in ASCII,
   ISO-Latin and all versions of EUC will be unaffected by the
   encoding. 

   two approaches for encoding data are presented.  first, procedures
   analogous to the ones used to set the result string in interpreters
   or to add to DStrings are provided.  secondly, there is a lower
   level interface with supports encoding/decoding in place.  my
   assumption is that the Tcl oriented interface will be more useful.

   the lower level idiom is designed to be used in code like this:

   char *data;  -- our binary data
   int len;     -- its original length
   int new;     -- the new length

   new = Bin_EncodedLength(data, len);
   data = realloc(data, new);
   Bin_EncodeData(data, data, len);

   where the realloc could as well have been a malloc of a separate
   buffer. 

   this package does use some knowledge about the internal structure
   of DStrings and interpreters.  hopefully this information is
   relatively stable.  the current implementation is compatible with
   tcl 7.3 and tcl 7.5.

*/

#include <malloc.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <tcl.h>

#include "tcl_binary.h"

/* Tcl_DStringTrunc is defined as a macro in 7.5 and later */
#ifndef Tcl_DStringTrunc
/*
 *----------------------------------------------------------------------
 *
 * Tcl_DStringSetLength --
 *
 *	Change the length of a dynamic string.  This can cause the
 *	string to either grow or shrink, depending on the value of
 *	length.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The length of dsPtr is changed to length and a null byte is
 *	stored at that position in the string.  If length is larger
 *	than the space allocated for dsPtr, then a panic occurs.
 *
 *----------------------------------------------------------------------
 */

void
Tcl_DStringSetLength(dsPtr, length)
    register Tcl_DString *dsPtr;	/* Structure describing dynamic
					 * string. */
    int length;				/* New length for dynamic string. */
{
    if (length < 0) {
	length = 0;
    }
    if (length >= dsPtr->spaceAvl) {
	char *newString;

	dsPtr->spaceAvl = length+1;
	newString = (char *) ckalloc((unsigned) dsPtr->spaceAvl);

	/*
	 * SPECIAL NOTE: must use memcpy, not strcpy, to copy the string
	 * to a larger buffer, since there may be embedded NULLs in the
	 * string in some cases.
	 */

	memcpy((VOID *) newString, (VOID *) dsPtr->string,
		(size_t) dsPtr->length);
	if (dsPtr->string != dsPtr->staticSpace) {
	    ckfree(dsPtr->string);
	}
	dsPtr->string = newString;
    }
    dsPtr->length = length;
    dsPtr->string[length] = 0;
}
#endif

#define ENCODED_NULL ((char) 128)
#define QUOTE_CHAR   ((char) 127)

void Bin_DStringAppend(Tcl_DString *s, void *data, int len)
{
    int base;

    base = s->length;
    Tcl_DStringSetLength(s, base+Bin_EncodedLength(data, len));
    Bin_EncodeData(s->string+base, data, len);
}

void Bin_DStringAppendElement(Tcl_DString *s, void *data, int len)
{
    Tcl_DStringStartSublist(s);
    Bin_DStringAppend(s, data, len);
    Tcl_DStringEndSublist(s);
}

void Bin_SetResult(Tcl_Interp *interp, char *data, int len)
{
    Tcl_DString s;

    Tcl_DStringInit(&s);
    Bin_DStringAppend(&s, data, len);
    Tcl_DStringResult(interp, &s);
}

int Bin_EncodedLength(char *src, int len)
{
    int r;
    int i;

    r = len;
    for (i=0;i<len;i++) {
	if (src[i] == QUOTE_CHAR || src[i] == ENCODED_NULL) r++;
    }
    return r;
}

void Bin_EncodeData(char *dst, char *src, int len)
{
    int i, j;
    if (dst == src) {
	/* avoid any shifting as long as possible */
	i = 0;
	while (i<len && src[i] != QUOTE_CHAR && src[i] != ENCODED_NULL) {
	    if (src[i] == '\0') {
		src[i] = ENCODED_NULL;
	    }
	    i++;
	}
	/* shift the origin */
	src += i;
	dst += i;
	len -= i;

	/* then count how much growth we get */
	j = 0;
	for (i=0;i<len;i++) {
	    if (src[i] == QUOTE_CHAR || src[i] == ENCODED_NULL) {
		j++;
	    }
	    j++;
	}
	dst[j] = 0;

	/* and finally scan backwards copying as we go */
	i--;
	j--;
	while (j >= 0) {
	    switch (src[i]) {
	    case 0:
		dst[j] = ENCODED_NULL;	  /* nulls get encoded */
		break;
	    case QUOTE_CHAR:
	    case ENCODED_NULL:
		dst[j--] = src[i];	  /* magic characters get quoted */
		dst[j] = QUOTE_CHAR;	  /* *backwards!!* */
		break;
	    default:
		dst[j] = src[i];	  /* other stuff just gets copied */
		break;
	    }
	    i--;
	    j--;
	}
    }
    else {
	j = 0;
	for (i=0;i<len;i++) {		  /* if src and dst are different */
	    switch (src[i]) {		  /* we can scan forward */
	    case 0:			  /* this would break if src */
		dst[j] = ENCODED_NULL;	  /* and dst overlap. */
		break;
	    case QUOTE_CHAR:
	    case ENCODED_NULL:
		dst[j++] = QUOTE_CHAR;
		dst[j] = src[i];
		break;
	    default:
		dst[j] = src[i];
		break;
	    }
	    j++;
	}
	dst[j] = 0;
    }
}

int Bin_DecodeData(char *dst, char *src, int len)
{
    int i, j;

    j = 0;
    for (i=0;i<len;i++) {
	switch (src[i]) {
	case QUOTE_CHAR:
	    i++;
	    dst[j] =  src[i];
	    break;
	case ENCODED_NULL:
	    dst[j] = 0;
	    break;
	default:
	    dst[j] = src[i];
	    break;
	}
	j++;
    }
    return j;
}



/* cheap hack of Tcl_ReadCmd to read coded binary data instead.  only
   one line changed.  */
#define READ_BUF_SIZE 4096
int Bin_Read(ClientData cl, Tcl_Interp *interp, int argc, char *argv[])
{
    int bytesLeft, bytesRead, askedFor, got;
    char buffer[2*READ_BUF_SIZE+1];
    int newline, i;
    FILE *f;

    if ((argc != 2) && (argc != 3)) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" fileId ?numBytes?\" or \"", argv[0],
		" ?-nonewline? fileId\"", (char *) NULL);
	return TCL_ERROR;
    }
    i = 1;
    newline = 1;
    if ((argc == 3) && (strcmp(argv[1], "-nonewline") == 0)) {
	newline = 0;
	i++;
    }
    if (Tcl_GetOpenFile(interp, argv[i], 0, 1, &f) != TCL_OK) {
	return TCL_ERROR;
    }

    /*
     * Compute how many bytes to read, and see whether the final
     * newline should be dropped.
     */

    if ((argc >= (i + 2)) && isdigit(argv[i+1][0])) {
	if (Tcl_GetInt(interp, argv[i+1], &bytesLeft) != TCL_OK) {
	    return TCL_ERROR;
	}
    } else {
	bytesLeft = INT_MAX;
    }

    /*
     * Read the file in one or more chunks.
     */

    bytesRead = 0;
    clearerr(f);
    while (bytesLeft > 0) {
	askedFor = READ_BUF_SIZE;
	if (bytesLeft < READ_BUF_SIZE) {
	    askedFor = bytesLeft;
	}
	got = fread(buffer, 1, (size_t) askedFor, f);
	if (ferror(f)) {
	    /*
	     * If the file is in non-blocking mode, break out of the
	     * loop and return any bytes that were read.
	     */

	    if (((errno == EWOULDBLOCK) || (errno == EAGAIN))
		    && ((got > 0) || (bytesRead > 0))) {
		clearerr(f);
		bytesLeft = got;
	    } else {
		Tcl_ResetResult(interp);
		Tcl_AppendResult(interp, "error reading \"", argv[i],
			"\": ", Tcl_PosixError(interp), (char *) NULL);
		return TCL_ERROR;
	    }
	}
	if (got != 0) {
	    buffer[got] = 0;
	    Bin_EncodeData(buffer, buffer, got);
	    Tcl_AppendResult(interp, buffer, (char *) NULL);
	    bytesLeft -= got;
	    bytesRead += got;
	}
	if (got < askedFor) {
	    break;
	}
    }
    if ((newline == 0) && (bytesRead > 0)
	    && (interp->result[bytesRead-1] == '\n')) {
	interp->result[bytesRead-1] = 0;
    }
    return TCL_OK;
}

int Bin_Write(ClientData cl, Tcl_Interp *interp, int argc, char *argv[])
{
    int i, n;
    FILE *f;

    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" fileId data\"", (char *) NULL);
	return TCL_ERROR;
    }
    i = 1;
    if (Tcl_GetOpenFile(interp, argv[1], 0, 1, &f) != TCL_OK) {
	return TCL_ERROR;
    }

    n = Bin_DecodeData(argv[2], argv[2], strlen(argv[2]));
    fwrite(argv[2], n, 1, f);
    if (ferror(f)) {
	Tcl_ResetResult(interp);
	Tcl_AppendResult(interp, "error writing \"", argv[i],
			 "\": ", Tcl_PosixError(interp), (char *) NULL);
	
	return TCL_ERROR;
    }
    return TCL_OK;
}

static inline
unsigned char get_byte(char *source, int *offset)
{
    unsigned char r;
    unsigned char *p = (unsigned char *)source;

    r = p[*offset];
    if (r == 0)
	return r;
    (*offset)++;
    if (r == (unsigned char)QUOTE_CHAR) {
	r = p[*offset];
	(*offset)++;
    } else if (r == (unsigned char)ENCODED_NULL) {
	r = 0;
    }
    return r;
}

static
short get_short(char *source, int *offset)
{
    short temp;
    unsigned char a, b;

    a = get_byte(source, offset);
    b = get_byte(source, offset);
    temp = a;
    temp <<= 8;
    temp |= b;
    return temp;
}

static
long get_long(char *source, int *offset)
{
    long temp;

    temp = get_byte(source, offset);
    temp <<= 8;
    temp |= get_byte(source, offset);
    temp <<= 8;
    temp |= get_byte(source, offset);
    temp <<= 8;
    temp |= get_byte(source, offset);
    return temp;
}

static int IntValue(Tcl_Interp *interp, char *v1, char *v2, int value, int flags)
{
    char buf[20];
    sprintf(buf, "%d", value);
    if (Tcl_SetVar2(interp, v1, v2, buf, TCL_LEAVE_ERR_MSG|flags) == NULL){
	return TCL_ERROR;
    }
    return TCL_OK;
}

static int DoubleValue(Tcl_Interp *interp, char *v1, char *v2, double value, int flags)
{
    char buf[100];
    Tcl_PrintDouble(interp, value, buf);

    if (Tcl_SetVar2(interp, v1, v2, buf, TCL_LEAVE_ERR_MSG|flags) == NULL) {
	return TCL_ERROR;
    }
    return TCL_OK;
}

static void copy_bytes(void *dst, char *src, int len)
{
    int i;
    i = 0;
    while (i < len) {
	i += Bin_DecodeData(dst, src, len);
    }
}

static int GetValue(Tcl_Interp *interp,
		    char **format, char *source, int *offset,
		    char *v1, char *v2, int flags)
{
    int ch, i, t;
    float f;
    double d;
    char *s, *p;
    char buf[20];

    ch = *(*format)++;
    while (ch && ch == ' ') ch = *(*format)++;

    switch (ch) {
    case 0:
    case '>':
	return TCL_ERROR;
    case 'i':
	if (IntValue(interp, v1, v2, get_long(source, offset),flags) != TCL_OK) {
	    return TCL_ERROR;
	}
	break;
    case 'h':
	if (IntValue(interp, v1, v2, get_short(source, offset),flags) != TCL_OK) {
	    return TCL_ERROR;
	}
	break;
    case 'b':
	if (IntValue(interp, v1, v2, get_byte(source, offset),flags) != TCL_OK) {
	    return TCL_ERROR;
	}
	break;
    case 'f':
	copy_bytes((char *)&f, source + *offset, sizeof(f));
	*offset += sizeof(f);
	if (DoubleValue(interp, v1, v2, f,flags) != TCL_OK) {
	    return TCL_ERROR;
	}
	break;
    case 'd':
	copy_bytes((char *) &d, source + *offset, sizeof(d));
	*offset += sizeof(d);
	if (DoubleValue(interp, v1, v2, d,flags) != TCL_OK) {
	    return TCL_ERROR;
	}
	break;
    case 's':
	s = strchr(source + *offset, ENCODED_NULL);
	if (!s) {
	    t = strlen(source + *offset)+1;
	}
	else {
	    t = (s - (source + *offset))+1;
	}
	s = ckalloc(t);
	Bin_DecodeData(s, source + *offset, t);
	if (Tcl_SetVar2(interp, v1, v2, s, TCL_LEAVE_ERR_MSG) == NULL) {
	    ckfree(s);
	    return TCL_ERROR;
	}
	*offset += t;
	ckfree(s);
	break;
    case 'v':
	if (isdigit(**format)) {
	    sscanf(*format, "%d", &t);
	    while (**format && isdigit(**format)) {
		(*format)++;
	    }
	}
	else {
	    t = get_long(source, offset);
	}
	if (**format != '(') {
#if 0
	    Tcl_SetResult(interp, "bad vector format in \"bin_scan\"",
			  TCL_STATIC);
	    return TCL_ERROR;
#endif
	    flags = TCL_APPEND_VALUE|TCL_LIST_ELEMENT;
	    p = NULL;
	} else {
	    flags = 0;
	    p = buf;
	    (*format)++;
	}

	for (i=0;i<t;i++) {

	    if (v2) sprintf(buf, "%s,%d", v2, i);
	    else sprintf(buf, "%d", i);

	    s = *format;
	    if (s == NULL ||
		GetValue(interp, &s, source, offset, v1, p, flags) != TCL_OK) {
		return TCL_ERROR;
	    }

	    /* have to check each time in case we run out */
	    if (flags == 0 && *s != ')') {
		Tcl_SetResult(interp, "bad vector format in \"bin_scan\"",
			      TCL_STATIC);
		return TCL_ERROR;
	    }
	}
	if (flags == 0) {
	    if (v2) sprintf(buf, "%s,count", v2);
	    else sprintf(buf, "count");
	    if (IntValue(interp, v1, buf, t,flags) != TCL_OK) {
		return TCL_ERROR;
	    }
	    *format = s+1;
	} else
	    *format = s;
	break;

    case '<':
	while (**format && **format != '>') {
	    char *slot, *colon;

	    while (**format && **format == ' ') (*format)++;

	    slot = *format;
	    colon = strchr(slot, ':');
	    if (!colon) return TCL_ERROR;
	    *colon = 0;
	    *format = colon+1;

	    if (v2) sprintf(buf, "%s,%s", v2, slot);
	    else sprintf(buf, "%s", slot);

	    if (GetValue(interp, format, source, offset, v1, buf,0) != TCL_OK) {
		return TCL_ERROR;
	    }
	    *colon = ':';
	}
	if (**format) (*format)++;
	break;
    }
    return TCL_OK;
}

int Bin_Scan(ClientData cl, Tcl_Interp *interp, int argc, char *argv[])
{
    int offset;
    char *format;
    char *source;
    char **vars;

    if (argc <= 4) {
	Tcl_SetResult(interp, "bad args for \"binary_scan\", wanted offset format data var1 ... varN", TCL_STATIC);
	return TCL_ERROR;
    }
    if (Tcl_GetInt(interp, argv[1], &offset) != TCL_OK) {
	Tcl_SetResult(interp, "bad offset for \"binary_scan\"", TCL_STATIC);
	return TCL_ERROR;
    }
    format = argv[2];
    source = argv[3];
    vars = argv+4;

    while (*format && source[offset] && *vars) {
	if (GetValue(interp, &format, source, &offset, vars[0], NULL,0)) {
	    return TCL_ERROR;
	}
	vars++;
    }
    if (*format) {
	Tcl_SetResult(interp, "ran out of data in \"binary_scan\"",
		      TCL_STATIC);
	return TCL_ERROR;
    }

    sprintf(interp->result, "%d", offset);
    return TCL_OK;
}

int PutValue(Tcl_Interp *interp,
	     Tcl_DString *r, char **format, char *v1, char *v2)
{
    char bval;
    short int sval;
    int ival;
    double dval;
    float fval;

    while (**format && isspace(**format)) (*format)++;
    switch (**format) {
    case 0:
	return TCL_OK;
    default:
	return TCL_ERROR;
    case 'i':
	if (v2 != NULL) {
	    v1 = Tcl_GetVar2(interp, v1, v2, 0);
	}
	if (Tcl_GetInt(interp, v1, &ival) != TCL_OK) {
	    return TCL_ERROR;
	}
	ival = htonl(ival);
	Bin_DStringAppend(r, (char *) &ival, 4);
	break;
    case 'h':
	if (v2 != NULL) {
	    v1 = Tcl_GetVar2(interp, v1, v2, 0);
	}
	if (Tcl_GetInt(interp, v1, &ival) != TCL_OK) {
	    return TCL_ERROR;
	}
	sval = (short) ival;
	sval = htons(sval);
	Bin_DStringAppend(r, &sval, 2);
	break;
    case 'b':
	if (v2 != NULL) {
	    v1 = Tcl_GetVar2(interp, v1, v2, 0);
	}
	if (Tcl_GetInt(interp, v1, &ival) != TCL_OK) {
	    return TCL_ERROR;
	}
	bval = (char) ival;
	Bin_DStringAppend(r, &bval, 1);
	break;
    case 'f':
	if (v2 != NULL) {
	    v1 = Tcl_GetVar2(interp, v1, v2, 0);
	}
	if (Tcl_GetDouble(interp, v1, &dval) != TCL_OK) {
	    return TCL_ERROR;
	}
	fval = dval;
	Bin_DStringAppend(r, &fval, 4);
	break;
    case 'd':
	if (v2 != NULL) {
	    v1 = Tcl_GetVar2(interp, v1, v2, 0);
	}
	if (Tcl_GetDouble(interp, v1, &dval) != TCL_OK) {
	    return TCL_ERROR;
	}
	Bin_DStringAppend(r, &dval, 8);
	break;
    case 's':
	if (v2 != NULL) {
	    v1 = Tcl_GetVar2(interp, v1, v2, 0);
	}
	Bin_DStringAppend(r, v1, strlen(v1)+1);
	break;
    case 'v':
#if 0
	if (isdigit(**format)) {
	    sscanf(*format, "%d", &t);
	    while (**format && isdigit(**format)) {
		(*format)++;
	    }
	}
	else {
	    t = get_long(source, offset);
	}
	if (**format != '(') {
	    Tcl_SetResult(interp, "bad vector format in \"bin_scan\"",
			  TCL_STATIC);
	    return TCL_ERROR;
	}
	(*format)++;

	for (i=0;i<t;i++) {
	    if (v2) sprintf(buf, "%s,%d", v2, i);
	    else sprintf(buf, "%d", i);

	    s = *format;
	    if (s == NULL ||
		GetValue(interp, &s, source, offset, v1, buf,0) != TCL_OK) {
		return TCL_ERROR;
	    }

	    /* have to check each time in case we run out */
	    if (*s != ')') {
		Tcl_SetResult(interp, "bad vector format in \"bin_scan\"",
			      TCL_STATIC);
		return TCL_ERROR;
	    }
	}
	if (v2) sprintf(buf, "%s,count", v2);
	else sprintf(buf, "count");
	if (IntValue(interp, v1, buf, t,flags) != TCL_OK) {
	    return TCL_ERROR;
	}
	*format = s+1;
#endif
	break;
    case '<':
	break;
    }
    if (**format) (*format)++;
    return TCL_OK;
}


/* this command can be called in the following way:

   binary_append var_name format var1 ... varn

   the result will be that var_name will have binary data consisting
   of the values of var1 ... varn appended to it.  this data will be
   encoded compatibly with the other binary functions.

   format specifies the way that var1 ... varn are to be interpreted.
   the codes used are the same as are used by binary_scan.  vector
   arguments are assumed to be names of tcl arrays which must follow
   the conventions used by binary_scan (i.e. they must have a count
   element as well as each element of the vector).  structures are
   also handled as in binary_scan.

   binary_scan and binary_append can be considered to be approximate
   inverses.  this means that:

   set end [binary_scan $start $x $format var1 ... varn]
   set y {}
   binary_append y $format $var1 ... $varn

   will cause the bytes in y to contain the same data as in the bytes
   in x from position $start and up to, but not including $end,
   subject to the accuracy limits of the conversion to a decimal
   representation inherent in tcl's representation of floating point
   numbers.  similarly, in the following code,

   binary_append y $format $var1 ... $varn
   set end [binary_scan 0 $y $format xvar1 ... xvarn]

   end will be the total length of y and xvar1 ... xvarn will contain
   the same values as var1 ... varn.  this equality will be subject to
   some roundoff of floating point values, but not as severely as in
   the first case.
   */

int Bin_Append(ClientData cl, Tcl_Interp *interp, int argc, char
	       *argv[])
{
    Tcl_DString r;
    char *format;
    char **vars;
	char *result;
    if (argc < 3) {
	Tcl_ResetResult(interp);
	Tcl_AppendResult(interp, "bad # of args, expected ", argv[0],
			 " data format ?var1 ... varn?", NULL);
	return TCL_ERROR;
    }
    Tcl_DStringInit(&r);
    format = argv[2];
    vars = argv+3;
    while (*format && *vars) {
	if (PutValue(interp, &r, &format, vars[0], NULL)) {
	    return TCL_ERROR;
	}
	vars++;
    }
    if (*format) {
	Tcl_SetResult(interp, "ran out of data in \"binary_append\"",
		      TCL_STATIC);
	return TCL_ERROR;
    }
	
    if ( Tcl_SetVar(interp, argv[1], Tcl_DStringValue(&r), TCL_APPEND_VALUE | TCL_LEAVE_ERR_MSG) == NULL) {
	return TCL_ERROR;
    }
    Tcl_DStringFree(&r);
    return TCL_OK;
    
}

#ifdef TEST

#include <time.h>

int binary_test(ClientData cl, Tcl_Interp *interp, int argc, char *argv[])
{
    int seed;
    double drand48();
    void srand48();
    int trial, i, len, encoded_len;
    char a[1000], *b, *c;

/*    malloc_debug(2);*/
    b = malloc(sizeof(a));
    c = malloc(sizeof(a));

    if (argc == 1) {
	seed = time(0);
	srand48(seed);
    }
    else {
	seed = atol(argv[1]);
	srand48(seed);
    }
    for (trial=0;trial<2000;trial++) {
	len = 10 + (sizeof(a) - 10) * drand48();
	for (i=0;i<len;i++) {
	    a[i] = 256*drand48();
	}

	encoded_len = Bin_EncodedLength(a, len);

	b = realloc(b, encoded_len);

	/* encode to a different place */
	Bin_EncodeData(b, a, len);

	for (i=0;i<encoded_len;i++) {
	    if (b[i] == 0) {
		sprintf(interp->result,
			"Found a NULL in pass %d.1 at %d\n", trial, i);
		return TCL_ERROR;
	    }
	}

	/* in place decode */
	Bin_DecodeData(b, b, encoded_len);

	for (i=0;i<len;i++) {
	    if (a[i] != b[i]) {
		sprintf(interp->result, "Round trip %d.1 failed at %d\n",
			trial, i);
		return TCL_ERROR;
	    }
	}

	/* in place encode */
	Bin_EncodeData(b, b, len);
	for (i=0;i<encoded_len;i++) {
	    if (b[i] == 0) {
		sprintf(interp->result, "Found a NULL in pass %d.2 at %d\n",
			trial, i);
		return TCL_ERROR;
	    }
	}

	/* and a foreign decode */
	c = realloc(c, len);
	Bin_DecodeData(c, b, encoded_len);

	for (i=0;i<len;i++) {
	    if (a[i] != c[i]) {
		sprintf(interp->result, "Round trip %d.2 failed at %d\n",
			trial, i);
		return TCL_ERROR;
	    }
	}
    }
    free(b);
    free(c);
    sprintf(a, "%d", seed);
    Tcl_SetResult(interp, a, TCL_STATIC);
    return TCL_OK;
}
#endif

int Binary_Init(Tcl_Interp *interp)
{
    Tcl_CreateCommand(interp, "binary_read", Bin_Read, NULL, NULL);
    Tcl_CreateCommand(interp, "binary_write", Bin_Write, NULL, NULL);
    Tcl_CreateCommand(interp, "binary_scan", Bin_Scan, NULL, NULL);
    Tcl_CreateCommand(interp, "binary_append", Bin_Append, NULL, NULL);
#ifdef TEST
    Tcl_CreateCommand(interp, "binary_test", binary_test, NULL, NULL);
#endif
    return TCL_OK;
}
