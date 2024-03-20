/****************************************************************************************
 *   FileName    : ID3.c
 *   Description : ID3.c
 ****************************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved 
 
This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited 
to re-distribution in source or binary form is strictly prohibited.
This source code is provided ¡°AS IS¡± and nothing contained in this source code 
shall constitute any express or implied warranty of any kind, including without limitation, 
any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent, 
copyright or other third party intellectual property right. 
No warranty is made, express or implied, regarding the information¡¯s accuracy, 
completeness, or performance. 
In no event shall Telechips be liable for any claim, damages or other liability arising from, 
out of or in connection with this source code or the use in the source code. 
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement 
between Telechips and Company.
*
****************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "nls720.h"

#ifdef __cplusplus
extern "C" {
#endif


/**************************************************************************
*  FUNCTION NAME: UNI_MappingFontOneUnicode
*
*  DESCRIPTION: NONE
*
*  INPUT: 		onechar = uint8_t
*  			WideChar = void *
*
*  OUTPUT			: int32_t - Return Type
*
*  REMARK			: NONE
*
**************************************************************************/
static int32_t UNI_MappingFontOneUnicode(uint8_t *onechar,void *WideChar)
{
	int32_t retval;
	uint16_t lTempCode;
	wchar_t *fontmp = (wchar_t *)WideChar;

	if (*onechar < (uint8_t)0x80)
	{
		retval = 1;
	}
	else
	{
		if (char2uni != NULL)
		{
			retval = char2uni(onechar, fontmp);
		}
		else
		{
			lTempCode = (uint16_t)(onechar[0] | ((uint16_t)onechar[1] << 8));	/* QAC : 12.8 */
			*fontmp = (wchar_t)lTempCode;
			retval = 2;
		}
	}

	return retval;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Function    : UNI_ConvertStringToUnicode
//
// Description : Conver National character string into Unicode
//
///////////////////////////////////////////////////////////////////////////////////
int32_t UNI_ConvertStringToUnicode(const uint8_t *pString,uint8_t *pUnicodeBuf, uint32_t iMaxStringSize)
{
	int32_t		ret = 0;
	uint32_t 	i, j;
	uint8_t 	tempchar[2];
	uint8_t 	ucWideChar[2];
	uint32_t	iWide;
	j = 0;
	for (i = 0; i < iMaxStringSize; i += iWide)
	{
		if (pString[i] == (uint8_t)0 )
		{
			break;
		}

		tempchar[0] = (uint8_t)pString[i];
		tempchar[1] = (uint8_t)pString[i+(uint32_t)1];
		iWide = (uint32_t)UNI_MappingFontOneUnicode(tempchar,ucWideChar);

		switch (iWide)
		{
			case 2: // Wide Character
				break;
			case 1:	// Not Wide Character

				if (tempchar[0] < (uint8_t)0x80)
				{
					ucWideChar[0] = tempchar[0];
					ucWideChar[1] = 0x00;
				}
				else
				{
					ucWideChar[1] =  0x00;
				}

				break;
			default:
				ret = -2;
				break;
		}

		if (ret < 0)
		{
			break;
		}
		else
		{
			pUnicodeBuf[j] =  ucWideChar[0];
			pUnicodeBuf[j+(uint32_t)1] =  ucWideChar[1];
			j+=(uint32_t)2;
		}
	}

	if (ret == 0)
	{
		pUnicodeBuf[j] =  0;
		pUnicodeBuf[j+(uint32_t)1] =  0;
		ret = (int32_t)j;
	}
	return ret;	// SUCCESS
}

/* Utility code - temperary */
int32_t Unicode2UTF8(const uint8_t *pUnicode,uint8_t *pUTF8, int32_t Unicodelen ,int32_t maxlen)
{
           uint16_t unicode;
           int32_t len=0;

           do {
                     if (len>=maxlen) {
                                break;
                     }
                     unicode=(uint16_t)pUnicode[0]+((uint16_t)pUnicode[1]*(uint16_t)256);
                     if (unicode<(uint16_t)0x80) {
                                *pUTF8++=(uint8_t)unicode;
                                if (++len>=maxlen) {
                                          break;
                                }
                     } else if (unicode<(uint16_t)0x0800) {
                                *pUTF8++=(uint8_t)(0xc0 | (((uint8_t)(unicode>>6))&0x1F));
                                if (++len>=maxlen) {
                                          break;
                                }
                                *pUTF8++=(uint8_t)(0x80 | ((uint8_t)(unicode&0x003F)));
                                if (++len>=maxlen) {
                                          break;
                                }
                     } else {
                                *pUTF8++=(uint8_t)(0xe0 | (((uint8_t)(unicode>>12))&0x0F));
                                if (++len>=maxlen) {
                                          break;
                                }
                                *pUTF8++=(uint8_t)(0x80 | (((uint8_t)(unicode>>6))&0x3F));
                                if (++len>=maxlen) {
                                          break;
                                }
                                *pUTF8++=(uint8_t)(0x80 | ((uint8_t)(unicode&0x003F)));
                                if (++len>=maxlen) {
                                          break;
                                }
                     }
                    pUnicode+=2;
 		      Unicodelen-=2;
           } while ((unicode!=(uint16_t)0)&&(Unicodelen > 0));

           if(unicode != (uint16_t)0)
           {
		*pUTF8++=(uint8_t)0x00;
           }

           return len;
}


#ifdef __cplusplus
}
#endif
