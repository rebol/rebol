/***********************************************************************
**
**  REBOL [R3] Language Interpreter and Run-time Environment
**
**  Copyright 2012 REBOL Technologies
**  Copyright 2012-2019 Rebol Open Source Developers
**  REBOL is a trademark of REBOL Technologies
**
**  Licensed under the Apache License, Version 2.0 (the "License");
**  you may not use this file except in compliance with the License.
**  You may obtain a copy of the License at
**
**  http://www.apache.org/licenses/LICENSE-2.0
**
**  Unless required by applicable law or agreed to in writing, software
**  distributed under the License is distributed on an "AS IS" BASIS,
**  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**  See the License for the specific language governing permissions and
**  limitations under the License.
**
************************************************************************
**
**  Module:  u-iconv.c
**  Summary: codepage conversions
**  Section: utility
**  Author:  Oldes
**	Notes:   This code is using call to iconv function on Posix
**	         and partially emulates it on Windows
**	TODO:    
**			* handle invalid char substitution
**			* utf-32 support on Windows (must be coded specially)
**			* utf-7  support on Windows (must be coded specially)
**			* deal with BOM on Windows
**
***********************************************************************/

#if defined(TO_WIN32) || defined(TO_WIN32_X64)
#include <windows.h>
#undef IS_ERROR
#else
#include <iconv.h>
#include <errno.h>
#endif

#include "sys-core.h"


