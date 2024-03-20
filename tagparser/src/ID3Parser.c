/****************************************************************************************
 *   FileName    : ID3Parser.c
 *   Description : ID3Parser.c
 ****************************************************************************************
 *
 *   TCC Version 1.0
 *   Copyright (c) Telechips Inc.
 *   All rights reserved

This source code contains confidential information of Telechips.
Any unauthorized use without a written permission of Telechips including not limited
to re-distribution in source or binary form is strictly prohibited.
This source code is provided “AS IS” and nothing contained in this source code
shall constitute any express or implied warranty of any kind, including without limitation,
any warranty of merchantability, fitness for a particular purpose or non-infringement of any patent,
copyright or other third party intellectual property right.
No warranty is made, express or implied, regarding the information’s accuracy,
completeness, or performance.
In no event shall Telechips be liable for any claim, damages or other liability arising from,
out of or in connection with this source code or the use in the source code.
This source code is provided subject to the terms of a Mutual Non-Disclosure Agreement
between Telechips and Company.
*
****************************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "TCTagParser.h"
#include "ID3Parser.h"
#include "nls720.h"
#include "nls_app.h"

static const char* ID3_GenreTable[ID3_GENRE_MAX_CNT + 1] =
{
	"Blues",				"Classic Rock",		"Country",			"Dance",				"Disco",				"Funk",
	"Grunge",			"Hip-Hop",			"Jazz",				"Metal",				"New Age",			"Oldies",
	"Other",				"Pop",				"R&B",				"Rap",				"Reggae",			"Rock",
	"Techno",				"Industrial",			"Alternative",			"Ska",				"Death Metal",			"Pranks",
	"Soundtrack",			"Euro-Techno",		"Ambient",			"Trip-Hop",			"Vocal",				"Jazz+Funk",
	"Fusion",				"Trance",				"Classical",			"Instrumental",		"Acid",				"House",
	"Game",				"Sound Clip",			"Gospel",				"Noise",				"Alt. Rock",			"Bass",
	"Soul",				"Punk",				"Space",				"Meditative",			"Instrum. Pop",		"Instrum. Rock",
	"Ethnic",				"Gothic",				"Darkwave",			"Techno-Indust.",		"Electronic",			"Pop-Folk",
	"Eurodance",			"Dream",				"Southern Rock",		"Comedy",			"Cult",				"Gangsta",
	"Top 40",				"Christian Rap",		"Pop/Funk",			"Jungle",				"Native American",		"Cabaret",
	"New Wave",			"Psychadelic",			"Rave",				"Showtunes",			"Trailer",				"Lo-Fi",
	"Tribal",				"Acid Punk",			"Acid Jazz",			"Polka",				"Retro",				"Musical",
	"Rock & Roll",			"Hard Rock",			"Folk",				"Folk/Rock",			"National Folk",		"Swing",
	"Fusion",				"Bebob",				"Latin",				"Revival",			"Celtic",				"Bluegrass",
	"Avantgarde",			"Gothic Rock",			"Progress. Rock",		"Psychadel. Rock",		"Symphonic Rock",		"Slow Rock",
	"Big Band",			"Chorus",				"Easy Listening",		"Acoustic",			"Humour",			"Speech",
	"Chanson",			"Opera",				"Chamber Music",		"Sonata",				"Symphony",			"Booty Bass",
	"Primus",				"Porn Groove",		"Satire",
//extended
	"Slow Jam",			"Club",				"Tango",				"Samba",				"Folklore",			"Ballad",
	"Power Ballad",		"Rhythmic Soul",		"Freestyle",			"Duet",				"Punk Rock",			"Drum Solo",
	"A Capella",			"Euro-House",			"Dance Hall",			"Goa",				"Drum & Bass",		"Club-House",
	"Hardcore",			"Terror",				"Indie",				"BritPop",				"Negerpunk",			"Polsk Punk",
	"Beat",				"Christian Gangsta Rap","Heavy Metal",		"Black Metal",			"Crossover",			"Contemporary Christian",
	"Christian Rock",
// winamp 1.91 genres
	"Merengue",			"Salsa",				"Thrash Metal",
// winamp 1.92 genres
	"Anime",				"Jpop",				"Synthpop",			NULL
};

static int32_t ID3_GetIntValue(const uint8_t *pValue)
{
	uint32_t uRet;
	uRet = (uint32_t)(((uint32_t)pValue[0] << 21)
	                       |((uint32_t)pValue[1] << 14)
	                       |((uint32_t)pValue[2] << 7)
	                       |((uint32_t)pValue[3]));
	return (int32_t)uRet;
}

static int16_t ID3_GetShortValue(const uint8_t *pValue)
{
	uint32_t uRet;
	uRet = (uint32_t)(((uint32_t)pValue[0] << 8) |((uint32_t)pValue[1]));
	return (int16_t)uRet;
}

static int32_t ID3_Get3ByteValue(const uint8_t *pValue)
{
	uint32_t uRet;
	uRet = (uint32_t)(((uint32_t)pValue[0] << 16)
	                       |((uint32_t)pValue[1] << 8)
	                       |((uint32_t)pValue[2]));
	return (int32_t)uRet;
}

static int32_t ID3_Get4byteIntValue(const uint8_t *pValue)
{
	uint32_t uRet;
	uRet = (uint32_t)(((uint32_t)pValue[0] << 24)
	                       |((uint32_t)pValue[1] << 16)
	                       |((uint32_t)pValue[2] << 8)
	                       |((uint32_t)pValue[3]));
	return (int32_t)uRet;
}

static void ID3_MakeWideChar2UnicodeUTF8(char *SourceBuf, char *TargetBuf,int32_t iSourceMaxSize)
{
	int32_t iStringSize;
	int32_t i, flag = 1;
	int32_t ulTmp;
	int32_t sMaxSize = iSourceMaxSize;

	uint8_t  tempUTF16buffer[MAX_ID3V2_FIELD];

	if (iSourceMaxSize > 1)
	{
		for ( i = iSourceMaxSize-1 ; i >0; i--)
		{
			if ((SourceBuf[i] != ' ') && (SourceBuf[i] != (char)0x00))
			{
				break;
			}
		}

		if (i == 0)
		{
			flag = 0;
		}
	}
	else
	{
		i = 0;

		if ((iSourceMaxSize == 0) || (SourceBuf[i] == ' ') || (SourceBuf[i] == (char)0x00))
		{
			flag = 0;
		}
	}

	if (flag > 0)
	{
		if (iSourceMaxSize>(i+1))		//jhpark 	 2011.01.26	QAC Rule 21.1
		{
			ulTmp = ( iSourceMaxSize-(i+1));

			(void)memset(&SourceBuf[i+1],0x00,(uint32_t)ulTmp);

			sMaxSize = i+1;
		}

		iStringSize = UNI_ConvertStringToUnicode((const uint8_t *)SourceBuf,tempUTF16buffer,(uint32_t)sMaxSize);
		if (iStringSize>0)
		{
			 iStringSize = Unicode2UTF8((const uint8_t *)tempUTF16buffer, (uint8_t *)&TargetBuf[1], iStringSize,MAX_ID3V2_FIELD);

			TargetBuf[0] = iStringSize + 1;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
// Function    : ID3_GetID3_V1X
//
// Description : Get data in a File System
//
// 1-Update : 2011.08.23, Jhpark
// 		Add parameter - int iDeviceType
// 		if DeviceType is CD, used Cd F/S
///////////////////////////////////////////////////////////////////////////////////
static int32_t ID3_GetID3_V1X_FD(int32_t fd, pstID3V2XInfoType pstID3V2XInfo,uint8_t *tempbuf)
{
	int32_t ret = 0;
	pstID3InfoType		pID3Info;
	int32_t result;

	result =lseek(fd,-MAX_ID3V1_SIZE,SEEK_END);		//jhpark 2011.01.19
	if(result > -1)
	{
		result = read(fd, tempbuf, MAX_ID3V1_SIZE);
		if (result <= 0)
		{
			(void)fprintf(stderr, "%s : read ID3v1 error (%d)\n", __FUNCTION__ , result);
			ret =-1;
		}
	}
	else
	{
		(void)fprintf(stderr, "%s : seek ID3V1 error (%d)\n", __FUNCTION__ , result);
		ret = -1;
	}

	if (ret == 0)
	{
		pID3Info = (pstID3InfoType)tempbuf;

		if (strncmp((const char *)pID3Info->mTAG,"TAG",3) == 0)
		{
			if (pstID3V2XInfo->mMetaData[ID3_TITLE][0] < (uint8_t)2 )
			{
				ID3_MakeWideChar2UnicodeUTF8((char *)pID3Info->mTitle ,(char *)pstID3V2XInfo->mMetaData[ID3_TITLE] ,30);
			}

			if (pstID3V2XInfo->mMetaData[ID3_ARTIST][0] < (uint8_t)2)
			{
				ID3_MakeWideChar2UnicodeUTF8((char *)pID3Info->mArtist,(char *)pstID3V2XInfo->mMetaData[ID3_ARTIST],30);
			}

			if (pstID3V2XInfo->mMetaData[ID3_ALBUM][0] < (uint8_t)2)
			{
				ID3_MakeWideChar2UnicodeUTF8((char *)pID3Info->mAlbum ,(char *)pstID3V2XInfo->mMetaData[ID3_ALBUM] ,30);
			}

			if ((pstID3V2XInfo->mMetaData[ID3_GENRE][0] < (uint8_t)2) && ((int32_t)pID3Info->mGenre < ID3_GENRE_MAX_CNT))
			{
				pstID3V2XInfo->mMetaData[ID3_GENRE][0] =  (uint8_t)strlen(ID3_GenreTable[pID3Info->mGenre]);
				(void)strcpy((char *)&pstID3V2XInfo->mMetaData[ID3_GENRE][1], (const char *)ID3_GenreTable[pID3Info->mGenre] );
			}

			if ((pID3Info->mTrack != (uint8_t)0x00) && (pID3Info->mTrack < (uint8_t)30)&& (pstID3V2XInfo->mTrack == (uint8_t)0x00))
			{
				pstID3V2XInfo->mTrack = pID3Info->mTrack;
			}

			if ((pstID3V2XInfo->mYear[0] < (uint8_t)2) &&(pID3Info->mYear[0] != (uint8_t)0)
				&& (pID3Info->mYear[1] != (uint8_t)0) && (pID3Info->mYear[2] != (uint8_t)0) && (pID3Info->mYear[3] != (uint8_t)0))
			{
				if ((pID3Info->mYear[0] != (uint8_t)0x20) && (pID3Info->mYear[1] != (uint8_t)0x20)
					&& (pID3Info->mYear[2] != (uint8_t)0x20) && (pID3Info->mYear[3] != (uint8_t)0x20))
				{
					(void)memset(pstID3V2XInfo->mYear,0x00,MAX_ID3V2_YEAR);
					pstID3V2XInfo->mYear[0] = 9;
					pstID3V2XInfo->mYear[1] = pID3Info->mYear[0];
					pstID3V2XInfo->mYear[3] = pID3Info->mYear[1];
					pstID3V2XInfo->mYear[5] = pID3Info->mYear[2];
					pstID3V2XInfo->mYear[7] = pID3Info->mYear[3];
				}
			}
			ret = 1;
		}
		else
		{
			//Not find ID3V1
		}
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Function    : ID3_GetID3V2xHeader
//
// Description : Find out ID3 v2.x header
//
///////////////////////////////////////////////////////////////////////////////////
static int32_t ID3_GetID3V2xHeader(uint8_t *tempbuf,int32_t *iFrameOffset,int32_t *iVersion, uint8_t *lFrameHdrFlag)
{
	int32_t ret;
	int32_t iTotalSize;
	pID3V2XHeaderType 	pID3V2XHeader;
	pID3V2XHeader = (pID3V2XHeaderType)tempbuf;

	if (pID3V2XHeader->mVersion > (uint16_t)4) // because big endian
	{
		ret = -1;	//not support
	}
	else
	{
		if((tempbuf == NULL)||(iFrameOffset==NULL)||(iVersion == NULL)|| (lFrameHdrFlag == NULL))
		{
			ret = -((int32_t)ID3_ERROR_BAD_ARGUMENTS);
		}
		else
		{
			*iVersion  = (int32_t)pID3V2XHeader->mVersion;
			*lFrameHdrFlag = pID3V2XHeader->mFlags;
			iTotalSize =  (ID3_GetIntValue(pID3V2XHeader->mSize) + (int32_t)sizeof(ID3V2XHeaderType));

			if ((pID3V2XHeader->mFlags & (uint8_t)ID3_FLAG_FOOTER) != (uint8_t)0)
			{
				iTotalSize += (int32_t)sizeof(ID3V2XFooterType);
			}

			ret = iTotalSize;

			*iFrameOffset = (int32_t)sizeof(ID3V2XHeaderType);

			if ((pID3V2XHeader->mFlags & (uint8_t)ID3_FLAG_EXTHEADER) != (uint8_t)0)
			{
				pID3V2XExtHeaderType pID3V2XExtHeader;
				pID3V2XExtHeader =  (pID3V2XExtHeaderType)&tempbuf[*iFrameOffset];
				*iFrameOffset += (int32_t)sizeof(ID3V2XExtHeaderType);

				// tag extheader info
				if ((pID3V2XExtHeader->mExtFlags & (uint8_t)ID3_FLAG_UPDATE) != (uint8_t)0)
				{
					*iFrameOffset += ID3_GetIntValue(pID3V2XExtHeader->mHeaderSize);
				}
			}
		}
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Function    : ID3_GetFrameInfoV22
//
// Description : this is ID3 TAG down to Ver. 2.2
//
///////////////////////////////////////////////////////////////////////////////////
static int32_t ID3_GetFrameInfoV22(const char *tempbuf)
{
	int32_t iFrameInfo = (int32_t)ID3_NONINFO;

	switch (tempbuf[0])
	{
		case (char)ID3_UPPERCASE_B:
			if (strncmp((const char *)&tempbuf[1], "UF", 2)==0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			break;

		case (char)ID3_UPPERCASE_C:
			switch (tempbuf[1])
			{
				case (char)ID3_UPPERCASE_N:
					if (tempbuf[2] == (char)ID3_UPPERCASE_T)
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_O:
					if (tempbuf[2] == (char)ID3_UPPERCASE_M)
					{
#ifdef ID3_EXT_INCLUDE
						iFrameInfo = (int32_t)ID3_COMMENT;
#else
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
#endif
					}
					break;

				case (char)ID3_UPPERCASE_R:
					if ((tempbuf[2] == (char)ID3_UPPERCASE_A) || (tempbuf[2] == (char)ID3_UPPERCASE_M))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				default:
					break;
			}
			break;

		case (char)ID3_UPPERCASE_E:
			switch (tempbuf[1])
			{
				case (char)ID3_UPPERCASE_T:
					if (tempbuf[2] == (char)ID3_UPPERCASE_C)
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_Q:
					if (tempbuf[2] == (char)ID3_UPPERCASE_U)
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				default:
					break;
			}
			break;

		case (char)ID3_UPPERCASE_G:
			if (strncmp((const char *)&tempbuf[1], "EO", 2)==0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			break;

		case (char)ID3_UPPERCASE_I:
			if (strncmp((const char *)&tempbuf[1], "PL", 2)==0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			break;

		case (char)ID3_UPPERCASE_L:
			if (strncmp((const char *)&tempbuf[1], "NK", 2)==0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			break;

		case (char)ID3_UPPERCASE_M:
			switch (tempbuf[1])
			{
				case (char)ID3_UPPERCASE_C:
					if (tempbuf[2] == (char)ID3_UPPERCASE_I)
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_L:
					if (tempbuf[2] == (char)ID3_UPPERCASE_L)
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				default:
					break;
			}
			break;

		case (char)ID3_UPPERCASE_P:
			switch (tempbuf[1])
			{
				case (char)ID3_UPPERCASE_I:
					if ( tempbuf[2] == (char)ID3_UPPERCASE_C )
					{
#ifdef JPEG_ID3_DISPLAY
// written by CN park
						if (memcmp(&tempbuf[10], "image/jpeg", 10)==0)
						{
							iFrameInfo = (int32_t)ID3_PICTURE;
						}
						else if(memcmp(&tempbuf[10], "image/jpg", 9)==0)
						{
							iFrameInfo = (int32_t)ID3_PICTURE;
						}
						else if(memcmp(&tempbuf[10], "image/JPEG", 10)==0)
						{
							iFrameInfo = (int32_t)ID3_PICTURE;
						}
						else if(memcmp(&tempbuf[10], "image/JPG", 9)==0)
						{
							iFrameInfo = (int32_t)ID3_PICTURE;
						}
						else if(memcmp(&tempbuf[10], "IMAGE/jpeg", 10)==0)
						{
							iFrameInfo = (int32_t)ID3_PICTURE;
						}
						else if (memcmp(&tempbuf[10], "IMAGE/jpg", 9)==0)
						{
							iFrameInfo = (int32_t)ID3_PICTURE;
						}
						else if(memcmp(&tempbuf[10], "IMAGE/JPEG", 10)==0)
						{
							iFrameInfo = (int32_t)ID3_PICTURE;
						}
						else if(memcmp(&tempbuf[10], "IMAGE/JPG", 9)==0)
						{
							iFrameInfo = (int32_t)ID3_PICTURE;
						}
						else
#endif //JPEG_ID3_DISPLAY
						{
							iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
						}
					}
					break;

				case (char)ID3_UPPERCASE_O:
					if (tempbuf[2] == (char)ID3_UPPERCASE_P)
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				default:
					break;
			}
			break;

		case (char)ID3_UPPERCASE_R:
			switch (tempbuf[1])
			{
				case (char)ID3_UPPERCASE_E:
					if (tempbuf[2] == (char)ID3_UPPERCASE_V)
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_V:
					if (tempbuf[2] == (char)ID3_UPPERCASE_A)
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				default:
					break;
			}
			break;

		case (char)ID3_UPPERCASE_S:
			switch (tempbuf[1])
			{
				case (char)ID3_UPPERCASE_L:
					if (tempbuf[2] == (char)ID3_UPPERCASE_T)
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_T:
					if (tempbuf[2] == (char)ID3_UPPERCASE_C)
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				default:
					break;
			}
			break;

		case (char)ID3_UPPERCASE_T:
			switch (tempbuf[1])
			{
				case (char)ID3_UPPERCASE_A:
					if (tempbuf[2] == (char)ID3_UPPERCASE_L)
					{
						iFrameInfo = (int32_t)ID3_ALBUM;
					}
					break;

				case (char)ID3_UPPERCASE_B:
					if (tempbuf[2] == (char)ID3_UPPERCASE_P)
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_C:
					if (tempbuf[2] == (char)ID3_UPPERCASE_O)
					{
						iFrameInfo = (int32_t)ID3_GENRE;
					}
					else if (tempbuf[2] == (char)ID3_UPPERCASE_M)
					{
#ifdef ID3_EXT_INCLUDE
						iFrameInfo = (int32_t)ID3_COMPOSER;
#else
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
#endif
					}
					else if (tempbuf[2] == (char)ID3_UPPERCASE_R)
					{
#ifdef ID3_EXT_INCLUDE
						iFrameInfo = (int32_t)ID3_COPYRIGHT;
#else
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
#endif
					}
					else
					{
						; /* No Statement */
					}
					break;

				case (char)ID3_UPPERCASE_D:
					if ((tempbuf[2] == (char)ID3_UPPERCASE_A) || (tempbuf[2] == (char)ID3_UPPERCASE_Y))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_E:
					if (tempbuf[2] == (char)ID3_UPPERCASE_N)
					{
#ifdef ID3_EXT_INCLUDE
						iFrameInfo = (int32_t)ID3_ENCODEDBY;
#else
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
#endif
					}
					break;

				case (char)ID3_UPPERCASE_F:
					if (tempbuf[2] == (char)ID3_UPPERCASE_T)
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_I:
					if (tempbuf[2] == (char)ID3_UPPERCASE_M)
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_K:
					if (tempbuf[2] == (char)ID3_UPPERCASE_E)
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_L:
					if ((tempbuf[2] == (char)ID3_UPPERCASE_A) || (tempbuf[2] == (char)ID3_UPPERCASE_E))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_M:
					if (tempbuf[2] == (char)ID3_UPPERCASE_T)
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_O:
					if (tempbuf[2] == (char)ID3_UPPERCASE_A)
					{
#ifdef ID3_EXT_INCLUDE
						iFrameInfo = (int32_t)ID3_ORIGARTIST;
#else
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
#endif
					}
					else if((tempbuf[2] == (char)ID3_UPPERCASE_F) || (tempbuf[2] == (char)ID3_UPPERCASE_L)
						|| (tempbuf[2] == (char)ID3_UPPERCASE_R) || (tempbuf[2] == (char)ID3_UPPERCASE_T))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					else
					{
						; /* No Statement */
					}
					break;

				case (char)ID3_UPPERCASE_P:
					if (tempbuf[2]  == (char)ID3_ONE)
					{
						iFrameInfo = (int32_t)ID3_ARTIST;
					}
