/***************************************************************************************
 *   FileName    : nls720.h
 *   Description : nls720.h
 ****************************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited 
to re-distribution in source or binary form is strictly prohibited.
This source code is provided ��AS IS�� and nothing contained in this source code 
shall constitute any express or implied warranty of any kind, including without limitation, 
any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, 
copyright or other third party intellectual property right. 
No warranty is made, express or implied, regarding the information��s accuracy, 
completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, 
out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement 
between Telechips and Company.
*
****************************************************************************************/


#ifndef	NLS720_H__
#define	NLS720_H__

#define LANG_KOREAN_INCLUDE
#define LANG_KOREAN_EXT_INCLUDE

#ifndef NULL
#define NULL 0
#endif

#if defined	(LANG_KOREAN_INCLUDE)		// Korean KSC 5601
#define	NLS_INCLUDE		1

#elif defined	(LANG_KOREAN_EXT_INCLUDE)	// Korean KSC 5601 and Extended Korean code
#define NLS_INCLUDE		1

#elif defined	(LANG_CHINESE_INCLUDE)		// Chinese Simplified same as LANG_CHINESE_SIMP_INCLUDE
#define NLS_INCLUDE		1

#elif defined	(LANG_CHINESE_SIMP_INCLUDE)	// Chinese Simplified
#define NLS_INCLUDE		1

#elif defined	(LANG_CHINESE_TRAD_INCLUDE)	// Chinese Traditional
#define	NLS_INCLUDE		1

#elif defined	(LANG_JAPANESE_INCLUDE)		// Japanese JIS X208
#define	NLS_INCLUDE		1

#elif defined	(LANG_ENGLISH_INCLUDE)		// English
#define	NLS_INCLUDE		1

#elif defined (LANG_MULTILINGUAL_INCLUDE)	// Multilingual Latin I
#define	NLS_INCLUDE		1

#elif defined	(LANG_RUSSIAN_INCLUDE)		// Cyrillic (CP_1251)
#define	NLS_INCLUDE		1

#elif defined	(LANG_CENTRAL_EUROPE_INCLUDE)	// Central Europe (CP_1250)
#define	NLS_INCLUDE		1

#elif defined	(LANG_LATIN_I_INCLUDE)		// Latin I (CP_1252)
#define	NLS_INCLUDE		1

#endif



#define CP_437      437     // MS-DOS United States 
#define CP_850      850     // MS-DOS Multilingual (Latin I) 
#define CP_932      932		// Japanese
#define CP_936      936     // Chinese (PRC, Singapore) 
#define CP_949      949     // Korean 
#define CP_950      950     // Chinese (Taiwan Region; Hong Kong SAR, PRC)  
#define CP_1250     1250    // Central Europe
#define CP_1251     1251    // Cyrillic
#define CP_1252     1252    // Latin I


// Locale code
#define CP_ENGLISH          CP_437
#define CP_MULTILINGUAL		CP_850
#define CP_JAPANESE         CP_932
#define CP_CHINESE          CP_936
#define CP_KOREAN           CP_949

// Chinese Simplified and Traditional
#define CP_CHINESE_SIMP     CP_936
#define CP_CHINESE_TRAD     CP_950

#define CP_CENTRAL_EUROPE   CP_1250
#define CP_RUSSIAN          CP_1251
#define CP_LATIN_I			CP_1252


///////////////////////////////////////////////////////////////////////////////

#define UNKNOWN_ANSICODE    '?'
#define UNKNOWN_UNICODE     0xff1f			// full with '?'

///////////////////////////////////////////////////////////////////////////////

#if 0//ndef wchar_t
typedef unsigned short int32_t wchar_t;
#endif


extern int32_t NLS_nCurCodePage;
extern int32_t (*char2uni)(uint8_t *ansi, wchar_t *uni);

#if defined(UNI2ANSI_INCLUDE)
extern int32_t (*Uni2Ansi)(wchar_t uni, uint8_t *ansi);
#endif

#if defined(__cplusplus)
extern "C"
{
#endif


	enum{
		LANG_NONE = 0,
		LANG_KOREAN,
		LANG_ENGLISH,
		LANG_MULTILANGUAL,
		LANG_JAPANESE,
		LANG_CHINESE_SIMP,
		LANG_CHINESE_TRAD,
		LANG_CENTRAL_EUROPE,
		LANG_RUSSIAN,
		LANG_LATIN_I,
		SAVE_FONT_FILE
	};



	extern void InitNLS(void); // Modified by jim on V3.48 [05/08/23]
	int32_t SetCodePage(int32_t codepage);
	int32_t GetCodePage(void);

// MS-DOS United States
#if 0
	int32_t char2uni_cp437(uint8_t *ansi, wchar_t *uni);
	int32_t char2uni_cp850(uint8_t *ansi, wchar_t *uni);
	int32_t char2uni_cp932(uint8_t *ansi, wchar_t *uni);
	int32_t char2uni_cp936(uint8_t *ansi, wchar_t *uni);
#endif	
	int32_t char2uni_cp949(const uint8_t *ansi, wchar_t *uni);
#if 0	
	int32_t char2uni_cp950(uint8_t *ansi, wchar_t *uni);
	int32_t char2uni_cp1250(uint8_t *ansi, wchar_t *uni);
	int32_t char2uni_cp1251(uint8_t *ansi, wchar_t *uni);
	int32_t char2uni_cp1252(uint8_t *ansi, wchar_t *uni);
#endif

#if defined(UNI2ANSI_INCLUDE)
	int32_t uni2char_cp437(wchar_t uni, uint8_t *ansi);
	int32_t uni2char_cp850(wchar_t uni, uint8_t *ansi);
	int32_t uni2char_cp932(wchar_t uni, uint8_t *ansi);
	int32_t uni2char_cp936(wchar_t uni, uint8_t *ansi);
	int32_t uni2char_cp949(wchar_t uni, uint8_t *ansi);
	int32_t uni2char_cp950(wchar_t uni, uint8_t *ansi);
	int32_t uni2char_cp1250(wchar_t uni, uint8_t *ansi);
	int32_t uni2char_cp1251(wchar_t uni, uint8_t *ansi);
	int32_t uni2char_cp1252(wchar_t uni, uint8_t *ansi);
#endif

#if defined(__cplusplus)
}
#endif

#endif

/* end of file */

