/****************************************************************************************
 *   FileName    : nls720.c
 *   Description : nls720.c
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

#include <stdlib.h>
#include <stdint.h>
#include "nls720.h"


static int32_t char2uni_none(const uint8_t *ansi, wchar_t *uni);

#if defined(UNI2ANSI_INCLUDE)
static int32_t uni2char_none(wchar_t uni, uint8_t *ansi);
#endif


//#pragma	memory=data_seg(TCC730_INTERNAL)
int32_t NLS_nCurCodePage = 0;
int32_t (*char2uni)(uint8_t *ansi, wchar_t *uni);

#if defined(UNI2ANSI_INCLUDE)
int32_t (*Uni2Ansi)(wchar_t uni, uint8_t *ansi);
#endif
//#pragma	memory=default



/**************************************************************************
*  FUNCTION NAME: InitNLS
*
*  DESCRIPTION: NONE
*
*  INPUT: NONE
*
*  OUTPUT: void - Return Type
*
*  REMARK: NONE
*
**************************************************************************/
void InitNLS(void) // Modified by jim on V3.48 [05/08/23]
{
	char2uni = (void *)char2uni_none;
#if defined(UNI2ANSI_INCLUDE)
	Uni2Ansi = uni2char_none;
#endif
#if defined(LANG_KOREAN_INCLUDE) || defined(LANG_KOREAN_EXT_INCLUDE)
	NLS_nCurCodePage = CP_KOREAN;
#elif defined(LANG_CHINESE_INCLUDE) || defined(LANG_CHINESE_SIMP_INCLUDE)
	NLS_nCurCodePage = CP_CHINESE_SIMP;
#elif defined(LANG_CHINESE_TRAD_INCLUDE)
	NLS_nCurCodePage = CP_CHINESE_TRAD;
#elif defined(LANG_JAPANESE_INCLUDE)
	NLS_nCurCodePage = CP_JAPANESE;
#elif defined(LANG_ENGLISH_INCLUDE)
	NLS_nCurCodePage = CP_ENGLISH;
#elif defined(LANG_MULTILINGUAL_INCLUDE)
	NLS_nCurCodePage = CP_MULTILINGUAL;
#elif defined(LANG_RUSSIAN_INCLUDE)
	NLS_nCurCodePage = CP_RUSSIAN;
#elif defined(LANG_CENTRAL_EUROPE_INCLUDE)
	NLS_nCurCodePage = CP_CENTRAL_EUROPE;
#elif defined(LANG_LATIN_I_INCLUDE)
	NLS_nCurCodePage = CP_LATIN_I;
#else
	NLS_nCurCodePage = CP_KOREAN;
#endif
#if 0
	if ( UNI_Init(part_id,Dir_num) < 0) // Modified by jim on V3.48 [05/08/23]
	{
		UNI_FontExistFlag = 0;
		NLS_nCurCodePage = 0;
	}
	else
#endif
	{
		(void) SetCodePage(NLS_nCurCodePage);
	}
}