#ifdef ID3_EXT_INCLUDE
					else if (tempbuf[2] == (char)ID3_TWO)
					{
						iFrameInfo = (int32_t)ID3_ALBUMARTIST;
					}
#else
					else if (tempbuf[2] == (char)ID3_TWO)
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
#endif
					else if ((tempbuf[2] == (char)ID3_THREE) || (tempbuf[2] == (char)ID3_FOUR)
						|| (tempbuf[2] == (char)ID3_UPPERCASE_A) || (tempbuf[2] == (char)ID3_UPPERCASE_B))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					else
					{
						; /* No Statement */
					}
					break;

				case (char)ID3_UPPERCASE_R:
					if (tempbuf[2] == (char)ID3_UPPERCASE_K )
					{
						iFrameInfo = (int32_t)ID3_TRACK;
					}
					else if ((tempbuf[2] == (char)ID3_UPPERCASE_C) || (tempbuf[2] == (char)ID3_UPPERCASE_D))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					else
					{
						; /* No Statement */
					}
					break;

				case (char)ID3_UPPERCASE_S:
					if ((tempbuf[2] == (char)ID3_UPPERCASE_I) || (tempbuf[2] == (char)ID3_UPPERCASE_S))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_T:
					if (tempbuf[2] == (char)ID3_TWO)
					{
						iFrameInfo = (int32_t)ID3_TITLE;
					}
					else if ((tempbuf[2] == (char)ID3_ONE) || (tempbuf[2] == (char)ID3_THREE))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					else
					{
						; /* No Statement */
					}
					break;

				case (char)ID3_UPPERCASE_X:
					if ((tempbuf[2] == (char)ID3_UPPERCASE_T) || (tempbuf[2] == (char)ID3_UPPERCASE_X))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_Y:
					if (tempbuf[2] == (char)ID3_UPPERCASE_E)
					{
						iFrameInfo = (int32_t)ID3_YEAR;
					}
					break;

				default:
					break;
			}
			break;

		case (char)ID3_UPPERCASE_U:
			switch (tempbuf[1])
			{
				case (char)ID3_UPPERCASE_F:
					if (tempbuf[2] == (char)ID3_UPPERCASE_I)
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_L:
					if (tempbuf[2] == (char)ID3_UPPERCASE_T)
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				default:
					break;
			}
			break;

		case (char)ID3_UPPERCASE_W:
			if ((tempbuf[1] >= (char)ID3_ZERO) && (tempbuf[1] <= (char)ID3_UPPERCASE_Z))
			{
				if ((tempbuf[2] >= (char)ID3_ZERO) && (tempbuf[2] <= (char)ID3_UPPERCASE_Z))
				{
#ifdef ID3_EXT_INCLUDE
					iFrameInfo = (int32_t)ID3_URL;
#else
					iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
#endif
				}
			}
			break;

		default:
			break;
	}
	return iFrameInfo;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Function    : ID3_GetFrameInfoV2X
