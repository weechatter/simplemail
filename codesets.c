/***************************************************************************
 SimpleMail - Copyright (C) 2000 Hynek Schlawack and Sebastian Bauer

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
***************************************************************************/

/*
** codesets.c
**
** To get the tables compile this with:
**  sc link codesets.c def=BUILD_TABLES
*/

#include <string.h>
#include <stdlib.h>

#include "codesets.h"
#include "codesets_table.h"
#include "lists.h"
#include "support_indep.h"

struct codeset
{
	struct node node;
	char *name;
	const utf8 **to_utf8;
};

static const char trailingBytesForUTF8[256] =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};

struct list codesets_list;

/**************************************************************************
 Returns the supported codesets as an null terminated array
**************************************************************************/
char **codesets_supported(void)
{
	static char **array;
	if (array) return array;
	if ((array = (char**)malloc(sizeof(char*)*(list_length(&codesets_list)+1))))
	{
		struct codeset *code = (struct codeset*)list_first(&codesets_list);
		int i = 0;
		while (code)
		{
			array[i++] = code->name;
			code = (struct codeset*)node_next(&code->node);
		}
		array[i] = NULL;
	}
	return array;
}

/**************************************************************************
 Initialized and loads the codesets
**************************************************************************/
int codesets_init(void)
{
	struct codeset *codeset;
	list_init(&codesets_list);

	if (!(codeset = (struct codeset*)malloc(sizeof(struct codeset)))) return 0;
	codeset->name = mystrdup("ISO Latin 1");
	codeset->to_utf8 = iso_8859_1_to_utf8;
	list_insert_tail(&codesets_list,&codeset->node);

	if (!(codeset = (struct codeset*)malloc(sizeof(struct codeset)))) return 1; /* One entry is enough */
	codeset->name = mystrdup("ISO Latin 2");
	codeset->to_utf8 = iso_8859_2_to_utf8;
	list_insert_tail(&codesets_list,&codeset->node);

	return 1;
}

/**************************************************************************
 Cleanup the memory for the codeset
**************************************************************************/
void codesets_cleanup(void)
{
}

/**************************************************************************
 Returns the to utf8 entry by a special codeset
**************************************************************************/
utf8 **codesets_find_to_utf8(char *codeset_name)
{
	struct codeset *codeset = (struct codeset*)list_first(&codesets_list);
	while (codeset)
	{
		if (!mystricmp(codeset_name,codeset->name)) return codeset->to_utf8;
		codeset = (struct codeset*)node_next(&codeset->node);
	}
	return NULL;
}

/**************************************************************************
 Returns the number of characters a uft8 string has. This is not
 identically with the size of memory is required to hold the string.
 Please use uftsize() for this.
**************************************************************************/
int utf8len(const utf8 *str)
{
	int len ;
	unsigned char c;

	if (!str) return 0;
	len = 0;

	while ((c = *str++))
	{
		len++;
		str += trailingBytesForUTF8[c];
	}

	return len;
}

/**************************************************************************
 Copies a number of characters from "from" to "to". 
**************************************************************************/
utf8 *utf8ncpy(utf8 *to, const utf8 *from, int n)
{
	utf8 *saved_to = to;
	for (;n;n--)
	{
		unsigned char c = *from++;
		int len = trailingBytesForUTF8[c];

		*to++ = c;
		for (;len;len--)
		{
			*to++ = *from++;
		}
	}
	return saved_to;
}

/**************************************************************************
 Creates a uf8 string from a different one. from is the iso string and
 charset the charset of from
**************************************************************************/
utf8 *utf8create(void *from, char *charset)
{
	int dest_size = 0;
	char *dest;
	char *src = (char*)from;
	unsigned char c;
	const utf8 **table = codesets_find_to_utf8(charset);

	if (!table) return NULL;

	while ((c = *src++))
		dest_size += (table[c])[0];

	if ((dest = malloc(dest_size+1)))
	{
		char *dest_ptr = dest;
		src = (char*)from;

		while ((c = *src))
		{
			const unsigned char *utf8_seq = table[c]+1;

			while ((c = *utf8_seq))
			{
				*dest_ptr++ = c;
				utf8_seq++;
			}
			src++;
		}

		*dest_ptr = 0;
		return dest;
	}
	return NULL;
}

#ifdef BUILD_TABLES

/* define BUILD_TABLES and compile and link this file to
** build the uft8 tables
*/

/* ConvertUTF.h */