/**************************************************************************
*  FUNCTION NAME: SetCodePage
*
*  DESCRIPTION: NONE
*
*  INPUT: codepage = int32_t
*
*  OUTPUT: int32_t - Return Type
*
*  REMARK: NONE
*
**************************************************************************/
int32_t SetCodePage(int32_t codepage)
{
	int32_t new_codepage;
	int32_t (*new_char2uni)(uint8_t *ansi, wchar_t *uni);
#if defined(UNI2ANSI_INCLUDE)
	int32_t (*new_uni2char)(wchar_t uni, uint8_t *ansi);
#endif

	switch (codepage)
	{
#if defined(LANG_KOREAN_INCLUDE) || defined(LANG_KOREAN_EXT_INCLUDE)
		case CP_949 :
			new_codepage = CP_949;
			new_char2uni = (void *)char2uni_cp949;
#if defined(UNI2ANSI_INCLUDE)
			new_uni2char = uni2char_cp949;
#endif
			break;
#endif
#if defined(LANG_CHINESE_INCLUDE) || defined(LANG_CHINESE_SIMP_INCLUDE)
		case CP_936 :
			new_codepage = CP_936;
			new_char2uni = char2uni_cp936;
#if defined(UNI2ANSI_INCLUDE)
			new_uni2char = uni2char_cp936;
#endif
			break;
#endif
#if defined(LANG_CHINESE_TRAD_INCLUDE)
		case CP_950 :
			new_codepage = CP_950;
			new_char2uni = char2uni_cp950;
#if defined(UNI2ANSI_INCLUDE)
			new_uni2char = uni2char_cp950;
#endif
			break;
#endif
#if defined(LANG_JAPANESE_INCLUDE)
		case CP_932 :
			new_codepage = CP_932;
			new_char2uni = char2uni_cp932;
#if defined(UNI2ANSI_INCLUDE)
			new_uni2char = uni2char_cp932;
#endif
			break;
#endif
#if defined(LANG_ENGLISH_INCLUDE)
		case CP_437 :
			new_codepage = CP_437;
			new_char2uni = char2uni_cp437;
#if defined(UNI2ANSI_INCLUDE)
			new_uni2char = uni2char_cp437;
#endif
			break;
#endif
#if defined(LANG_MULTILINGUAL_INCLUDE)
		case CP_850 :
			new_codepage = CP_850;
			new_char2uni = char2uni_cp850;
#if defined(UNI2ANSI_INCLUDE)
			new_uni2char = uni2char_cp850;
#endif
			break;
#endif
#if defined(LANG_CENTRAL_EUROPE_INCLUDE)
		case CP_1250 :
			new_codepage = CP_1250;
			new_char2uni = char2uni_cp1250;
#if defined(UNI2ANSI_INCLUDE)
			new_uni2char = uni2char_cp1250;
#endif
			break;
#endif
#if defined(LANG_RUSSIAN_INCLUDE)
		case CP_1251 :
			new_codepage = CP_1251;
			new_char2uni = char2uni_cp1251;
#if defined(UNI2ANSI_INCLUDE)
			new_uni2char = uni2char_cp1251;
#endif
			break;
#endif
#if defined(LANG_LATIN_I_INCLUDE)
		case CP_1252 :
			new_codepage = CP_1252;
			new_char2uni = char2uni_cp1252;
#if defined(UNI2ANSI_INCLUDE)
			new_uni2char = uni2char_cp1252;
#endif
			break;
#endif
		default :
			new_codepage = 0;
			new_char2uni = NULL;
#if defined(UNI2ANSI_INCLUDE)
			new_uni2char = 0;
			break;
#endif
	}

	if (new_codepage != 0)
	{
		NLS_nCurCodePage = new_codepage;
		char2uni = new_char2uni;
#if defined(UNI2ANSI_INCLUDE)
		Uni2Ansi = new_uni2char;
#endif
	}
#if 0
	if ( (UNI_IsFontExist() == 1) && (codepage != 0) )
	{
		UNI_FontExistFlag = 1;
	}
	else
	{
		UNI_FontExistFlag = 0;
		NLS_nCurCodePage = 0;
	}
#endif
	return NLS_nCurCodePage;
}

/**************************************************************************
*  FUNCTION NAME: static int32_t char2uni_none(uint8_t *ansi, wchar_t *uni);
*
*  DESCRIPTION : char2uni_none
*  INPUT: 	ansi	= uint8_t
*		uni	= wchar_t
*
*  OUTPUT:int32_t - Return Type
*  
*  REMARK: NONE
**************************************************************************/
static int32_t char2uni_none(const uint8_t *ansi, wchar_t *uni)
{
	if (*ansi < (uint8_t)0x80)
	{
		*uni = (wchar_t)*ansi;
	}
	else
	{
		*uni = (wchar_t)UNKNOWN_UNICODE;
	}

	return 1;
}


#if defined(UNI2ANSI_INCLUDE)
/**************************************************************************
*  FUNCTION NAME: uni2char_none
*
*  DESCRIPTION	: uni2char_none
*
*  INPUT: 	ansi = wchar_t
*  		uni = uint8_t
*
*  OUTPUT: int32_t - Return Type
*
*  REMARK: NONE
*
**************************************************************************/
static int32_t uni2char_none(wchar_t uni, uint8_t *ansi)
{
	if ((uni & 0xff00) == 0 && (uni & 0xff) < 0x80)
	{
		*ansi = (uint8_t)(uni & 0xff);
	}
	else
	{
		*ansi = UNKNOWN_ANSICODE;
	}

	return 1;
}
#endif

/* end of file */