// Codepage aliases borrowed from: https://github.com/win-iconv/win-iconv/blob/master/win_iconv.c
// (slightly modified for preferences, because it looks some iconv posix variants does not recognize CP1200 etc)
// On macOS it looks there are not aliased utf16 variants, one must use utf-16!
// 
static struct {
	REBINT codepage;
	const char *name;
} codepage_alias[] = {
	{65001, "UTF-8"},
	{65001, "UTF8"},
	{65001, "CP65001"},

	{1200, "UTF-16LE"},
	{1200, "UTF16LE"},
	{1200, "UCS-2LE"},
	{1200, "UCS2LE"},
	{1200, "UCS-2-INTERNAL"},
	{1200, "CP1200"},

	{1201, "UTF-16BE"},
	{1201, "UTF16BE"},
	{1201, "UCS-2BE"},
	{1201, "UCS2BE"},
	{1201, "unicodeFFFE"},
	{1201, "CP1201"},

	{12000, "CP12000"},
	{12000, "UTF-32LE"},
	{12000, "UTF32LE"},
	{12000, "UCS-4LE"},
	{12000, "UCS4LE"},

	{12001, "UTF-32BE"},
	{12001, "UTF32BE"},
	{12001, "UCS-4BE"},
	{12001, "UCS4BE"},
	{12001, "CP12001"},

//#ifndef GLIB_COMPILATION
//    /*
//     * Default is big endian.
//     * See rfc2781 4.3 Interpreting text labelled as UTF-16.
//     */
//    {1201, "UTF16"},
//    {1201, "UTF-16"},
//    {1201, "UCS2"},
//    {1201, "UCS-2"},
//    {12001, "UTF32"},
//    {12001, "UTF-32"},
//    {12001, "UCS-4"},
//    {12001, "UCS4"},
//#else
	/* Default is little endian, because the platform is */
	{1200, "UTF-16"},
	{1200, "UTF16"},
	{1200, "UCS-2"},
	{1200, "UCS2"},
	{12000, "UTF-32"},
	{12000, "UTF32"},
	{12000, "UCS-4"},
	{12000, "UCS4"},
//#endif

	/* copy from libiconv `iconv -l` */
	/* !IsValidCodePage(367) */
	{20127, "ANSI_X3.4-1968"},
	{20127, "ANSI_X3.4-1986"},
	{20127, "ASCII"},
	{20127, "CP367"},
	{20127, "IBM367"},
	{20127, "ISO-IR-6"},
	{20127, "ISO646-US"},
	{20127, "ISO_646.IRV:1991"},
	{20127, "US"},
	{20127, "US-ASCII"},
	{20127, "CSASCII"},

	/* !IsValidCodePage(819) */
	{1252, "CP819"},
	{1252, "IBM819"},
	{28591, "ISO-8859-1"},
	{28591, "ISO-IR-100"},
	{28591, "ISO8859-1"},
	{28591, "ISO_8859-1"},
	{28591, "ISO_8859-1:1987"},
	{28591, "L1"},
	{28591, "LATIN1"},
	{28591, "CSISOLATIN1"},

	{1250, "CP1250"},
	{1250, "MS-EE"},
	{1250, "WINDOWS-1250"},

	{1251, "CP1251"},
	{1251, "MS-CYRL"},
	{1251, "WINDOWS-1251"},

	{1252, "CP1252"},
	{1252, "MS-ANSI"},
	{1252, "WINDOWS-1252"},

	{1253, "CP1253"},
	{1253, "MS-GREEK"},
	{1253, "WINDOWS-1253"},

	{1254, "CP1254"},
	{1254, "MS-TURK"},
	{1254, "WINDOWS-1254"},

	{1255, "CP1255"},
	{1255, "MS-HEBR"},
	{1255, "WINDOWS-1255"},

	{1256, "CP1256"},
	{1256, "MS-ARAB"},
	{1256, "WINDOWS-1256"},

	{1257, "CP1257"},
	{1257, "WINBALTRIM"},
	{1257, "WINDOWS-1257"},

	{1258, "CP1258"},
	{1258, "WINDOWS-1258"},

	{850, "850"},
	{850, "CP850"},
	{850, "IBM850"},
	{850, "CSPC850MULTILINGUAL"},

	/* !IsValidCodePage(862) */
	{862, "862"},
	{862, "CP862"},
	{862, "IBM862"},
	{862, "CSPC862LATINHEBREW"},

	{866, "866"},
	{866, "CP866"},
	{866, "IBM866"},
	{866, "CSIBM866"},

	/* !IsValidCodePage(154) */
	{154, "CP154"},
	{154, "CYRILLIC-ASIAN"},
	{154, "PT154"},
	{154, "PTCP154"},
	{154, "CSPTCP154"},

	/* !IsValidCodePage(1133) */
	{1133, "CP1133"},
	{1133, "IBM-CP1133"},

	{874, "CP874"},
	{874, "WINDOWS-874"},

	/* !IsValidCodePage(51932) */
	{51932, "CP51932"},
	{51932, "MS51932"},
	{51932, "WINDOWS-51932"},
	{51932, "EUC-JP"},

	{932, "CP932"},
	{932, "MS932"},
	{932, "SHIFFT_JIS"},
	{932, "SHIFFT_JIS-MS"},
	{932, "SJIS"},
	{932, "SJIS-MS"},
	{932, "SJIS-OPEN"},
	{932, "SJIS-WIN"},
	{932, "WINDOWS-31J"},
	{932, "WINDOWS-932"},
	{932, "CSWINDOWS31J"},

	{50221, "CP50221"},
	{50221, "ISO-2022-JP"},
	{50221, "ISO-2022-JP-MS"},
	{50221, "ISO2022-JP"},
	{50221, "ISO2022-JP-MS"},
	{50221, "MS50221"},
	{50221, "WINDOWS-50221"},

	{936, "CP936"},
	{936, "GBK"},
	{936, "MS936"},
	{936, "WINDOWS-936"},

	{950, "CP950"},
	{950, "BIG5"},
	{950, "BIG5HKSCS"},
	{950, "BIG5-HKSCS"},

	{949, "CP949"},
	{949, "UHC"},
	{949, "EUC-KR"},

	{1361, "CP1361"},
	{1361, "JOHAB"},

	{437, "437"},
	{437, "CP437"},
	{437, "IBM437"},
	{437, "CSPC8CODEPAGE437"},

	{737, "CP737"},

	{775, "CP775"},
	{775, "IBM775"},
	{775, "CSPC775BALTIC"},

	{852, "852"},
	{852, "CP852"},
	{852, "IBM852"},
	{852, "CSPCP852"},

	/* !IsValidCodePage(853) */
	{853, "CP853"},

	{855, "855"},
	{855, "CP855"},
	{855, "IBM855"},
	{855, "CSIBM855"},

	{857, "857"},
	{857, "CP857"},
	{857, "IBM857"},
	{857, "CSIBM857"},

	/* !IsValidCodePage(858) */
	{858, "CP858"},

	{860, "860"},
	{860, "CP860"},
	{860, "IBM860"},
	{860, "CSIBM860"},

	{861, "861"},
	{861, "CP-IS"},
	{861, "CP861"},
	{861, "IBM861"},
	{861, "CSIBM861"},

	{863, "863"},
	{863, "CP863"},
	{863, "IBM863"},
	{863, "CSIBM863"},

	{864, "CP864"},
	{864, "IBM864"},
	{864, "CSIBM864"},

	{865, "865"},
	{865, "CP865"},
	{865, "IBM865"},
	{865, "CSIBM865"},

	{869, "869"},
	{869, "CP-GR"},
	{869, "CP869"},
	{869, "IBM869"},
	{869, "CSIBM869"},

	/* !IsValidCodePage(1152) */
	{1125, "CP1125"},

	/*
	 * Code Page Identifiers
	 * http://msdn2.microsoft.com/en-us/library/ms776446.aspx
	 */
	{37, "IBM037"}, /* IBM EBCDIC US-Canada */
	{437, "IBM437"}, /* OEM United States */
	{500, "IBM500"}, /* IBM EBCDIC International */
	{708, "ASMO-708"}, /* Arabic (ASMO 708) */
	/* 709 		Arabic (ASMO-449+, BCON V4) */
	/* 710 		Arabic - Transparent Arabic */
	{720, "DOS-720"}, /* Arabic (Transparent ASMO); Arabic (DOS) */
	{737, "ibm737"}, /* OEM Greek (formerly 437G); Greek (DOS) */
	{775, "ibm775"}, /* OEM Baltic; Baltic (DOS) */
	{850, "ibm850"}, /* OEM Multilingual Latin 1; Western European (DOS) */
	{852, "ibm852"}, /* OEM Latin 2; Central European (DOS) */
	{855, "IBM855"}, /* OEM Cyrillic (primarily Russian) */
	{857, "ibm857"}, /* OEM Turkish; Turkish (DOS) */
	{858, "IBM00858"}, /* OEM Multilingual Latin 1 + Euro symbol */
	{860, "IBM860"}, /* OEM Portuguese; Portuguese (DOS) */
	{861, "ibm861"}, /* OEM Icelandic; Icelandic (DOS) */
	{862, "DOS-862"}, /* OEM Hebrew; Hebrew (DOS) */
	{863, "IBM863"}, /* OEM French Canadian; French Canadian (DOS) */
	{864, "IBM864"}, /* OEM Arabic; Arabic (864) */
	{865, "IBM865"}, /* OEM Nordic; Nordic (DOS) */
	{866, "cp866"}, /* OEM Russian; Cyrillic (DOS) */
	{869, "ibm869"}, /* OEM Modern Greek; Greek, Modern (DOS) */
	{870, "IBM870"}, /* IBM EBCDIC Multilingual/ROECE (Latin 2); IBM EBCDIC Multilingual Latin 2 */
	{874, "windows-874"}, /* ANSI/OEM Thai (same as 28605, ISO 8859-15); Thai (Windows) */
	{875, "cp875"}, /* IBM EBCDIC Greek Modern */
	{932, "shift_jis"}, /* ANSI/OEM Japanese; Japanese (Shift-JIS) */
	{932, "shift-jis"}, /* alternative name for it */
	{936, "gb2312"}, /* ANSI/OEM Simplified Chinese (PRC, Singapore); Chinese Simplified (GB2312) */
	{949, "ks_c_5601-1987"}, /* ANSI/OEM Korean (Unified Hangul Code) */
	{950, "big5"}, /* ANSI/OEM Traditional Chinese (Taiwan; Hong Kong SAR, PRC); Chinese Traditional (Big5) */
	{950, "big5hkscs"}, /* ANSI/OEM Traditional Chinese (Hong Kong SAR); Chinese Traditional (Big5-HKSCS) */
	{950, "big5-hkscs"}, /* alternative name for it */
	{1026, "IBM1026"}, /* IBM EBCDIC Turkish (Latin 5) */
	{1047, "IBM01047"}, /* IBM EBCDIC Latin 1/Open System */
	{1140, "IBM01140"}, /* IBM EBCDIC US-Canada (037 + Euro symbol); IBM EBCDIC (US-Canada-Euro) */
	{1141, "IBM01141"}, /* IBM EBCDIC Germany (20273 + Euro symbol); IBM EBCDIC (Germany-Euro) */
	{1142, "IBM01142"}, /* IBM EBCDIC Denmark-Norway (20277 + Euro symbol); IBM EBCDIC (Denmark-Norway-Euro) */
	{1143, "IBM01143"}, /* IBM EBCDIC Finland-Sweden (20278 + Euro symbol); IBM EBCDIC (Finland-Sweden-Euro) */
	{1144, "IBM01144"}, /* IBM EBCDIC Italy (20280 + Euro symbol); IBM EBCDIC (Italy-Euro) */
	{1145, "IBM01145"}, /* IBM EBCDIC Latin America-Spain (20284 + Euro symbol); IBM EBCDIC (Spain-Euro) */
	{1146, "IBM01146"}, /* IBM EBCDIC United Kingdom (20285 + Euro symbol); IBM EBCDIC (UK-Euro) */
	{1147, "IBM01147"}, /* IBM EBCDIC France (20297 + Euro symbol); IBM EBCDIC (France-Euro) */
	{1148, "IBM01148"}, /* IBM EBCDIC International (500 + Euro symbol); IBM EBCDIC (International-Euro) */
	{1149, "IBM01149"}, /* IBM EBCDIC Icelandic (20871 + Euro symbol); IBM EBCDIC (Icelandic-Euro) */
	{1250, "windows-1250"}, /* ANSI Central European; Central European (Windows) */
	{1251, "windows-1251"}, /* ANSI Cyrillic; Cyrillic (Windows) */
	{1252, "windows-1252"}, /* ANSI Latin 1; Western European (Windows) */
	{1253, "windows-1253"}, /* ANSI Greek; Greek (Windows) */
	{1254, "windows-1254"}, /* ANSI Turkish; Turkish (Windows) */
	{1255, "windows-1255"}, /* ANSI Hebrew; Hebrew (Windows) */
	{1256, "windows-1256"}, /* ANSI Arabic; Arabic (Windows) */
	{1257, "windows-1257"}, /* ANSI Baltic; Baltic (Windows) */
	{1258, "windows-1258"}, /* ANSI/OEM Vietnamese; Vietnamese (Windows) */
	{1361, "Johab"}, /* Korean (Johab) */
	{10000, "macintosh"}, /* MAC Roman; Western European (Mac) */
	{10001, "x-mac-japanese"}, /* Japanese (Mac) */
	{10002, "x-mac-chinesetrad"}, /* MAC Traditional Chinese (Big5); Chinese Traditional (Mac) */
	{10003, "x-mac-korean"}, /* Korean (Mac) */
	{10004, "x-mac-arabic"}, /* Arabic (Mac) */
	{10005, "x-mac-hebrew"}, /* Hebrew (Mac) */
	{10006, "x-mac-greek"}, /* Greek (Mac) */
	{10007, "x-mac-cyrillic"}, /* Cyrillic (Mac) */
	{10008, "x-mac-chinesesimp"}, /* MAC Simplified Chinese (GB 2312); Chinese Simplified (Mac) */
	{10010, "x-mac-romanian"}, /* Romanian (Mac) */
	{10017, "x-mac-ukrainian"}, /* Ukrainian (Mac) */
	{10021, "x-mac-thai"}, /* Thai (Mac) */
	{10029, "x-mac-ce"}, /* MAC Latin 2; Central European (Mac) */
	{10079, "x-mac-icelandic"}, /* Icelandic (Mac) */
	{10081, "x-mac-turkish"}, /* Turkish (Mac) */
	{10082, "x-mac-croatian"}, /* Croatian (Mac) */
	{20000, "x-Chinese_CNS"}, /* CNS Taiwan; Chinese Traditional (CNS) */
	{20001, "x-cp20001"}, /* TCA Taiwan */
	{20002, "x_Chinese-Eten"}, /* Eten Taiwan; Chinese Traditional (Eten) */
	{20003, "x-cp20003"}, /* IBM5550 Taiwan */
	{20004, "x-cp20004"}, /* TeleText Taiwan */
	{20005, "x-cp20005"}, /* Wang Taiwan */
	{20105, "x-IA5"}, /* IA5 (IRV International Alphabet No. 5, 7-bit); Western European (IA5) */
	{20106, "x-IA5-German"}, /* IA5 German (7-bit) */
	{20107, "x-IA5-Swedish"}, /* IA5 Swedish (7-bit) */
	{20108, "x-IA5-Norwegian"}, /* IA5 Norwegian (7-bit) */
	{20127, "us-ascii"}, /* US-ASCII (7-bit) */
	{20261, "x-cp20261"}, /* T.61 */
	{20269, "x-cp20269"}, /* ISO 6937 Non-Spacing Accent */
	{20273, "IBM273"}, /* IBM EBCDIC Germany */
	{20277, "IBM277"}, /* IBM EBCDIC Denmark-Norway */
	{20278, "IBM278"}, /* IBM EBCDIC Finland-Sweden */
	{20280, "IBM280"}, /* IBM EBCDIC Italy */
	{20284, "IBM284"}, /* IBM EBCDIC Latin America-Spain */
	{20285, "IBM285"}, /* IBM EBCDIC United Kingdom */
	{20290, "IBM290"}, /* IBM EBCDIC Japanese Katakana Extended */
	{20297, "IBM297"}, /* IBM EBCDIC France */
	{20420, "IBM420"}, /* IBM EBCDIC Arabic */
	{20423, "IBM423"}, /* IBM EBCDIC Greek */
	{20424, "IBM424"}, /* IBM EBCDIC Hebrew */
	{20833, "x-EBCDIC-KoreanExtended"}, /* IBM EBCDIC Korean Extended */
	{20838, "IBM-Thai"}, /* IBM EBCDIC Thai */
	{20866, "koi8-r"}, /* Russian (KOI8-R); Cyrillic (KOI8-R) */
	{20871, "IBM871"}, /* IBM EBCDIC Icelandic */
	{20880, "IBM880"}, /* IBM EBCDIC Cyrillic Russian */
	{20905, "IBM905"}, /* IBM EBCDIC Turkish */
	{20924, "IBM00924"}, /* IBM EBCDIC Latin 1/Open System (1047 + Euro symbol) */
	{20932, "EUC-JP"}, /* Japanese (JIS 0208-1990 and 0121-1990) */
	{20936, "x-cp20936"}, /* Simplified Chinese (GB2312); Chinese Simplified (GB2312-80) */
	{20949, "x-cp20949"}, /* Korean Wansung */
	{21025, "cp1025"}, /* IBM EBCDIC Cyrillic Serbian-Bulgarian */
	/* 21027 		(deprecated) */
	{21866, "koi8-u"}, /* Ukrainian (KOI8-U); Cyrillic (KOI8-U) */
	{28591, "iso-8859-1"}, /* ISO 8859-1 Latin 1; Western European (ISO) */
	{28591, "iso8859-1"}, /* ISO 8859-1 Latin 1; Western European (ISO) */
	{28591, "iso_8859-1"},
	{28591, "iso_8859_1"},
	{28592, "iso-8859-2"}, /* ISO 8859-2 Central European; Central European (ISO) */
	{28592, "iso8859-2"}, /* ISO 8859-2 Central European; Central European (ISO) */
	{28592, "iso_8859-2"},
	{28592, "iso_8859_2"},
	{28593, "iso-8859-3"}, /* ISO 8859-3 Latin 3 */
	{28593, "iso8859-3"}, /* ISO 8859-3 Latin 3 */
	{28593, "iso_8859-3"},
	{28593, "iso_8859_3"},
	{28594, "iso-8859-4"}, /* ISO 8859-4 Baltic */
	{28594, "iso8859-4"}, /* ISO 8859-4 Baltic */
	{28594, "iso_8859-4"},
	{28594, "iso_8859_4"},
	{28595, "iso-8859-5"}, /* ISO 8859-5 Cyrillic */
	{28595, "iso8859-5"}, /* ISO 8859-5 Cyrillic */
	{28595, "iso_8859-5"},
	{28595, "iso_8859_5"},
	{28596, "iso-8859-6"}, /* ISO 8859-6 Arabic */
	{28596, "iso8859-6"}, /* ISO 8859-6 Arabic */
	{28596, "iso_8859-6"},
	{28596, "iso_8859_6"},
	{28597, "iso-8859-7"}, /* ISO 8859-7 Greek */
	{28597, "iso8859-7"}, /* ISO 8859-7 Greek */
	{28597, "iso_8859-7"},
	{28597, "iso_8859_7"},
	{28598, "iso-8859-8"}, /* ISO 8859-8 Hebrew; Hebrew (ISO-Visual) */
	{28598, "iso8859-8"}, /* ISO 8859-8 Hebrew; Hebrew (ISO-Visual) */
	{28598, "iso_8859-8"},
	{28598, "iso_8859_8"},
	{28599, "iso-8859-9"}, /* ISO 8859-9 Turkish */
	{28599, "iso8859-9"}, /* ISO 8859-9 Turkish */
	{28599, "iso_8859-9"},
	{28599, "iso_8859_9"},
	{28603, "iso-8859-13"}, /* ISO 8859-13 Estonian */
	{28603, "iso8859-13"}, /* ISO 8859-13 Estonian */
	{28603, "iso_8859-13"},
	{28603, "iso_8859_13"},
	{28605, "iso-8859-15"}, /* ISO 8859-15 Latin 9 */
	{28605, "iso8859-15"}, /* ISO 8859-15 Latin 9 */
	{28605, "iso_8859-15"},
	{28605, "iso_8859_15"},
	{29001, "x-Europa"}, /* Europa 3 */
	{38598, "iso-8859-8-i"}, /* ISO 8859-8 Hebrew; Hebrew (ISO-Logical) */
	{38598, "iso8859-8-i"}, /* ISO 8859-8 Hebrew; Hebrew (ISO-Logical) */
	{38598, "iso_8859-8-i"},
	{38598, "iso_8859_8-i"},
	{50220, "iso-2022-jp"}, /* ISO 2022 Japanese with no halfwidth Katakana; Japanese (JIS) */
	{50221, "csISO2022JP"}, /* ISO 2022 Japanese with halfwidth Katakana; Japanese (JIS-Allow 1 byte Kana) */
	{50222, "iso-2022-jp"}, /* ISO 2022 Japanese JIS X 0201-1989; Japanese (JIS-Allow 1 byte Kana - SO/SI) */
	{50225, "iso-2022-kr"}, /* ISO 2022 Korean */
	{50225, "iso2022-kr"}, /* ISO 2022 Korean */
	{50227, "x-cp50227"}, /* ISO 2022 Simplified Chinese; Chinese Simplified (ISO 2022) */
	/* 50229 		ISO 2022 Traditional Chinese */
	/* 50930 		EBCDIC Japanese (Katakana) Extended */
	/* 50931 		EBCDIC US-Canada and Japanese */
	/* 50933 		EBCDIC Korean Extended and Korean */
	/* 50935 		EBCDIC Simplified Chinese Extended and Simplified Chinese */
	/* 50936 		EBCDIC Simplified Chinese */
	/* 50937 		EBCDIC US-Canada and Traditional Chinese */
	/* 50939 		EBCDIC Japanese (Latin) Extended and Japanese */
	{51932, "euc-jp"}, /* EUC Japanese */
	{51936, "EUC-CN"}, /* EUC Simplified Chinese; Chinese Simplified (EUC) */
	{51949, "euc-kr"}, /* EUC Korean */
	/* 51950 		EUC Traditional Chinese */
	{52936, "hz-gb-2312"}, /* HZ-GB2312 Simplified Chinese; Chinese Simplified (HZ) */
	{54936, "GB18030"}, /* Windows XP and later: GB18030 Simplified Chinese (4 byte); Chinese Simplified (GB18030) */
	{57002, "x-iscii-de"}, /* ISCII Devanagari */
	{57003, "x-iscii-be"}, /* ISCII Bengali */
	{57004, "x-iscii-ta"}, /* ISCII Tamil */
	{57005, "x-iscii-te"}, /* ISCII Telugu */
	{57006, "x-iscii-as"}, /* ISCII Assamese */
	{57007, "x-iscii-or"}, /* ISCII Oriya */
	{57008, "x-iscii-ka"}, /* ISCII Kannada */
	{57009, "x-iscii-ma"}, /* ISCII Malayalam */
	{57010, "x-iscii-gu"}, /* ISCII Gujarati */
	{57011, "x-iscii-pa"}, /* ISCII Punjabi */

	{0, NULL}
};