//
// Description : this is ID3 TAG up to Ver. 2.2
//
///////////////////////////////////////////////////////////////////////////////////
static int32_t ID3_GetFrameInfoV2X(const char *tempbuf)
{
	int32_t iFrameInfo = (int32_t)ID3_NONINFO;

	switch (tempbuf[0])
	{
		case (char)ID3_UPPERCASE_A:
			if (strncmp((const char *)&tempbuf[1], "PIC", 3)==0)
			{
#ifdef JPEG_ID3_DISPLAY

// written by CN PARK
				if (strncmp((const char *)&tempbuf[11], "image/jpeg", 10)==0)
				{
					iFrameInfo = (int32_t)ID3_PICTURE;
				}
				else if(strncmp((const char *)&tempbuf[11], "image/jpg", 9)==0)
				{
					iFrameInfo = (int32_t)ID3_PICTURE;
				}
				else if(strncmp((const char *)&tempbuf[11], "image/JPEG", 10)==0)
				{
					iFrameInfo = (int32_t)ID3_PICTURE;
				}
				else if(strncmp((const char *)&tempbuf[11], "image/JPG", 9)==0)
				{
					iFrameInfo = (int32_t)ID3_PICTURE;
				}
				else if(strncmp((const char *)&tempbuf[11], "IMAGE/jpeg", 10)==0)
				{
					iFrameInfo = (int32_t)ID3_PICTURE;
				}
				else if (strncmp((const char *)&tempbuf[11], "IMAGE/jpg", 9)==0)
				{
					iFrameInfo = (int32_t)ID3_PICTURE;
				}
				else if(strncmp((const char *)&tempbuf[11], "IMAGE/JPEG", 10)==0)
				{
					iFrameInfo = (int32_t)ID3_PICTURE;
				}
				else if(strncmp((const char *)&tempbuf[11], "IMAGE/JPG", 9)==0)
				{
					iFrameInfo = (int32_t)ID3_PICTURE;
				}
				else
#endif //JPEG_ID3_DISPLAY
				{
					iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
				}
			}
			else if (strncmp((const char *)&tempbuf[1], "ENC", 3) == 0) /* QAC : 12.4 */
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else if (strncmp((const char *)&tempbuf[1], "SPI", 3) == 0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else
			{
				; /* No Statement */
			}
			break;

		case (char)ID3_UPPERCASE_C:
			if (strncmp((const char *)&tempbuf[1],"OMM",3)==0)
			{
#ifdef ID3_EXT_INCLUDE
				iFrameInfo = (int32_t)ID3_COMMENT;
#else
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
#endif
			}
			else if (strncmp((const char *)&tempbuf[1], "OMR", 3)==0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else
			{
				; /* No Statement */
			}
			break;

		case (char)ID3_UPPERCASE_E:
			if (strncmp((const char *)&tempbuf[1],"NCR",3) == 0) /* QAC : 12.4 */
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else if (strncmp((const char *)&tempbuf[1], "QU2", 3) == 0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else if (strncmp((const char *)&tempbuf[1], "TCO", 3) == 0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else
			{
				; /* No Statement */
			}
			break;

		case (char)ID3_UPPERCASE_G:
			if (strncmp((const char *)&tempbuf[1],"EOB",3) == 0) /* QAC : 12.4 */
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else if (strncmp((const char *)&tempbuf[1], "RID", 3) == 0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else
			{
				; /* No Statement */
			}
			break;

		case (char)ID3_UPPERCASE_L:
			if (strncmp((const char *)&tempbuf[1],"INK",3) == 0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			break;

		case (char)ID3_UPPERCASE_M:
			if (strncmp((const char *)&tempbuf[1],"CDI",3) == 0) /* QAC : 12.4 */
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else if (strncmp((const char *)&tempbuf[1], "LLT", 3) == 0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else
			{
				; /* No Statement */
			}
			break;

		case (char)ID3_UPPERCASE_O:
			if (strncmp((const char *)&tempbuf[1],"WNE",3) == 0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			break;

		case (char)ID3_UPPERCASE_P:
			if (strncmp((const char *)&tempbuf[1],"RIV",3) == 0) /* QAC : 12.4 */
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else if (strncmp((const char *)&tempbuf[1], "CNT", 3) == 0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else if (strncmp((const char *)&tempbuf[1], "OPM", 3) == 0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else if (strncmp((const char *)&tempbuf[1], "OSS", 3) == 0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else
			{
				; /* No Statement */
			}
			break;

		case (char)ID3_UPPERCASE_R:
			if (strncmp((const char *)&tempbuf[1],"BUF",3) == 0) /* QAC : 12.4 */
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else if (strncmp((const char *)&tempbuf[1], "VA2", 3) == 0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else if (strncmp((const char *)&tempbuf[1], "VRB", 3) == 0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else
			{
				; /* No Statement */
			}
			break;

		case (char)ID3_UPPERCASE_S:
			if (strncmp((const char *)&tempbuf[1],"EEK",3) == 0) /* QAC : 12.4 */
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else if (strncmp((const char *)&tempbuf[1], "IGN", 3) == 0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else if (strncmp((const char *)&tempbuf[1], "YLT", 3) == 0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else if (strncmp((const char *)&tempbuf[1], "YTC", 3) == 0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else
			{
				; /* No Statement */
			}
			break;

		case (char)ID3_UPPERCASE_T:
			switch (tempbuf[1])
			{
				case (char)ID3_UPPERCASE_A:
					if ((tempbuf[2]  == (char)ID3_UPPERCASE_L) && (tempbuf[3]  == (char)ID3_UPPERCASE_B))
					{
						iFrameInfo = (int32_t)ID3_ALBUM;
					}
					break;

				case (char)ID3_UPPERCASE_B:
					if ((tempbuf[2]  == (char)ID3_UPPERCASE_P) && (tempbuf[3]  == (char)ID3_UPPERCASE_M))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_C:
					if (tempbuf[2]  == (char)ID3_UPPERCASE_O)
					{
						switch (tempbuf[3])
						{
							case (char)ID3_UPPERCASE_P:
#ifdef ID3_EXT_INCLUDE
								iFrameInfo = (int32_t)ID3_COPYRIGHT;
#else
								iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
#endif
								break;
							case (char)ID3_UPPERCASE_M:
#ifdef ID3_EXT_INCLUDE
								iFrameInfo = (int32_t)ID3_COMPOSER;
#else
								iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
#endif
								break;
							case (char)ID3_UPPERCASE_N:
								iFrameInfo = (int32_t)ID3_GENRE;
								break;
							default:
								break;
						}
					}
					break;

				case (char)ID3_UPPERCASE_D:
					if (((tempbuf[2]  == (char)ID3_UPPERCASE_E) && (tempbuf[3]  == (char)ID3_UPPERCASE_N))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_L) && (tempbuf[3]  == (char)ID3_UPPERCASE_Y))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_O) && (tempbuf[3]  == (char)ID3_UPPERCASE_R))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_R) && (tempbuf[3]  == (char)ID3_UPPERCASE_C))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_R) && (tempbuf[3]  == (char)ID3_UPPERCASE_L))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_T) && (tempbuf[3]  == (char)ID3_UPPERCASE_G)))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_E:
					if ((tempbuf[2]  == (char)ID3_UPPERCASE_N) && (tempbuf[3]  == (char)ID3_UPPERCASE_C))
					{
#ifdef ID3_EXT_INCLUDE
						iFrameInfo = (int32_t)ID3_ENCODEDBY;
#else
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
#endif
					}
					else if ((tempbuf[2]  == (char)ID3_UPPERCASE_X) && (tempbuf[3]  == (char)ID3_UPPERCASE_T))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					else
					{
						; /* No Statement */
					}
					break;

				case (char)ID3_UPPERCASE_F:
					if ((tempbuf[2]  == (char)ID3_UPPERCASE_L) && (tempbuf[3]  == (char)ID3_UPPERCASE_T))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_I:
					if ( (tempbuf[2]  == (char)ID3_UPPERCASE_T) && (tempbuf[3]  == (char)ID3_TWO) )
					{
						iFrameInfo = (int32_t)ID3_TITLE;
					}
					else if (((tempbuf[2]  == (char)ID3_UPPERCASE_P) && (tempbuf[3]  == (char)ID3_UPPERCASE_L))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_T) && (tempbuf[3]  == (char)ID3_ONE))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_T) && (tempbuf[3]  == (char)ID3_THREE)))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					else
					{
						; /* No Statement */
					}
					break;

				case (char)ID3_UPPERCASE_K:
					if ((tempbuf[2]  == (char)ID3_UPPERCASE_E) && (tempbuf[3]  == (char)ID3_UPPERCASE_Y))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_L:
					if (((tempbuf[2]  == (char)ID3_UPPERCASE_A) && (tempbuf[3]  == (char)ID3_UPPERCASE_N))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_E) && (tempbuf[3]  == (char)ID3_UPPERCASE_N)))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_M:
					if (((tempbuf[2]  == (char)ID3_UPPERCASE_C) && (tempbuf[3]  == (char)ID3_UPPERCASE_L))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_E) && (tempbuf[3]  == (char)ID3_UPPERCASE_D))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_O) && (tempbuf[3]  == (char)ID3_UPPERCASE_O)))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_O:
					if ( (tempbuf[2]  == (char)ID3_UPPERCASE_P) && (tempbuf[3]  == (char)ID3_UPPERCASE_E) )
					{
#ifdef ID3_EXT_INCLUDE
						iFrameInfo = (int32_t)ID3_ORIGARTIST;
#else
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
#endif
					}
					else if (((tempbuf[2]  == (char)ID3_UPPERCASE_A) && (tempbuf[3]  == (char)ID3_UPPERCASE_L))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_F) && (tempbuf[3]  == (char)ID3_UPPERCASE_N))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_L) && (tempbuf[3]  == (char)ID3_UPPERCASE_Y))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_W) && (tempbuf[3]  == (char)ID3_UPPERCASE_N)))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					else
					{
						; /* No Statement */
					}
					break;

				case (char)ID3_UPPERCASE_P:
					if ( (tempbuf[2]  == (char)ID3_UPPERCASE_E) && (tempbuf[3]  == (char)ID3_ONE) )
					{
						iFrameInfo = (int32_t)ID3_ARTIST;
					}