/*
 * Copyright 2001 Unicode, Inc.
 * 
 * Disclaimer
 * 
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 * 
 * Limitations on Rights to Redistribute This Code
 * 
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 */

/* ---------------------------------------------------------------------

    Conversions between UTF32, UTF-16, and UTF-8.  Header file.

    Several funtions are included here, forming a complete set of
    conversions between the three formats.  UTF-7 is not included
    here, but is handled in a separate source file.

    Each of these routines takes pointers to input buffers and output
    buffers.  The input buffers are const.

    Each routine converts the text between *sourceStart and sourceEnd,
    putting the result into the buffer between *targetStart and
    targetEnd. Note: the end pointers are *after* the last item: e.g. 
    *(sourceEnd - 1) is the last item.

    The return result indicates whether the conversion was successful,
    and if not, whether the problem was in the source or target buffers.
    (Only the first encountered problem is indicated.)

    After the conversion, *sourceStart and *targetStart are both
    updated to point to the end of last text successfully converted in
    the respective buffers.

    Input parameters:
	sourceStart - pointer to a pointer to the source buffer.
		The contents of this are modified on return so that
		it points at the next thing to be converted.
	targetStart - similarly, pointer to pointer to the target buffer.
	sourceEnd, targetEnd - respectively pointers to the ends of the
		two buffers, for overflow checking only.

    These conversion functions take a ConversionFlags argument. When this
    flag is set to strict, both irregular sequences and isolated surrogates
    will cause an error.  When the flag is set to lenient, both irregular
    sequences and isolated surrogates are converted.

    Whether the flag is strict or lenient, all illegal sequences will cause
    an error return. This includes sequences such as: <F4 90 80 80>, <C0 80>,
    or <A0> in UTF-8, and values above 0x10FFFF in UTF-32. Conformant code
    must check for illegal sequences.

    When the flag is set to lenient, characters over 0x10FFFF are converted
    to the replacement character; otherwise (when the flag is set to strict)
    they constitute an error.

    Output parameters:
	The value "sourceIllegal" is returned from some routines if the input
	sequence is malformed.  When "sourceIllegal" is returned, the source
	value will point to the illegal value that caused the problem. E.g.,
	in UTF-8 when a sequence is malformed, it points to the start of the
	malformed sequence.  

    Author: Mark E. Davis, 1994.
    Rev History: Rick McGowan, fixes & updates May 2001.

------------------------------------------------------------------------ */

/* ---------------------------------------------------------------------
    The following 4 definitions are compiler-specific.
    The C standard does not guarantee that wchar_t has at least
    16 bits, so wchar_t is no less portable than unsigned short!
    All should be unsigned values to avoid sign extension during
    bit mask & shift operations.
------------------------------------------------------------------------ */

typedef unsigned long	UTF32;	/* at least 32 bits */
typedef unsigned short	UTF16;	/* at least 16 bits */
typedef unsigned char	UTF8;	/* typically 8 bits */
typedef unsigned char	Boolean; /* 0 or 1 */

/* Some fundamental constants */
#define UNI_REPLACEMENT_CHAR (UTF32)0x0000FFFD
#define UNI_MAX_BMP (UTF32)0x0000FFFF
#define UNI_MAX_UTF16 (UTF32)0x0010FFFF
#define UNI_MAX_UTF32 (UTF32)0x7FFFFFFF

typedef enum {
	conversionOK, 		/* conversion successful */
	sourceExhausted,	/* partial character in source, but hit end */
	targetExhausted,	/* insuff. room in target for conversion */
	sourceIllegal		/* source sequence is illegal/malformed */
} ConversionResult;

typedef enum {
	strictConversion = 0,
	lenientConversion
} ConversionFlags;

ConversionResult ConvertUTF32toUTF16 (
		UTF32** sourceStart, const UTF32* sourceEnd, 
		UTF16** targetStart, const UTF16* targetEnd, const ConversionFlags flags);

ConversionResult ConvertUTF16toUTF32 (
		UTF16** sourceStart, UTF16* sourceEnd, 
		UTF32** targetStart, const UTF32* targetEnd, const ConversionFlags flags);

ConversionResult ConvertUTF16toUTF8 (
		UTF16** sourceStart, const UTF16* sourceEnd, 
		UTF8** targetStart, const UTF8* targetEnd, ConversionFlags flags);
		