static REBINT get_codepage_id(REBVAL *cp)
{
	int i;
	REBYTE *name;

	if (IS_INTEGER(cp))
		return VAL_INT32(cp);
	if (IS_WORD(cp)) {
		name = VAL_WORD_NAME(cp);
	} else if (IS_STRING(cp) || IS_TAG(cp)) {
		name = VAL_BIN_DATA(cp);
	} else
		return -1;

	//printf("get_codepage_id from: %s\n", name);
#ifdef TO_WINDOWS
	if (_strnicmp(cs_cast(name), "cp", 2) == 0)
		return atoi(cs_cast(name + 2)); /* CP123 */
	else if ('0' <= name[0] && name[0] <= '9')
		return atoi(cs_cast(name));     /* 123 */

	for (i = 0; codepage_alias[i].name != NULL; ++i)
		if (_stricmp(cs_cast(name), codepage_alias[i].name) == 0)
			return codepage_alias[i].codepage;
#else
	if (strncasecmp(cs_cast(name), "cp", 2) == 0)
		return atoi(cs_cast(name) + 2); /* CP123 */
	else if ('0' <= name[0] && name[0] <= '9')
		return atoi(cs_cast(name));     /* 123 */

	for (i = 0; codepage_alias[i].name != NULL; ++i)
		if (strcasecmp(cs_cast(name), codepage_alias[i].name) == 0)
			return codepage_alias[i].codepage;
#endif
	return -1;
}