#ifdef ID3_EXT_INCLUDE
					else if ( (tempbuf[2]  == (char)ID3_UPPERCASE_E) && (tempbuf[3]  == (char)ID3_TWO) )
					{
						iFrameInfo = (int32_t)ID3_ALBUMARTIST;
					}
#else
					else if ( (tempbuf[2]  == (char)ID3_UPPERCASE_E) && (tempbuf[3]  == (char)ID3_TWO) )
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
#endif
					else if (((tempbuf[2]  == (char)ID3_UPPERCASE_E) && (tempbuf[3]  == (char)ID3_THREE))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_E) && (tempbuf[3]  == (char)ID3_FOUR))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_O) && (tempbuf[3]  == (char)ID3_UPPERCASE_S))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_R) && (tempbuf[3]  == (char)ID3_UPPERCASE_O))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_U) && (tempbuf[3]  == (char)ID3_UPPERCASE_B)))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					else
					{
						; /* No Statement */
					}
					break;

				case (char)ID3_UPPERCASE_R:
					if ( (tempbuf[2]  == (char)ID3_UPPERCASE_C) && (tempbuf[3]  == (char)ID3_UPPERCASE_K) )
					{
						iFrameInfo = (int32_t)ID3_TRACK;
					}
					else if (((tempbuf[2]  == (char)ID3_UPPERCASE_S) && (tempbuf[3]  == (char)ID3_UPPERCASE_N))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_S) && (tempbuf[3]  == (char)ID3_UPPERCASE_O)))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					else
					{
						; /* No Statement */
					}
					break;

				case (char)ID3_UPPERCASE_S:
					if (((tempbuf[2]  == (char)ID3_UPPERCASE_O) && (tempbuf[3]  == (char)ID3_UPPERCASE_A))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_O) && (tempbuf[3]  == (char)ID3_UPPERCASE_P))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_O) && (tempbuf[3]  == (char)ID3_UPPERCASE_T))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_R) && (tempbuf[3]  == (char)ID3_UPPERCASE_C))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_S) && (tempbuf[3]  == (char)ID3_UPPERCASE_E))
						|| ((tempbuf[2]  == (char)ID3_UPPERCASE_S) && (tempbuf[3]  == (char)ID3_UPPERCASE_T)))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_X:
					if ((tempbuf[2]  == (char)ID3_UPPERCASE_X) && (tempbuf[3]  == (char)ID3_UPPERCASE_X))
					{
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
					}
					break;

				case (char)ID3_UPPERCASE_Y:
					if ( (tempbuf[2]  == (char)ID3_UPPERCASE_E) && (tempbuf[3]  == (char)ID3_UPPERCASE_R) )
					{
						iFrameInfo = (int32_t)ID3_YEAR;
					}
					break;

				default:
					break;
			}
			break;

		case (char)ID3_UPPERCASE_U:
			if (strncmp((const char *)&tempbuf[1],"FID",3) == 0) /* QAC : 12.4 */
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else if (strncmp((const char *)&tempbuf[1], "SER", 3) == 0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else if (strncmp((const char *)&tempbuf[1], "SLT", 3) == 0)
			{
				iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
			}
			else
			{
				; /* No Statement */
			}
			break;

		case (char)ID3_UPPERCASE_W:
			if ((tempbuf[1] >= (char)ID3_ZERO) && (tempbuf[1] <= (char)ID3_UPPERCASE_Z))
			{
				if ((tempbuf[2] >= (char)ID3_ZERO) && (tempbuf[2] <= (char)ID3_UPPERCASE_Z))
				{
					if ((tempbuf[3] >= (char)ID3_ZERO) && (tempbuf[3] <= (char)ID3_UPPERCASE_Z))
					{
#ifdef ID3_EXT_INCLUDE
						iFrameInfo = (int32_t)ID3_URL;
#else
						iFrameInfo = (int32_t)ID3_NOTUSEDFRAME;
#endif
					}
				}
			}
			break;

		default:
			break;
	}
	return iFrameInfo;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Function    : ID3_GetFrameInfo