ConversionResult ConvertUTF8toUTF16 (
		UTF8** sourceStart, UTF8* sourceEnd, 
		UTF16** targetStart, const UTF16* targetEnd, const ConversionFlags flags);

ConversionResult ConvertUTF32toUTF8 (
		UTF32** sourceStart, const UTF32* sourceEnd, 
		UTF8** targetStart, const UTF8* targetEnd, ConversionFlags flags);
		
ConversionResult ConvertUTF8toUTF32 (
		UTF8** sourceStart, UTF8* sourceEnd, 
		UTF32** targetStart, const UTF32* targetEnd, ConversionFlags flags);

Boolean isLegalUTF8Sequence(UTF8 *source, UTF8 *sourceEnd);

/* --------------------------------------------------------------------- */

/* ConvertUTF.c */

/*
 * Copyright 2001 Unicode, Inc.
 * 
 * Disclaimer
 * 
 * This source code is provided as is by Unicode, Inc. No claims are
 * made as to fitness for any particular purpose. No warranties of any
 * kind are expressed or implied. The recipient agrees to determine
 * applicability of information provided. If this file has been
 * purchased on magnetic or optical media from Unicode, Inc., the
 * sole remedy for any claim will be exchange of defective media
 * within 90 days of receipt.
 * 
 * Limitations on Rights to Redistribute This Code
 * 
 * Unicode, Inc. hereby grants the right to freely use the information
 * supplied in this file in the creation of products supporting the
 * Unicode Standard, and to make copies of this file in any form
 * for internal or external distribution as long as this notice
 * remains attached.
 */

/* ---------------------------------------------------------------------

    Conversions between UTF32, UTF-16, and UTF-8. Source code file.
	Author: Mark E. Davis, 1994.
	Rev History: Rick McGowan, fixes & updates May 2001.

    See the header file "ConvertUTF.h" for complete documentation.

------------------------------------------------------------------------ */


/*#include "ConvertUTF.h"*/
/*#ifdef CVTUTF_DEBUG*/
#include <stdio.h>
/*#endif*/

static const int halfShift	= 10; /* used for shifting by 10 bits */

static const UTF32 halfBase	= 0x0010000UL;
static const UTF32 halfMask	= 0x3FFUL;

#define UNI_SUR_HIGH_START	(UTF32)0xD800
#define UNI_SUR_HIGH_END	(UTF32)0xDBFF
#define UNI_SUR_LOW_START	(UTF32)0xDC00
#define UNI_SUR_LOW_END		(UTF32)0xDFFF
#define false			0
#define true			1

/* --------------------------------------------------------------------- */

ConversionResult ConvertUTF32toUTF16 (
		UTF32** sourceStart, const UTF32* sourceEnd, 
		UTF16** targetStart, const UTF16* targetEnd, const ConversionFlags flags) {
	ConversionResult result = conversionOK;
	UTF32* source = *sourceStart;
	UTF16* target = *targetStart;
	while (source < sourceEnd) {
		UTF32 ch;
		if (target >= targetEnd) {
			result = targetExhausted; break;
		}
		ch = *source++;
		if (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
			if ((flags == strictConversion) && (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END)) {
				--source; /* return to the illegal value itself */
				result = sourceIllegal;
				break;
			} else {
			    *target++ = ch;	/* normal case */
			}
		} else if (ch > UNI_MAX_UTF16) {
			if (flags == strictConversion) {
				result = sourceIllegal;
			} else {
				*target++ = UNI_REPLACEMENT_CHAR;
			}
		} else {
			/* target is a character in range 0xFFFF - 0x10FFFF. */
			if (target + 1 >= targetEnd) {
				result = targetExhausted; break;
			}
			ch -= halfBase;
			*target++ = (ch >> halfShift) + UNI_SUR_HIGH_START;
			*target++ = (ch & halfMask) + UNI_SUR_LOW_START;
		}
	}
	*sourceStart = source;
	*targetStart = target;
	return result;
}

/* --------------------------------------------------------------------- */