static REBYTE* get_codepage_name(REBVAL *cp)
{
	REBINT id, i;

	if (IS_WORD(cp)) {
		return VAL_WORD_NAME(cp);
	} else if (IS_STRING(cp) || IS_TAG(cp)) {
		return VAL_BIN_DATA(cp);
	} else if (IS_INTEGER(cp)) {
		id = VAL_INT32(cp);
	} else
		return NULL;

	//printf("get_codepage_name from: %d\n", id);

	for (i = 0; codepage_alias[i].name != NULL; ++i) {
		if (codepage_alias[i].codepage == id) {
			// printf("found: %s\n", codepage_alias[i].name);
			return (REBYTE*)codepage_alias[i].name;
		}
	}
	return NULL;
}

#ifdef TO_WINDOWS
/***********************************************************************
**
*/	static void Swap_Endianess_U16(u16 *data, REBCNT len)
/*
***********************************************************************/
{
	//printf("Swap_Endianess_U16 len: %d\n", len);
	REBCNT i;
	for (i = 0; i < len; ++i) {
		u16 num = data[i];
		data[i] = (num>>8) | (num<<8);
	}
}
#endif

/***********************************************************************
**
*/	REBNATIVE(iconv)
/*
//	iconv: native [
//		{Convert binary to text using specified codepage, or transcode to a new binary}
//		data     [binary!] 
//		codepage [word! integer! tag! string!] {Source codepage id}
//		/to                                    {Transcode to a new binary}
//		target   [word! integer! tag! string!] {Target codepage id}
//	]
***********************************************************************/
{
	REBVAL *data     = D_ARG(1) ;
	REBVAL *val_from = D_ARG(2);
	REBOOL  ref_to   = D_REF(3) ;
	REBVAL *val_to   = D_ARG(4);

	REBCNT  src_len = VAL_LEN(data);
	REBSER *dst_wide;
	

#ifdef TO_WINDOWS
    REBINT  dst_len = 0;
    REBSER *dest;
	REBINT cp, tp;
	  BOOL default_char_used;

	cp = get_codepage_id(val_from);
	if (cp < 0) {
		Trap1(RE_INVALID_ARG, val_from);
		return R_NONE;
	}

	REBYTE *bp;

	if (cp == 1200 || cp == 1201) { // data are already wide (UTF-16LE or UTF-16BE)
		bp = VAL_BIN_AT(data);
		dst_len = src_len / 2;
		dst_wide = Make_Series(dst_len + 1, 2, FALSE);
		memcpy(BIN_HEAD(dst_wide), bp, src_len);
		dst_wide->tail = dst_len;
		TERM_SERIES(dst_wide);
		if (ref_to) {
			goto convert_to;
		}
		if (cp == 1201) {
			Swap_Endianess_U16((u16*)BIN_HEAD(dst_wide), dst_wide->tail);
		}
		SET_STRING(D_RET, dst_wide);
		return R_RET;
	}
	else if (cp == 12000 || cp == 12001) {  // data are UTF-32LE or UTF-32BE
		// this codepage is not supported by `MultiByteToWideChar`
		dst_wide = Decode_UTF_String(VAL_BIN_AT(data), src_len, cp == 12000 ? -32 : 32, FALSE, TRUE);
		dst_len = SERIES_TAIL(dst_wide);
		if (ref_to) {
			goto convert_to;
		}
		SET_STRING(D_RET, dst_wide);
		return R_RET;
	}
	
	if (VAL_LEN(data) > 0) {
		dst_len = MultiByteToWideChar(cp, 0, cs_cast(VAL_BIN_DATA(data)), src_len, NULL, 0);
		if (dst_len <= 0) return R_NONE; //@@ or error?

		dst_wide = Make_Series(dst_len, 2, FALSE);
		dst_len = MultiByteToWideChar(cp, 0, cs_cast(VAL_BIN_DATA(data)), src_len, (REBCHR*)BIN_HEAD(dst_wide), dst_len);
		dst_wide->tail = dst_len;
	} else {
		dst_wide = Make_Series(1, 2, FALSE);
		dst_wide->tail = 0;
	}
	
	if (ref_to) {
	convert_to:
		tp = get_codepage_id(val_to);
		if (tp < 0) {
			Trap1(RE_INVALID_ARG, val_to);
			return R_NONE;
		}

		if (tp == 1200) { // UTF-16LE
			if (cp == 1200) {
				// we already have data in this format.
				dest = dst_wide;
			} else {
				dest = Copy_Series(dst_wide);
			}
		} else if (tp == 1201) { // UTF-16BE
			if (cp == 1201) {
				// we already have data in this format.
				dest = dst_wide;
			} else {
				dest = Copy_Series(dst_wide);
			}
		} else {
			if (cp == 1201) {
				Swap_Endianess_U16((u16*)BIN_HEAD(dst_wide), dst_wide->tail);
			}
			if (dst_wide->tail > 0) {
				dst_len = WideCharToMultiByte(tp, 0, (REBCHR*)BIN_HEAD(dst_wide), dst_wide->tail, NULL, 0, 0, &default_char_used);
				if (dst_len <= 0) return R_NONE; //@@ or error?

				dest = Make_Series(dst_len, 1, FALSE);
				dst_len = WideCharToMultiByte(tp, 0, (REBCHR*)BIN_HEAD(dst_wide), dst_wide->tail, s_cast(BIN_HEAD(dest)), dst_len, 0, &default_char_used);
				dest->tail = dst_len;
			}
			else {
				dest = Make_Series(1, 1, FALSE);
				dest->tail = 0;
			}
		}
		if(SERIES_WIDE(dest) == 2) {
			// in case source was already UTF-16
			// change series width to 1 (for binary result)
			dest->info &=  0xFFFFFF00;
			dest->info |=  0x00000001;
			dest->tail *= 2;
		}
		if (
			dest->tail > 0 &&
			((tp == 1201 && cp != 1201) ||  // result is UTF-16BE or
			(tp == 1200 && cp == 1201))     // conversion from UTF-16BE to UTF-16LE
		) {
			Swap_Endianess_U16((u16*)BIN_HEAD(dest), dest->tail / 2);
		}
		SET_BINARY(D_RET, dest);
	} else {
		SET_STRING(D_RET, dst_wide);
	}
	
	return R_RET;
#else
	//FIXME: does all supported non Windows OSes have iconv?
	iconv_t cd;
	size_t src_size = (size_t)src_len;
	size_t dst_size;
	size_t nread;
	REBCNT tail;
	const char *fromcode = cs_cast(get_codepage_name(val_from));
	const char *tocode;
	int wide;

	if (!fromcode) Trap1(RE_INVALID_ARG, val_from);
	if (ref_to) {
		tocode = cs_cast(get_codepage_name(val_to));
		if (!tocode) Trap1(RE_INVALID_ARG, val_to);
		wide = 1; // result is raw binary series
	} else {
		tocode = "UTF-16LE";
		wide = 2; // result is string
	}
	//printf("iconv_open %s %s\n", tocode, fromcode);
	cd = iconv_open(tocode, fromcode);
	if (cd == (iconv_t)-1) {
		if (get_codepage_id(val_from) < 0) Trap1(RE_INVALID_ARG, val_from);
		if (get_codepage_id(val_to) < 0) Trap1(RE_INVALID_ARG, val_to);
		Trap0(RE_FEATURE_NA);
	}
		
	dst_wide = Make_Series(src_len + 1, wide, FALSE);
	dst_size = SERIES_SPACE(dst_wide);

	
	char *src = (char*)VAL_BIN_DATA(data);
	char *dst = (char*)BIN_HEAD(dst_wide);
	//Reb_Opts->watch_expand = TRUE;
	for(;;) {
		//printf("iconv from: %s to: %s wide: %d src_size: %d dst_size: %d\n", fromcode, tocode, wide, src_size, dst_size);
		//Dump_Series(dst_wide, "dst");
		nread = iconv(cd, &src, &src_size, &dst, &dst_size);
		//printf("ret: %d src_size: %d dst_size: %d %d\n", nread, src_size, dst_size, (REBYTE *)dst - BIN_HEAD(dst_wide));
		if(nread == (size_t)-1) {
			//printf("iconv failed: %d\n", errno );
			if(errno == E2BIG) {
				// There is not sufficient room at destination
				if (SERIES_SPACE(dst_wide) < (4 * src_len)) {
					// expand the destination series; maximum 4x of the source
					tail = (REBCNT)((REBYTE*)dst - BIN_HEAD(dst_wide)) / wide;
					SERIES_TAIL(dst_wide) = tail;
					//printf("\n---expand delta: %d\n", src_len);
					Expand_Series(dst_wide, tail, src_len / 2);
					// update destination pointers for continuation
					dst = (char*)BIN_SKIP(dst_wide, wide * tail);
					dst_size = SERIES_SPACE(dst_wide) - wide * tail;
					continue;
				}
				//printf("errno == E2BIG\n");
			}
			if(errno == EILSEQ) {
				// An invalid multibyte sequence
				//printf("errno == EILSEQ\n");
			}
			else if(errno == EINVAL) {
				// An incomplete multibyte sequence
				//printf("errno == EINVAL\n");
			}
			Free_Series(dst_wide);
			iconv_close(cd);
			Trap1(RE_INVALID_DATA, D_ARG(1));
			return R_NONE;
		}
		if (src_size == 0) break;
	}
	SERIES_TAIL(dst_wide) = (REBCNT)((REBYTE*)dst - BIN_HEAD(dst_wide)) / wide;
	if (ref_to) {
		SET_BINARY(D_RET, dst_wide);
	}
	else {
		SET_STRING(D_RET, dst_wide);
	}
	
result:
	//Reb_Opts->watch_expand = FALSE;
	iconv_close(cd);
	return R_RET;
#endif

}
