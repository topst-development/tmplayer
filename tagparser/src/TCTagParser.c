/****************************************************************************************
 *   FileName    : TCTagParser.c
 *   Description : TCTagParser.c
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
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "ID3Parser.h"

void TC_ClearID3V2Info(stID3V2XInfoType * ID3Data)
{
	if(ID3Data != NULL)
	{
		int32_t i, j;
		ID3Data->mUnSyncFlag =0;
		ID3Data->mTrack =0;
		for(i=0; i< MAX_ID3V2_YEAR;i++)
		{
			ID3Data->mYear[i]=0;
		}

		for(i=0; i< (int32_t)ID3_NONINFO;i++)
		{
			for(j=0; j< MAX_ID3V2_FIELD ; j++)
			{
				ID3Data->mMetaData[i][j]=0;
			}
		}
	}
}

int32_t TC_GetMP3Tag(const char *pfolderPath, const char * pfileName, stID3V2XInfoType * ID3Data)
{
	int32_t retval = (int32_t)ID3_NO_ERROR;
	if((pfolderPath !=NULL)&&(pfileName!=NULL)&&(ID3Data !=NULL))
	{
		int32_t dirfd;
		int32_t filefd;

		TC_ClearID3V2Info(ID3Data);
		dirfd = open(pfolderPath, O_RDONLY);
		if(dirfd >0 )
		{
			filefd = openat(dirfd, pfileName, O_RDONLY);
			if(filefd  > -1)
			{
				retval=(int32_t)ID3_GetMP3Data(filefd,ID3Data);
				(void)close(filefd);
			}
			(void)close(dirfd);
		}
		else
		{
			retval = -((int32_t)ID3_ERROR_FILEREAD);
		}
	}
	else
	{
		retval = -((int32_t)ID3_ERROR_BAD_ARGUMENTS);
	}

	return retval;
}


int32_t TC_GetWMATag(const char *pfolderPath, const char * pfileName, stID3V2XInfoType * ID3Data)
{
	int32_t retval;

	if((pfolderPath !=NULL)&&(pfileName!=NULL)&&(ID3Data !=NULL))
	{
#ifdef WMA_INCLUDE
		int32_t dirfd;
		int32_t filefd;
		TC_ClearID3V2Info(ID3Data);
		dirfd = open(pfolderPath, O_RDONLY);
		if(dirfd >0 )
		{
			filefd = openat(dirfd, pfileName, O_RDONLY);
			if(filefd  > -1)
			{
				retval = ID3_GetWMAData(filefd,ID3Data);
				(void)close(filefd);
			}
			(void)close(dirfd);
		}
		else
		{
			retval = -((int32_t)ID3_ERROR_FILEREAD);
		}
#else
		TC_ClearID3V2Info(ID3Data);
		retval = -((int32_t)ID3_ERROR_NOT_SUPPORT);
#endif
	}
	else
	{
		retval = -((int32_t)ID3_ERROR_BAD_ARGUMENTS);
	}

	return retval;
}