ConversionResult ConvertUTF16toUTF32 (
		UTF16** sourceStart, UTF16* sourceEnd, 
		UTF32** targetStart, const UTF32* targetEnd, const ConversionFlags flags) {
	ConversionResult result = conversionOK;
	UTF16* source = *sourceStart;
	UTF32* target = *targetStart;
	UTF32 ch, ch2;
	while (source < sourceEnd) {
		ch = *source++;
		if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END && source < sourceEnd) {
			ch2 = *source;
			if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
				ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
					+ (ch2 - UNI_SUR_LOW_START) + halfBase;
				++source;
			} else if (flags == strictConversion) { /* it's an unpaired high surrogate */
				--source; /* return to the illegal value itself */
				result = sourceIllegal;
				break;
			}
		} else if ((flags == strictConversion) && (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END)) {
			/* an unpaired low surrogate */
			--source; /* return to the illegal value itself */
			result = sourceIllegal;
			break;
		}
		if (target >= targetEnd) {
			result = targetExhausted; break;
		}
		*target++ = ch;
	}
	*sourceStart = source;
	*targetStart = target;
#ifdef CVTUTF_DEBUG
if (result == sourceIllegal) {
    fprintf(stderr, "ConvertUTF16toUTF32 illegal seq 0x%04x,%04x\n", ch, ch2);
    fflush(stderr);
}
#endif
	return result;
}

/* --------------------------------------------------------------------- */

/*
 * Index into the table below with the first byte of a UTF-8 sequence to
 * get the number of trailing bytes that are supposed to follow it.
 */
/*
static const char trailingBytesForUTF8[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
};
*/
/*
 * Magic values subtracted from a buffer value during UTF8 conversion.
 * This table contains as many values as there might be trailing bytes
 * in a UTF-8 sequence.
 */
static const UTF32 offsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 
					 0x03C82080UL, 0xFA082080UL, 0x82082080UL };

/*
 * Once the bits are split out into bytes of UTF-8, this is a mask OR-ed
 * into the first byte, depending on how many bytes follow.  There are
 * as many entries in this table as there are UTF-8 sequence types.
 * (I.e., one byte sequence, two byte... six byte sequence.)
 */
static const UTF8 firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/* --------------------------------------------------------------------- */

/* The interface converts a whole buffer to avoid function-call overhead.
 * Constants have been gathered. Loops & conditionals have been removed as
 * much as possible for efficiency, in favor of drop-through switches.
 * (See "Note A" at the bottom of the file for equivalent code.)
 * If your compiler supports it, the "isLegalUTF8" call can be turned
 * into an inline function.
 */

/* --------------------------------------------------------------------- */

ConversionResult ConvertUTF16toUTF8 (
		UTF16** sourceStart, const UTF16* sourceEnd, 
		UTF8** targetStart, const UTF8* targetEnd, ConversionFlags flags) {
	ConversionResult result = conversionOK;
	UTF16* source = *sourceStart;
	UTF8* target = *targetStart;
	while (source < sourceEnd) {
		UTF32 ch;
		unsigned short bytesToWrite = 0;
		const UTF32 byteMask = 0xBF;
		const UTF32 byteMark = 0x80; 
		ch = *source++;
		/* If we have a surrogate pair, convert to UTF32 first. */
		if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END && source < sourceEnd) {
			UTF32 ch2 = *source;
			if (ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) {
				ch = ((ch - UNI_SUR_HIGH_START) << halfShift)
					+ (ch2 - UNI_SUR_LOW_START) + halfBase;
				++source;
			} else if (flags == strictConversion) { /* it's an unpaired high surrogate */
				--source; /* return to the illegal value itself */
				result = sourceIllegal;
				break;
			}
		} else if ((flags == strictConversion) && (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END)) {
			--source; /* return to the illegal value itself */
			result = sourceIllegal;
			break;
		}
		/* Figure out how many bytes the result will require */
		if (ch < (UTF32)0x80) {			bytesToWrite = 1;
		} else if (ch < (UTF32)0x800) {		bytesToWrite = 2;
		} else if (ch < (UTF32)0x10000) {	bytesToWrite = 3;
		} else if (ch < (UTF32)0x200000) {	bytesToWrite = 4;
		} else {				bytesToWrite = 2;
							ch = UNI_REPLACEMENT_CHAR;
		}

		target += bytesToWrite;
		if (target > targetEnd) {
			target -= bytesToWrite; result = targetExhausted; break;
		}
		switch (bytesToWrite) {	/* note: everything falls through. */
			case 4:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
			case 3:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
			case 2:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
			case 1:	*--target =  ch | firstByteMark[bytesToWrite];
		}
		target += bytesToWrite;
	}
	*sourceStart = source;
	*targetStart = target;
	return result;
}

/* --------------------------------------------------------------------- */