//
// Description : this is ID3 TAG 2.x
//
///////////////////////////////////////////////////////////////////////////////////
static int32_t ID3_GetFrameInfo(const char *tempbuf, int32_t iVersion)
{
	int32_t iFrameInfo;

	if (iVersion<3)
	{
		iFrameInfo = ID3_GetFrameInfoV22(tempbuf);
	}
	else
	{
		iFrameInfo = ID3_GetFrameInfoV2X(tempbuf);
	}

	return iFrameInfo;
}

static int32_t ID3_ParseMetaData2UTF8(uint8_t *tempbuf,uint8_t *cTargetBuf,int32_t iStringSize)
{
	pID3V2XEncodeInfoType pID3V2XEncodeInfo;
	uint8_t *pSrcBuf;
	int32_t iStringCnt;
	int32_t sSize = iStringSize;
	pID3V2XEncodeInfo = (pID3V2XEncodeInfoType)tempbuf;
	iStringCnt   	  = 0;

	switch (pID3V2XEncodeInfo->mEncodeType & (uint8_t)ID3_ENCODE_MASK)
	{
		case ID3_ENCODE_UTF16:

			if ((pID3V2XEncodeInfo->mUnicodeBOM != (uint16_t)ID3_UNICODE_BOM1)&&
			        (pID3V2XEncodeInfo->mUnicodeBOM != (uint16_t)ID3_UNICODE_BOM2))
			{
			}
			else
			{
				sSize-=3;

				if (sSize <= 0 )
				{
				}
				else
				{
					pSrcBuf	  = &tempbuf[3];
					iStringCnt = Unicode2UTF8(pSrcBuf, &cTargetBuf[ID3_UNICODE_START], sSize, MAX_ID3V2_FIELD);

					cTargetBuf[0] = (uint8_t)iStringCnt  + (uint8_t)1;
				}
			}
			break;
		case ID3_ENCODE_UTF16BE:
			sSize--;

			if (sSize <= 0 )
			{
			}
			else 			/* QAC : 14.6 */
			{
				pSrcBuf	   = &tempbuf[1];
				iStringCnt = Unicode2UTF8(pSrcBuf, &cTargetBuf[ID3_UNICODE_START],sSize, MAX_ID3V2_FIELD);
				cTargetBuf[0] = (uint8_t)iStringCnt + (uint8_t)1;
			}

			break;
		case ID3_ENCODE_UTF8:

			//out unicode buffer needs iStringSize*2.
			//so if length exceeds the buffer size, ..
			if ( sSize > (MAX_ID3V2_FIELD/2) )
			{
				sSize = MAX_ID3V2_FIELD/2;
			}

			sSize--;

			if (sSize <= 0 )
			{
			}
			else
			{
				pSrcBuf	 = &tempbuf[1];
				iStringCnt = sSize;

				(void)memcpy(&cTargetBuf[ID3_UNICODE_START],pSrcBuf,(uint32_t)sSize);

				cTargetBuf[0] = (uint8_t)iStringCnt+(uint8_t)1;

			}
			break;
		case ID3_ENCODE_ISO88591:
			iStringCnt = -1;
			break;
		default:
			break;
	}

	return iStringCnt;
}

///////////////////////////////////////////////////////////////////////////////////
//
// Function    : ID3_GetMetaData
//
// Description : Get meta data
//
///////////////////////////////////////////////////////////////////////////////////
#define ID3_GENRE_TAG_V1 // using only id3v1 tag in genre
static int32_t ID3_GetMetaData(uint8_t *tempbuf,uint8_t *cTargetBuf,int32_t iStringSize,int32_t iFrameInfo)
{
	int32_t Result;
	uint8_t  tempTargebuffer[MAX_ID3V2_FIELD];
	int32_t sSize = iStringSize;
	(void)memset(tempTargebuffer,0x00,MAX_ID3V2_FIELD);
	// parsing
	Result = ID3_ParseMetaData2UTF8(tempbuf,tempTargebuffer,sSize);
	if(Result == -1 ) // ID3_ENCODE_ISO88591
	{
		if ( sSize > (MAX_ID3V2_FIELD/2) )
		{
			sSize = (int32_t)MAX_ID3V2_FIELD/2;
		}
		//make uni-code from local code
		uint8_t  tempUTF16buffer[MAX_ID3V2_FIELD];
		Result = UNI_ConvertStringToUnicode(&tempbuf[1], tempUTF16buffer, (uint32_t)sSize-(uint32_t)1);
		if(Result >0 )
		{
			Result = Unicode2UTF8(tempUTF16buffer, &tempTargebuffer[ID3_UNICODE_START], Result,MAX_ID3V2_FIELD);
		}

		if(Result > 0)
		{
			tempTargebuffer[0] = (uint8_t)Result +(uint8_t)1;
		}
		else
		{
			tempTargebuffer[0] = 0;
		}
	}

	if (iFrameInfo == (int32_t)ID3_GENRE)
	{
		int32_t ID3V1_firstPosition =0;
		int32_t ID3V1_endPosition;
		int32_t v1TagSize = 0; // 0ffset 0 ~

		// check id3 tag 1 in Genre tag
		while (ID3V1_firstPosition<Result )
		{
			if(tempTargebuffer[1+ID3V1_firstPosition]==(uint8_t)'(')
			{
				ID3V1_endPosition=ID3V1_firstPosition;
				while(ID3V1_endPosition<Result)
				{
					if(tempTargebuffer[1+ID3V1_endPosition]==(uint8_t)')')
					{
						v1TagSize = ID3V1_endPosition-ID3V1_firstPosition+1;
						break;
					}
					ID3V1_endPosition++;
				}
			}

			if(v1TagSize !=0)
			{
				break;
			}
			ID3V1_firstPosition++;
		}

		if (v1TagSize == 0 ) // only string
		{
			(void)memcpy(cTargetBuf, tempTargebuffer, MAX_ID3V2_FIELD);
		}
		else
		{
			// transfer unicode to number
			int32_t i;
			int32_t iSeed = 1;
			int32_t iTotal = 0;

			for(i=v1TagSize-1 ; i > 1 ; i--)
			{
				if ((tempTargebuffer[i]>=(uint8_t)'0') && (tempTargebuffer[i] <=(uint8_t)'9'))
				{
					iTotal += ((int32_t)tempTargebuffer[i]-(int32_t)'0') * iSeed;
					iSeed *= 10;
				}
				else
				{
					iTotal = -1;
					break;
				}
			}

			if((iTotal < 0)||(iTotal > ID3_GENRE_MAX_CNT)||(v1TagSize>5))
			{
				(void)memcpy(cTargetBuf, tempTargebuffer, MAX_ID3V2_FIELD);
			}
			else
			{
				if(v1TagSize == Result) // only v1tag
				{
					if (iTotal < ID3_GENRE_MAX_CNT)
					{
						cTargetBuf[0] = (uint8_t)strlen(ID3_GenreTable[iTotal]);
						(void)strncpy((char *)&cTargetBuf[1], &ID3_GenreTable[iTotal][0], strlen(ID3_GenreTable[iTotal]));
					}
				}
				else if ((v1TagSize+1) == Result)// v1tag + 00
				{
					if (iTotal < ID3_GENRE_MAX_CNT)
					{
						cTargetBuf[0] = (uint8_t)strlen(ID3_GenreTable[iTotal]);
						(void)strncpy((char *)&cTargetBuf[1], &ID3_GenreTable[iTotal][0], strlen(ID3_GenreTable[iTotal]));
					}
				}
				else if((v1TagSize+1) < Result) // v1tag + 00 + String
				{
					#ifdef ID3_GENRE_TAG_V1
					if (iTotal < ID3_GENRE_MAX_CNT)
					{
						cTargetBuf[0] = (uint8_t)strlen(ID3_GenreTable[iTotal]);
						(void)strncpy((char *)&cTargetBuf[1], &ID3_GenreTable[iTotal][0], strlen(ID3_GenreTable[iTotal]));
					}
					#endif

					#ifdef ID3_GENRE_TAG_STRING
					{
						int32_t i =0;
						int32_t j =0;
						int32_t uicodesize =tempTargebuffer[0];
						cTargetBuf[0] = uicodesize-(v1TagSize/2);

						for(i =0 ; i < Result ; i ++)
						{
							if((i>=ID3V1_firstPosition)&&(i<=ID3V1_endPosition+1))
							{
								;
							}
							else
							{
								cTargetBuf[1+j] = tempTargebuffer[1+i];
								j++;
							}
						}
					}
					#endif
				}
				else
				{
					;
				}
			}
		}
	}
	else
	{
		(void)memcpy(cTargetBuf, tempTargebuffer, MAX_ID3V2_FIELD);
	}
	return Result;
}