/*
 * Utility routine to tell whether a sequence of bytes is legal UTF-8.
 * This must be called with the length pre-determined by the first byte.
 * If not calling this from ConvertUTF8to*, then the length can be set by:
 *	length = trailingBytesForUTF8[*source]+1;
 * and the sequence is illegal right away if there aren't that many bytes
 * available.
 * If presented with a length > 4, this returns false.  The Unicode
 * definition of UTF-8 goes up to 4-byte sequences.
 */

static Boolean isLegalUTF8(UTF8 *source, int length) {
	UTF8 a;
	UTF8 *srcptr = source+length;
	switch (length) {
	default: return false;
		/* Everything else falls through when "true"... */
	case 4: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
	case 3: if ((a = (*--srcptr)) < 0x80 || a > 0xBF) return false;
	case 2: if ((a = (*--srcptr)) > 0xBF) return false;
		switch (*source) {
		    /* no fall-through in this inner switch */
		    case 0xE0: if (a < 0xA0) return false; break;
		    case 0xF0: if (a < 0x90) return false; break;
		    case 0xF4: if (a > 0x8F) return false; break;
		    default:  if (a < 0x80) return false;
		}
    	case 1: if (*source >= 0x80 && *source < 0xC2) return false;
		if (*source > 0xF4) return false;
	}
	return true;
}

/* --------------------------------------------------------------------- */

/*
 * Exported function to return whether a UTF-8 sequence is legal or not.
 * This is not used here; it's just exported.
 */
Boolean isLegalUTF8Sequence(UTF8 *source, UTF8 *sourceEnd) {
	int length = trailingBytesForUTF8[*source]+1;
	if (source+length > sourceEnd) {
	    return false;
	}
	return isLegalUTF8(source, length);
}

/* --------------------------------------------------------------------- */

ConversionResult ConvertUTF8toUTF16 (
		UTF8** sourceStart, UTF8* sourceEnd, 
		UTF16** targetStart, const UTF16* targetEnd, const ConversionFlags flags) {
	ConversionResult result = conversionOK;
	UTF8* source = *sourceStart;
	UTF16* target = *targetStart;
	while (source < sourceEnd) {
		UTF32 ch = 0;
		unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
		if (source + extraBytesToRead >= sourceEnd) {
			result = sourceExhausted; break;
		}
		/* Do this check whether lenient or strict */
		if (! isLegalUTF8(source, extraBytesToRead+1)) {
			result = sourceIllegal;
			break;
		}
		/*
		 * The cases all fall through. See "Note A" below.
		 */
		switch (extraBytesToRead) {
			case 3:	ch += *source++; ch <<= 6;
			case 2:	ch += *source++; ch <<= 6;
			case 1:	ch += *source++; ch <<= 6;
			case 0:	ch += *source++;
		}
		ch -= offsetsFromUTF8[extraBytesToRead];

		if (target >= targetEnd) {
			result = targetExhausted; break;
		}
		if (ch <= UNI_MAX_BMP) { /* Target is a character <= 0xFFFF */
			if ((flags == strictConversion) && (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_LOW_END)) {
				--source; /* return to the illegal value itself */
				result = sourceIllegal;
				break;
			} else {
			    *target++ = ch;	/* normal case */
			}
		} else if (ch > UNI_MAX_UTF16) {
			if (flags == strictConversion) {
				result = sourceIllegal;
				source -= extraBytesToRead; /* return to the start */
			} else {
				*target++ = UNI_REPLACEMENT_CHAR;
			}
		} else {
			/* target is a character in range 0xFFFF - 0x10FFFF. */
			if (target + 1 >= targetEnd) {
				result = targetExhausted; break;
			}
			ch -= halfBase;
			*target++ = (ch >> halfShift) + UNI_SUR_HIGH_START;
			*target++ = (ch & halfMask) + UNI_SUR_LOW_START;
		}
	}
	*sourceStart = source;
	*targetStart = target;
	return result;
}

/* --------------------------------------------------------------------- */