static int32_t ID3_UpdateUnunsyncData(uint8_t *out_data, const uint8_t *unsync_data, uint32_t data_size)
{
	uint32_t i, j, lCount;

	i = 0; j = 0; lCount = 0;

	while(j < data_size)
	{
		if(out_data != unsync_data)
		{
			out_data[i] = unsync_data[j];
		}
		else
		{
			if(lCount != (uint32_t)0)
			{
				out_data[i] = unsync_data[j];
			}
		}

		if((j+(uint32_t)1) != data_size)
		{
			if((unsync_data[j] == (uint8_t)0xff) && (unsync_data[(int32_t)j+1] == (uint8_t)0x00))
			{
				j++; lCount++;
			}
		}
		i++; j++;
	}

	return (int32_t)i;
}

static int32_t ID3_UpdateBufferData_FD(int32_t fd,uint8_t *tempbuf,int32_t iRemainSize,int32_t iOffset)
{
	int32_t result;
	int32_t iReadSize;

	if ( iRemainSize < MAX_ID3_FIND_SIZE)
	{
		iReadSize = iRemainSize;
	}
	else
	{
		iReadSize = MAX_ID3_FIND_SIZE;
	}

	result = lseek(fd,iOffset,SEEK_SET);
	if(result < 0)
	{
		(void)fprintf(stderr,"%s : file seek fail (%d)\n",__FUNCTION__, result);
		iReadSize =  -((int32_t)ID3_ERROR_FILEREAD);
	}
	else
	{
		result = read(fd, tempbuf, (uint32_t)iReadSize);
		if (result <= 0)
		{
			(void)fprintf(stderr,"%s : file read fail (%d)\n",__FUNCTION__, result);
			iReadSize =  -((int32_t)ID3_ERROR_FILEREAD);
		}
	}

	return iReadSize;
}