ConversionResult ConvertUTF32toUTF8 (
		UTF32** sourceStart, const UTF32* sourceEnd, 
		UTF8** targetStart, const UTF8* targetEnd, ConversionFlags flags) {
	ConversionResult result = conversionOK;
	UTF32* source = *sourceStart;
	UTF8* target = *targetStart;
	while (source < sourceEnd) {
		UTF32 ch;
		unsigned short bytesToWrite = 0;
		const UTF32 byteMask = 0xBF;
		const UTF32 byteMark = 0x80; 
		ch = *source++;
		/* surrogates of any stripe are not legal UTF32 characters */
		if (flags == strictConversion ) {
			if ((ch >= UNI_SUR_HIGH_START) && (ch <= UNI_SUR_LOW_END)) {
				--source; /* return to the illegal value itself */
				result = sourceIllegal;
				break;
			}
		}
		/* Figure out how many bytes the result will require */
		if (ch < (UTF32)0x80) {			bytesToWrite = 1;
		} else if (ch < (UTF32)0x800) {		bytesToWrite = 2;
		} else if (ch < (UTF32)0x10000) {	bytesToWrite = 3;
		} else if (ch < (UTF32)0x200000) {	bytesToWrite = 4;
		} else {				bytesToWrite = 2;
							ch = UNI_REPLACEMENT_CHAR;
		}
		
		target += bytesToWrite;
		if (target > targetEnd) {
			target -= bytesToWrite; result = targetExhausted; break;
		}
		switch (bytesToWrite) {	/* note: everything falls through. */
			case 4:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
			case 3:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
			case 2:	*--target = (ch | byteMark) & byteMask; ch >>= 6;
			case 1:	*--target =  ch | firstByteMark[bytesToWrite];
		}
		target += bytesToWrite;
	}
	*sourceStart = source;
	*targetStart = target;
	return result;
}

/* --------------------------------------------------------------------- */

ConversionResult ConvertUTF8toUTF32 (
		UTF8** sourceStart, UTF8* sourceEnd, 
		UTF32** targetStart, const UTF32* targetEnd, ConversionFlags flags) {
	ConversionResult result = conversionOK;
	UTF8* source = *sourceStart;
	UTF32* target = *targetStart;
	while (source < sourceEnd) {
		UTF32 ch = 0;
		unsigned short extraBytesToRead = trailingBytesForUTF8[*source];
		if (source + extraBytesToRead >= sourceEnd) {
			result = sourceExhausted; break;
		}
		/* Do this check whether lenient or strict */
		if (! isLegalUTF8(source, extraBytesToRead+1)) {
			result = sourceIllegal;
			break;
		}
		/*
		 * The cases all fall through. See "Note A" below.
		 */
		switch (extraBytesToRead) {
			case 3:	ch += *source++; ch <<= 6;
			case 2:	ch += *source++; ch <<= 6;
			case 1:	ch += *source++; ch <<= 6;
			case 0:	ch += *source++;
		}
		ch -= offsetsFromUTF8[extraBytesToRead];

		if (target >= targetEnd) {
			result = targetExhausted; break;
		}
		if (ch <= UNI_MAX_UTF32) {
			*target++ = ch;
		} else if (ch > UNI_MAX_UTF32) {
			*target++ = UNI_REPLACEMENT_CHAR;
		} else {
			if (target + 1 >= targetEnd) {
				result = targetExhausted; break;
			}
			ch -= halfBase;
			*target++ = (ch >> halfShift) + UNI_SUR_HIGH_START;
			*target++ = (ch & halfMask) + UNI_SUR_LOW_START;
		}
	}
	*sourceStart = source;
	*targetStart = target;
	return result;
}

/* ---------------------------------------------------------------------

	Note A.
	The fall-through switches in UTF-8 reading code save a
	temp variable, some decrements & conditionals.  The switches
	are equivalent to the following loop:
		{
			int tmpBytesToRead = extraBytesToRead+1;
			do {
				ch += *source++;
				--tmpBytesToRead;
				if (tmpBytesToRead) ch <<= 6;
			} while (tmpBytesToRead > 0);
		}
	In UTF-8 writing code, the switches on "bytesToWrite" are
	similarly unrolled loops.

   --------------------------------------------------------------------- */

void main(void)
{
	int i;
	UTF32 src;
	UTF8 dest[16];

	for (i=0;i<256;i++)
	{
		UTF32 *src_ptr = &src;
		UTF8 *dest_ptr = dest;
		int j;

/*		src = i;*/
		if (i < 128) src = i;
		else src = iso_8859_2_to_ucs4[i-128];

		ConvertUTF32toUTF8 (&src_ptr, src_ptr + 1, &dest_ptr, dest + 16, strictConversion);

		*dest_ptr = 0;

		printf("  \"\\0%03lo",strlen(dest));
		for (j=0;dest[j];j++)
		{
			printf("\\%03lo",(int)dest[j]);

		}
		printf("\", /* 0x%04lx */\n",src);
	}
}

#endif