static int32_t ID3_GetID3_V2X_FD(int32_t fd, pstID3V2XInfoType pstID3V2XInfo,   uint8_t *tempbuf)
{
	int32_t iID3Index, iAllID3Get;
	int32_t i, j, ID3tag_headerSize=10;
	int32_t iFrameOffset;
	int32_t iFrameInfo;
	int32_t iFrameSize;
	int32_t iCopySize;
	int32_t iFileSize;
	int32_t iReadSize;
	int32_t iTotalSize;
	int32_t iRemainSize;
	int32_t iCurrentOffset;
	int32_t iMetaCnt = -1;
	int32_t iVersion;
	int32_t iOffset;
#ifdef JPEG_ID3_DISPLAY
	uint32_t uiImageTag;
#endif
	int32_t iUnSyncFlag;
	uint8_t lFrameHdrFlag;
	uint16_t lFrameFlag;
	uint8_t *pFrameData;
	int32_t sRet=0;
	pID3V2XFrameInfoType pID3V2XFrameInfo;
	pID3V22FrameInfoType pID3V22FrameInfo;
	uint8_t ID3UnsyncBuff[MAX_ID3_FIND_SIZE];
	int32_t ret;
	struct stat sb;

	ret = lseek (fd, 0, SEEK_END);
	if(ret < 0)
	{
		ret = -((int32_t)ID3_ERROR_FILEREAD);
		(void)fprintf(stderr, "seek error : offset (%d) \n",ret );
		return ret;
	}

	if(fstat(fd, &sb)== -1)
	{
		(void)fprintf(stderr, "fstat error \n");
		return -1;
	}

	iFileSize= sb.st_size;

	if(iFileSize <MAX_ID3V2_FIELD )// if file size is smaller than 10byte, it is invalid file size to parsing id3 tag.
	{
		return -((int32_t)ID3_ERROR_INVALID_FILE_SIZE);// do not process id3 meta gathering
	}

	iRemainSize   = iFileSize;
	//////////////////////////////////////////////////////////////////////
	iReadSize = ID3_UpdateBufferData_FD(fd,tempbuf,iRemainSize,0);
	if (iReadSize < 0)
	{
		return iReadSize;
	}

	iCurrentOffset = iReadSize;
	iRemainSize    -= iReadSize;

	if ((tempbuf[0] == (uint8_t)ID3_UPPERCASE_I)&& (tempbuf[1] == (uint8_t)ID3_UPPERCASE_D) && (tempbuf[2] == (uint8_t)ID3_THREE))
	{
		iTotalSize = ID3_GetID3V2xHeader(&tempbuf[0],&iFrameOffset,&iVersion, &lFrameHdrFlag);

		if (iTotalSize == -1)
		{
			return -((int32_t)ID3_ERROR_INVALID_HEADER);
		}
	}
	else
	{
		return -((int32_t)ID3_ERROR_NOT_SUPPORT);
	}

	if (iVersion < 3)
	{
		iOffset = (int32_t)sizeof(ID3V22FrameInfoType);
	}
	else
	{
		iOffset = (int32_t)sizeof(ID3V2XFrameInfoType);
	}

	i=iFrameOffset;		//Jhpark : i는 현재 읽어 들인 오프셋
	j=iFrameOffset;		// jhpark : j는 현재 읽어들인 데이터에서의 오프셋

	while ((i >= 0) && (i < iFileSize) && (i < (iTotalSize+(ID3tag_headerSize-3))))
	{
		if (iTotalSize > 0)
		{
			if (((j + iOffset) + MAX_ID3V2_FIELD) > iReadSize)
			{
				iCurrentOffset = i;
				iRemainSize    = iFileSize - i;
				if(iRemainSize <MAX_ID3V2_FIELD)
				{
					return -((int32_t)ID3_ERROR_INVALID_FILE_SIZE);// do not process id3 meta gathering
				}
				iReadSize = ID3_UpdateBufferData_FD(fd,tempbuf,iRemainSize,iCurrentOffset);
				if (iReadSize < 0)
				{
					return iReadSize;
				}
				iCurrentOffset += iReadSize;
				iRemainSize    -= iReadSize;
				j = 0;
			}

			iFrameInfo = ID3_GetFrameInfo((const char *)&tempbuf[j],iVersion);

			if (iFrameInfo != (int32_t)ID3_NONINFO)
			{
				iMetaCnt = 1;

				if (iVersion < 3)
				{
					if((lFrameHdrFlag & (uint8_t)ID3_FLAG_UNSYNCH) != (uint8_t)0)
					{
						pFrameData = ID3UnsyncBuff;
						(void)ID3_UpdateUnunsyncData(pFrameData, &tempbuf[j], (uint32_t)MAX_ID3V2_FIELD+(uint32_t)iOffset);
						iUnSyncFlag = 1;
						pID3V22FrameInfo = (pID3V22FrameInfoType)pFrameData;
						pFrameData = &pFrameData[iOffset];
					}
					else
					{
						iUnSyncFlag = 0;
						pID3V22FrameInfo = (pID3V22FrameInfoType)&tempbuf[j];
						pFrameData = &tempbuf[j+iOffset];
					}

					iFrameSize = ID3_Get3ByteValue(pID3V22FrameInfo->mSize);
					iCopySize = iFrameSize;

					if (iCopySize > MAX_ID3V2_FIELD)
					{
						iCopySize = MAX_ID3V2_FIELD - ID3_UNICODE_START;
					}
				}
				else
				{
					if(iVersion > 3)
					{
						pID3V2XFrameInfo = (pID3V2XFrameInfoType)&tempbuf[j];
						iFrameSize =  ID3_GetIntValue(pID3V2XFrameInfo->mSize);
						lFrameFlag = (uint16_t)ID3_GetShortValue(pID3V2XFrameInfo->mFlags);
						iCopySize = iFrameSize;

						if (iCopySize > MAX_ID3V2_FIELD)
						{
							iCopySize = MAX_ID3V2_FIELD - ID3_UNICODE_START;
						}

						if((iVersion == 4) &&
							(((lFrameHdrFlag & (uint8_t)ID3_FLAG_UNSYNCH) != (uint8_t)0)
							|| ((lFrameFlag & (uint8_t)ID3V2_FRAME_FORMAT_UNSYNCHRONISATION) != (uint8_t)0))
							)
						{
							pFrameData = ID3UnsyncBuff;
							iCopySize = ID3_UpdateUnunsyncData(pFrameData, &tempbuf[j+iOffset], (uint32_t)iCopySize);
							iUnSyncFlag = 2;
						}
						else
						{
							pFrameData = &tempbuf[j+iOffset];
							iUnSyncFlag = 0;
						}
					}
					else
					{
						if((lFrameHdrFlag & (uint8_t)ID3_FLAG_UNSYNCH) != (uint8_t)0)
						{
							pFrameData = ID3UnsyncBuff;
							(void)ID3_UpdateUnunsyncData(pFrameData, &tempbuf[j], (uint32_t)MAX_ID3V2_FIELD+(uint32_t)iOffset);
							iUnSyncFlag = 1;
							pID3V2XFrameInfo = (pID3V2XFrameInfoType)pFrameData;
							pFrameData = &pFrameData[iOffset];
						}
						else
						{
							iUnSyncFlag = 0;
							pID3V2XFrameInfo = (pID3V2XFrameInfoType)&tempbuf[j];
							pFrameData = &tempbuf[j+iOffset];
						}

						iFrameSize =  ID3_Get4byteIntValue(pID3V2XFrameInfo->mSize);
						(void)ID3_GetShortValue(pID3V2XFrameInfo->mFlags);
						iCopySize = iFrameSize;
						if (iCopySize > MAX_ID3V2_FIELD)
						{
							iCopySize = MAX_ID3V2_FIELD - ID3_UNICODE_START;
						}
					}
				}

				switch (iFrameInfo)
				{
					case (int32_t)ID3_TRACK:
					{
						int32_t k;
						int32_t Dec = 1;
						pstID3V2XInfo->mTrack = 0;

						for ( k = 0 ; k < (iFrameSize-1); k++)
						{
							Dec *= 10;
						}

						for ( k = 0; k < iFrameSize ; k++ )
						{
							if ((pFrameData[k] >= (uint8_t)ID3_ZERO) && (pFrameData[k] <= (uint8_t)ID3_NINE))
							{
								pstID3V2XInfo->mTrack += (uint8_t)((tempbuf[k] - (uint8_t)ID3_ZERO)*(uint8_t)Dec);
							}

							Dec /= 10;
						}
					}
					break;
					case (int32_t)ID3_YEAR:
						break;
					case (int32_t)ID3_NOTUSEDFRAME:
						break;
#ifdef	JPEG_ID3_DISPLAY
					case (int32_t)ID3_PICTURE :
						break;
#endif
					default:

						if (iFrameInfo < (int32_t)ID3_NONINFO)
						{
							sRet= ID3_GetMetaData(pFrameData, &pstID3V2XInfo->mMetaData[iFrameInfo][0], iCopySize, iFrameInfo);
						}

						break;
				}

				if((sRet ==-((int32_t)ID3_ERROR_BAD_ARGUMENTS))||(sRet == -((int32_t)ID3_ERROR_MALOC)))
				{
					return sRet; // do not process id3 meta gathering
				}

				// jhpark 2011.11.19. 원하는 ID3 tag만 구하면 바로 빠져 나감
				iAllID3Get = 0;
				for(iID3Index=0; iID3Index <(int32_t)ID3_NONINFO; iID3Index++)
				{
					if(pstID3V2XInfo->mMetaData[iID3Index][0]==(uint8_t)0x0)
					{
						iAllID3Get =-1;
						break;
					}
				}
				if(iAllID3Get==0)
				{
					return iMetaCnt;
				}

				if( iFrameSize >= 0)
				{
					if((iOffset + iFrameSize) > iTotalSize)	/* QAC : 12.1 */
					{
						iFrameSize = iOffset;
					}
					else
					{
						iFrameSize = (iOffset + iFrameSize);
					}
				}
				else
				{
					iFrameSize = iOffset;
				}
				j += iFrameSize;
				i += iFrameSize;

				if (j > iReadSize )
				{
					iCurrentOffset = i;
					iReadSize = ID3_UpdateBufferData_FD(fd,tempbuf,iRemainSize,iCurrentOffset);
					if (iReadSize < 0)
					{
						return iReadSize;
					}

					iCurrentOffset += iReadSize;
					iRemainSize    -= iReadSize;
					j = 0;
				}

				if(iUnSyncFlag == 0)
				{
					if ( (tempbuf[j] < (uint8_t)ID3_ZERO) || (tempbuf[j] > (uint8_t)ID3_UPPERCASE_Z) )
					{
						if ( (tempbuf[j+1] < (uint8_t)ID3_ZERO) || (tempbuf[j+1] > (uint8_t)ID3_UPPERCASE_Z) )
						{
							if ( (tempbuf[j+2] < (uint8_t)ID3_ZERO) || (tempbuf[j+2] > (uint8_t)ID3_UPPERCASE_Z) )
							{
								return iMetaCnt;
							}
						}
					}
				}
			}
			else
			{
				j++;
				i++;
			}
		}

		if (j > iReadSize )
		{
			iReadSize = ID3_UpdateBufferData_FD(fd,tempbuf,iRemainSize,iCurrentOffset);
			if (iReadSize < 0)
			{
				return iReadSize;
			}

			iCurrentOffset += iReadSize;
			iRemainSize    -= iReadSize;
			j = 0;
		}
	}

	return iMetaCnt;
}

int32_t ID3_GetMP3Data(int32_t fd, pstID3V2XInfoType pID3V2XInfo)
{
	int32_t retval;
	uint8_t *tempbuf;
	uint8_t ID3MetaBuff[MAX_ID3_FIND_SIZE];

	if((fd < 0)||(pID3V2XInfo == NULL))
	{
		retval  = -((int32_t)ID3_ERROR_BAD_ARGUMENTS);
	}
	else
	{
		tempbuf 	= ID3MetaBuff;

		(void )memset((void *)pID3V2XInfo, 0x00, sizeof(stID3V2XInfoType));
		retval = ID3_GetID3_V2X_FD(fd, pID3V2XInfo, tempbuf);
		if(retval == -((int32_t)ID3_ERROR_FILEREAD))
		{
			retval = -((int32_t)ID3_ERROR_FILEREAD);
		}
		else
		{
			if (retval < 0)
			{
				retval = ID3_GetID3_V1X_FD(fd, pID3V2XInfo, tempbuf);
			}
			if((retval <0)&&(retval != -((int32_t)ID3_ERROR_FILEREAD)))
			{
				retval = -((int32_t)ID3_ERROR_FILEREAD);
			}
		}
		if(retval >=0)
		{
			retval = (int32_t)ID3_NO_ERROR;
		}
	}
	return retval;
}

static const uint8_t META_UnKnown[] = { 0x55, 0x00, 0x6E, 0x00, 0x6B, 0x00, 0x6E, 0x00, 0x6F, 0x00,
                                       0x77, 0x00, 0x6E, 0x00, 0x00, 0x00
                                     };

