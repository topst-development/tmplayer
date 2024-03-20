/****************************************************************************************
 *   FileName    : UI_META_browsing.c
 *   Description : UI_META_browsing.c
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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>
#include <ctype.h>
#include "sqlite3.h"
#include "TMPlayerType.h"
#include "TCDBDefine.h"
#include "TCDBGen.h"
#include "UI_META_browsing.h"
#if __cplusplus
#include <stdbool.h>
#endif 

/****************************************************************************
Description
	one page unit on Navigation
	3 lines ard one unit on CPU interface.	
****************************************************************************/

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
#define error(...) do {\
	(void)fprintf(stderr, "[%s:%d] ", __FUNCTION__, __LINE__); \
	(void)fprintf(stderr, __VA_ARGS__); \
} while (0)
#else
#define error(args...) do {\
	(void)fprintf(stderr, "[%s:%d] ", __FUNCTION__, __LINE__); \
	(void)fprintf(stderr, ##args); \
} while (0)
#endif

extern int32_t g_tc_debug;
#define DEBUG_META_BRWS_PRINTF(format, arg...) \
	if (g_tc_debug != 0) \
	{ \
		(void)fprintf(stderr, "[META BRWS] [%s:%d] : " format "", __FUNCTION__, __LINE__, ##arg); \
	}

#define ADD_LIST_NUM		(1)
#define NORMAL_LIST_NUM	(0)
#define ALL_INDEX_NUM	(0)

typedef enum{
	UI_META_NAVI_ARTIST,
	UI_META_NAVI_ALBUM,
	UI_META_NAVI_GENRE,
	UI_META_NAVI_TRACK,
	UI_META_NAVI_MAX
} UI_META_NAVI_IPOD_BRWS_TYPE;

typedef struct
{
	char Data[MAX_TAG_LENGTH];

} DBGEN_RECORD_TYPE, *PDBGEN_RECORD_TYPE;

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************
Description
	Navigation function list for meta
****************************************************************************/
typedef int32_t (*fUI_META_NAVI_Initialize)( uint32_t DiskType);
typedef int32_t (*fUI_META_NAVI_SelectParentItem)( uint32_t DiskType);
typedef int32_t (*fUI_META_NAVI_SelectChildItem)( uint32_t DiskType, int32_t nSelectIndex );
typedef int32_t (*fUI_META_NAVI_DrawList)( uint32_t DiskType );


typedef struct 
{
	fUI_META_NAVI_Initialize			InitBrowse;
	fUI_META_NAVI_SelectParentItem	SelectParentItem;
	fUI_META_NAVI_SelectChildItem	SelectChildItem;
	fUI_META_NAVI_DrawList			DrawList;
} UI_META_NAVI_FUNCs;

typedef struct
{
	uint32_t CurrentDBType;
	uint32_t SelectedDBType;
	uint32_t bAudioBookSelected;
	uint32_t bNoFile;
}UI_META_IPOD_CLASSIFIED_DATA;

typedef union 
{
	UI_META_IPOD_CLASSIFIED_DATA	stIPODData;
} UI_META_DEVICE_CLASSIFIED_DATA;

typedef struct
{
	uint32_t DiskType;
	uint32_t bIsInitialized;

	uint32_t zPageIndex;
	uint32_t zListNumInPage;
	uint32_t nTotalItemNumber;

	uint32_t RootCategory;
	uint32_t CurrentCategory;
	uint32_t nDepth;
	uint32_t nSavedListIndex[UI_META_NAVI_MAX_DEPTH];
	uint32_t nSavedDBIndex[UI_META_NAVI_MAX_DEPTH];		/* Save real DBIndex of SQL Table */

	uint32_t nMaxBrowsingListNumber;
	uint16_t *pBrowsingItemList;
	
	UI_META_DEVICE_CLASSIFIED_DATA ClassifiedData;
	
	UI_META_NAVI_FUNCs BrowsingFunc;
} UI_META_NAVI_INFO;


/****************************************************************************
Description

****************************************************************************/
static UI_META_NAVI_INFO gstMetaNaviInfo[UI_META_NAVI_MAX_NUM];

/****************************************************************************
Callbackl functions ( Static )
****************************************************************************/
static MetaCategoryMenuName_cb		MetaCategoryMenuName = NULL;
static MetaCategoryIndexChange_cb		MetaCategoryIndexChange = NULL;
static MetaCategoryInfo_cb				MetaCategoryInfo = NULL;
static MetaSelectTrack_cb				MetaSelectTrack = NULL;

/****************************************************************************
 Local functions ( Static )
****************************************************************************/

static uint32_t UI_META_NAVI_GetPageIndex( uint32_t DiskType );
static int32_t UI_META_NAVI_SetPageIndex( uint32_t DiskType, uint32_t zPageIndex );

static uint32_t UI_META_NAVI_GetListNumInPage( uint32_t DiskType );
static int32_t UI_META_NAVI_SetListNumInPage( uint32_t DiskType, uint32_t zListNumInPage );

static uint32_t UI_META_NAVI_GetTotalItemNumber( uint32_t DiskType );
static int32_t UI_META_NAVI_SetTotalItemNumber( uint32_t DiskType, uint32_t nTotalItemNumber );

static int32_t UI_META_NAVI_SetRootCategory( uint32_t DiskType, uint32_t RootCategory );

static uint32_t UI_META_NAVI_GetCurrentCategory( uint32_t DiskType );
static int32_t UI_META_NAVI_SetCurrentCategory( uint32_t DiskType, uint32_t CurrentCategory );

static uint32_t UI_META_NAVI_GetDepth( uint32_t DiskType );
static int32_t UI_META_NAVI_SetDepth( uint32_t DiskType, uint32_t nDepth );

static uint32_t UI_META_NAVI_GetSavedListIndex( uint32_t DiskType, uint32_t nDepth );
static int32_t UI_META_NAVI_SetSavedListIndex( uint32_t DiskType, uint32_t nDepth, uint32_t nSavedListIndex );

static UI_META_NAVI_INFO *UI_META_NAVI_GetNaviInfo( uint32_t DiskType );
#if 0
static int32_t UI_META_NAVI_GetNaviInfoIndex( uint32_t DiskType );
#endif

static int32_t UI_META_NAVI_IsListInitialized(  /*DISK_DEVICE*/uint32_t DiskType );
static int32_t UI_META_NAVI_SetListInitializationFlag(  /*DISK_DEVICE*/uint32_t DiskType, uint32_t bIsInitialized );

static UI_META_NAVI_FUNCs *UI_META_NAVI_GetBrowsingFunctions( uint32_t DiskType );

static void UI_META_NAVI_StartPlay(uint32_t DiskType, uint32_t selectIndex);


static uint32_t UI_META_NAVI_DBGEN_IsSelectAllCategory( CategoryType eCategory );
static CategoryType UI_META_NAVI_DBGEN_GetBrowserTopCategory( uint32_t zIndex ) ;
static const char *UI_META_NAVI_DBGEN_GetDBName( CategoryType eCategory) ;
static const char *UI_META_NAVI_DBGEN_GetCategoryName( CategoryType eCategory, uint32_t *length ) ;
static const char *UI_META_NAVI_DBGEN_GetMenuCategoryName( CategoryType eCategory, uint32_t *length ) ;
static CategoryType UI_META_NAVI_DBGEN_GetUpperCategory( uint32_t DiskType, CategoryType eCurrnetType, uint32_t nDepth );
static CategoryType UI_META_NAVI_DBGEN_GetSubCategory( CategoryType eCurrnetType);
static void UI_META_NAVI_DBGEN_DrawMenuName( uint32_t DiskType, CategoryType eCategory );
static int32_t UI_META_NAVI_DBGEN_InitializeBrowseList(uint32_t DiskType);
static int32_t UI_META_NAVI_DBGEN_SelectParentItem(uint32_t DiskType);
static int32_t UI_META_NAVI_DBGEN_SelectChildItem( uint32_t DiskType, int32_t nSelectIndex );
static int32_t UI_META_NAVI_DBGEN_DrawList( uint32_t DiskType);

void UI_META_SetEventCallbackFunctions(TCDB_MetaBrwsEventCB *cb)
{
	if(NULL != cb)
	{
		MetaCategoryMenuName = cb->MetaCategoryMenuName;
		MetaCategoryIndexChange = cb->MetaCategoryIndexChange;
		MetaCategoryInfo =  cb->MetaCategoryInfo;
		MetaSelectTrack = cb->MetaSelectTrack;
	}	
}

static uint32_t UI_META_NAVI_GetPageIndex( uint32_t DiskType )
{
	uint32_t lPageIndex;

	UI_META_NAVI_INFO *pstMetaNaviInfo;

	pstMetaNaviInfo = UI_META_NAVI_GetNaviInfo( DiskType );
	if ( NULL == pstMetaNaviInfo )
	{
		// Invalid
		lPageIndex = 0;
	}
	else
	{
		lPageIndex = pstMetaNaviInfo->zPageIndex;
	}	

	return lPageIndex;
}

static int32_t UI_META_NAVI_SetPageIndex( uint32_t DiskType, uint32_t zPageIndex )
{
	int32_t nErrorCode = 0;

	UI_META_NAVI_INFO *pstMetaNaviInfo;

	pstMetaNaviInfo = UI_META_NAVI_GetNaviInfo( DiskType );
	if ( NULL == pstMetaNaviInfo )
	{
		// Invalid
		nErrorCode = -1;
	}
	else
	{
		pstMetaNaviInfo->zPageIndex = zPageIndex;
	}	

	return nErrorCode;
}

static uint32_t UI_META_NAVI_GetListNumInPage( uint32_t DiskType )
{
	uint32_t lPageIndex;

	UI_META_NAVI_INFO *pstMetaNaviInfo;

	pstMetaNaviInfo = UI_META_NAVI_GetNaviInfo( DiskType );
	if ( NULL == pstMetaNaviInfo )
	{
		// Invalid
		lPageIndex = 0;
	}
	else
	{
		lPageIndex = pstMetaNaviInfo->zListNumInPage;
	}	

	return lPageIndex;
}

static int32_t UI_META_NAVI_SetListNumInPage( uint32_t DiskType, uint32_t zListNumInPage )
{
	int32_t nErrorCode = 0;

	UI_META_NAVI_INFO *pstMetaNaviInfo;

	pstMetaNaviInfo = UI_META_NAVI_GetNaviInfo( DiskType );
	if ( NULL == pstMetaNaviInfo )
	{
		// Invalid
		nErrorCode = -1;
	}
	else
	{
		pstMetaNaviInfo->zListNumInPage = zListNumInPage;
	}	

	return nErrorCode;
}

static uint32_t UI_META_NAVI_GetTotalItemNumber( uint32_t DiskType )
{
	uint32_t lTotalItemNumber;

	UI_META_NAVI_INFO *pstMetaNaviInfo;

	pstMetaNaviInfo = UI_META_NAVI_GetNaviInfo( DiskType );
	if ( NULL == pstMetaNaviInfo )
	{
		// Invalid
		lTotalItemNumber = 0;
	}
	else
	{
		lTotalItemNumber = pstMetaNaviInfo->nTotalItemNumber;
	}		

	return lTotalItemNumber;
}

static int32_t UI_META_NAVI_SetTotalItemNumber( uint32_t DiskType, uint32_t nTotalItemNumber )
{
	int32_t nErrorCode = 0;

	UI_META_NAVI_INFO *pstMetaNaviInfo;

	pstMetaNaviInfo = UI_META_NAVI_GetNaviInfo( DiskType );
	if ( NULL == pstMetaNaviInfo )
	{
		// Invalid
		nErrorCode = -1;
	}
	else
	{
		pstMetaNaviInfo->nTotalItemNumber = nTotalItemNumber;
	}	

	return nErrorCode;
}

static int32_t UI_META_NAVI_SetRootCategory( uint32_t DiskType, uint32_t RootCategory )
{
	int32_t nErrorCode = 0;
	UI_META_NAVI_INFO *pstMetaNaviInfo;

	pstMetaNaviInfo = UI_META_NAVI_GetNaviInfo( DiskType );
	if ( NULL == pstMetaNaviInfo )
	{
		// Invalid
		nErrorCode = -1;
	}
	else
	{
		pstMetaNaviInfo->RootCategory = RootCategory;
	}

	return nErrorCode;
}

static uint32_t UI_META_NAVI_GetCurrentCategory( uint32_t DiskType )
{
	uint32_t lCurrentCategory;
	
	UI_META_NAVI_INFO *pstMetaNaviInfo;

	pstMetaNaviInfo = UI_META_NAVI_GetNaviInfo( DiskType );
	if ( NULL == pstMetaNaviInfo )
	{
		// Invalid
		lCurrentCategory = 0;
	}
	else
	{
		lCurrentCategory = pstMetaNaviInfo->CurrentCategory;
	}

	return lCurrentCategory;
}

static int32_t UI_META_NAVI_SetCurrentCategory( uint32_t DiskType, uint32_t CurrentCategory )
{
	int32_t nErrorCode = 0;

	UI_META_NAVI_INFO *pstMetaNaviInfo;

	pstMetaNaviInfo = UI_META_NAVI_GetNaviInfo( DiskType );
	if ( NULL == pstMetaNaviInfo )
	{
		// Invalid
		nErrorCode = -1;
	}
	else
	{
		pstMetaNaviInfo->CurrentCategory = CurrentCategory;
	}

	return nErrorCode;
}

static uint32_t UI_META_NAVI_GetDepth( uint32_t DiskType )
{
	uint32_t lDepth;

	UI_META_NAVI_INFO *pstMetaNaviInfo;

	pstMetaNaviInfo = UI_META_NAVI_GetNaviInfo( DiskType );
	if ( NULL == pstMetaNaviInfo )
	{
		// Invalid
		lDepth = 0;
	}
	else
	{
		lDepth = pstMetaNaviInfo->nDepth;
	}

	return lDepth;
}

static int32_t UI_META_NAVI_SetDepth( uint32_t DiskType, uint32_t nDepth )
{
	int32_t nErrorCode = 0;

	UI_META_NAVI_INFO *pstMetaNaviInfo;

	pstMetaNaviInfo = UI_META_NAVI_GetNaviInfo( DiskType );
	if ( NULL == pstMetaNaviInfo )
	{
		// Invalid
		nErrorCode = -1;
	}
	else
	{
		pstMetaNaviInfo->nDepth = nDepth;
	}

	return nErrorCode;
}

static uint32_t UI_META_NAVI_GetSavedListIndex( uint32_t DiskType, uint32_t nDepth )
{
	uint32_t lSavedListIndex;

	UI_META_NAVI_INFO *pstMetaNaviInfo;

	pstMetaNaviInfo = UI_META_NAVI_GetNaviInfo( DiskType );
	if ( (NULL == pstMetaNaviInfo) || ((uint32_t)UI_META_NAVI_MAX_DEPTH<nDepth) )
	{
		// Invalid
		lSavedListIndex = 0;
	}
	else
	{
		lSavedListIndex = pstMetaNaviInfo->nSavedListIndex[nDepth];
	}

	return lSavedListIndex;
}

static int32_t UI_META_NAVI_SetSavedListIndex( uint32_t DiskType, uint32_t nDepth, uint32_t nSavedListIndex )
{
	int32_t nErrorCode = 0;

	UI_META_NAVI_INFO *pstMetaNaviInfo;

	pstMetaNaviInfo = UI_META_NAVI_GetNaviInfo( DiskType );
	if ( (NULL == pstMetaNaviInfo) || ((uint32_t)UI_META_NAVI_MAX_DEPTH<nDepth) )
	{
		// Invalid
		nErrorCode = -1;
	}
	else
	{
		if ( nSavedListIndex <= pstMetaNaviInfo->nTotalItemNumber )
		{
			pstMetaNaviInfo->nSavedListIndex[nDepth] = nSavedListIndex;
		}
		else
		{
			pstMetaNaviInfo->nSavedListIndex[nDepth] = 0;
		}
	}

	return nErrorCode;
}

static UI_META_NAVI_FUNCs *UI_META_NAVI_GetBrowsingFunctions( uint32_t DiskType )
{
	UI_META_NAVI_FUNCs *pstBrowsingFunc;

	UI_META_NAVI_INFO *pstMetaNaviInfo;

	pstMetaNaviInfo = UI_META_NAVI_GetNaviInfo( DiskType );
	if ( NULL == pstMetaNaviInfo )
	{
		// Invalid
		pstBrowsingFunc = NULL;
	}
	else
	{
		pstBrowsingFunc = &(pstMetaNaviInfo->BrowsingFunc);
	}

	return pstBrowsingFunc;
}

static UI_META_NAVI_INFO *UI_META_NAVI_GetNaviInfo( uint32_t DiskType )
{
	UI_META_NAVI_INFO *pstMetaNaviInfo = NULL ;
	uint32_t nDeviceIndex;
	
	for(nDeviceIndex=(uint32_t)0; nDeviceIndex<(uint32_t)UI_META_NAVI_MAX_NUM; nDeviceIndex++){
		if ( DiskType == gstMetaNaviInfo[nDeviceIndex].DiskType ){
			pstMetaNaviInfo = &gstMetaNaviInfo[nDeviceIndex];
			break;
		}
	}

	return pstMetaNaviInfo;
}

#if 0
static int32_t UI_META_NAVI_GetNaviInfoIndex( uint32_t DiskType )
{
	uint32_t nDeviceIndex = 0;
	int32_t zNaviInfoIndex = (-1);
	
	for(nDeviceIndex=0; nDeviceIndex<UI_META_NAVI_MAX_NUM; nDeviceIndex++){
		if ( DiskType == gstMetaNaviInfo[nDeviceIndex].DiskType ){
			zNaviInfoIndex = (int32_t)nDeviceIndex;
			break;
		}
	}

	return zNaviInfoIndex;
}
#endif

static int32_t UI_META_NAVI_IsListInitialized( /*DISK_DEVICE*/uint32_t DiskType )
{
	uint32_t nDeviceIndex;
	int32_t lIsInitialized = FALSE;
	
	for(nDeviceIndex= (uint32_t)0; nDeviceIndex<(uint32_t)UI_META_NAVI_MAX_NUM; nDeviceIndex++)
	{
		if ( DiskType == gstMetaNaviInfo[nDeviceIndex].DiskType )
		{
			if ( (uint32_t)TRUE == gstMetaNaviInfo[nDeviceIndex].bIsInitialized )
			{
				lIsInitialized = TRUE;
				break;
			}
		}
	}

	return lIsInitialized;
}

static int32_t UI_META_NAVI_SetListInitializationFlag( uint32_t DiskType, uint32_t bIsInitialized )
{
	int32_t nErrorCode = 0;
	UI_META_NAVI_INFO *pstMetaNaviInfo;

	pstMetaNaviInfo = UI_META_NAVI_GetNaviInfo( DiskType );

	if ( NULL == pstMetaNaviInfo )
	{
		// Invalid
		nErrorCode = -1;
	}
	else
	{
		if ( (uint32_t)0 == bIsInitialized )
		{
			pstMetaNaviInfo->bIsInitialized = FALSE;
		}
		else
		{
			pstMetaNaviInfo->bIsInitialized = TRUE;
		}
	}

	return nErrorCode;
}

#if 0 // not used currently
int32_t UI_META_NAVI_ClearAllNaviInfo( void )
{
	int32_t nErrorCode = 0;
	UI_META_NAVI_FUNCs *pstBrowsingFunc = NULL;
	UI_META_NAVI_INFO *pstMetaNaviInfo = NULL;
	uint32_t nDeviceIndex = 0;

	DEBUG_META_BRWS_PRINTF("\n");

	for(nDeviceIndex=0; nDeviceIndex<UI_META_NAVI_MAX_NUM; nDeviceIndex++)
	{
		pstMetaNaviInfo = &gstMetaNaviInfo[nDeviceIndex];
		pstBrowsingFunc = &(pstMetaNaviInfo->BrowsingFunc);

		pstMetaNaviInfo->DiskType			= 0;
		pstMetaNaviInfo->bIsInitialized		= 0;
		pstMetaNaviInfo->zPageIndex			= 0;
		pstMetaNaviInfo->nTotalItemNumber	= 0;
		pstMetaNaviInfo->RootCategory		= 0;
		pstMetaNaviInfo->CurrentCategory	= 0;
		pstMetaNaviInfo->nDepth				= 0;

		memset ( &(pstMetaNaviInfo->nSavedListIndex), 0x00, sizeof(pstMetaNaviInfo->nSavedListIndex) );
		
		pstMetaNaviInfo->nMaxBrowsingListNumber	= 0;
		if(pstMetaNaviInfo->pBrowsingItemList != 0)
		{
			(void) free(pstMetaNaviInfo->pBrowsingItemList);
			pstMetaNaviInfo->pBrowsingItemList	= 0;
		}

		memset ( &(pstMetaNaviInfo->ClassifiedData), 0x00, sizeof(pstMetaNaviInfo->ClassifiedData) );

		pstBrowsingFunc->InitBrowse			= NULL;
		pstBrowsingFunc->SelectParentItem	= NULL;
		pstBrowsingFunc->SelectChildItem	= NULL;
	}
	
	return nErrorCode;
}
#endif

int32_t UI_META_NAVI_AddNaviInfo( uint32_t dbNum )
{
	int32_t nErrorCode = 0;
	UI_META_NAVI_FUNCs *pstBrowsingFunc;
	UI_META_NAVI_INFO *pstMetaNaviInfo;

	// Check existance
	pstMetaNaviInfo = UI_META_NAVI_GetNaviInfo( dbNum );
	if ( NULL != pstMetaNaviInfo )
	{
		// Invalid
		nErrorCode = -1;
	}
	else
	{
		// Find empty info
		pstMetaNaviInfo = UI_META_NAVI_GetNaviInfo( 0 );
		if ( NULL == pstMetaNaviInfo )
		{
			// Invalid
			nErrorCode = -2;
		}
		else
		{
#if 0 /* Don't need this function for codesonar */
			int32_t zNaviInfoIndex = UI_META_NAVI_GetNaviInfoIndex( NO_DEVICE_NUM );

			if ( zNaviInfoIndex < 0){
				zNaviInfoIndex = 0;
			}
#endif			
			pstBrowsingFunc = &(pstMetaNaviInfo->BrowsingFunc);
			pstMetaNaviInfo->DiskType			= dbNum;
			pstMetaNaviInfo->bIsInitialized		= 0;
			pstMetaNaviInfo->zPageIndex			= 0;
			pstMetaNaviInfo->nTotalItemNumber	= 0;
			pstMetaNaviInfo->RootCategory		= 0;
			pstMetaNaviInfo->CurrentCategory	= 0;
			pstMetaNaviInfo->nDepth				= 0;
			(void)memset( &(pstMetaNaviInfo->nSavedListIndex), 0x00, sizeof(pstMetaNaviInfo->nSavedListIndex) );

			pstMetaNaviInfo->nMaxBrowsingListNumber = 0; /* Not used filed */
			pstMetaNaviInfo->pBrowsingItemList = NULL; /* Not used filed */

			pstBrowsingFunc->InitBrowse			= UI_META_NAVI_DBGEN_InitializeBrowseList;
			pstBrowsingFunc->SelectParentItem	= UI_META_NAVI_DBGEN_SelectParentItem;
			pstBrowsingFunc->SelectChildItem	= UI_META_NAVI_DBGEN_SelectChildItem;
			pstBrowsingFunc->DrawList			= UI_META_NAVI_DBGEN_DrawList;
		}
	}

	return nErrorCode;
}

int32_t UI_META_NAVI_DeleteNaviInfo( uint32_t dbNum )
{
	int32_t nErrorCode = 0;
	UI_META_NAVI_FUNCs *pstBrowsingFunc;
	UI_META_NAVI_INFO *pstMetaNaviInfo;
	DEBUG_META_BRWS_PRINTF("dbNum (%d)\n", dbNum);
	pstMetaNaviInfo = UI_META_NAVI_GetNaviInfo( dbNum );
	if ( NULL == pstMetaNaviInfo )
	{
		// Invalid
		nErrorCode = -1;
	}
	else
	{
		pstBrowsingFunc = UI_META_NAVI_GetBrowsingFunctions( dbNum );
#if 0 /* Redundant Condition - Codesonar */
		if ( NULL == pstBrowsingFunc )
		{
			// Invalid
			nErrorCode = -2;
		}
		else
#endif
		{
			pstMetaNaviInfo->DiskType			= 0;
			pstMetaNaviInfo->bIsInitialized		= 0;
			pstMetaNaviInfo->zPageIndex			= 0;
			pstMetaNaviInfo->nTotalItemNumber	= 0;
			pstMetaNaviInfo->RootCategory		= 0;
			pstMetaNaviInfo->CurrentCategory	= 0;
			pstMetaNaviInfo->nDepth				= 0;
			(void)memset ( &(pstMetaNaviInfo->nSavedListIndex), 0x00, sizeof(pstMetaNaviInfo->nSavedListIndex) );

			(void)memset ( &(pstMetaNaviInfo->ClassifiedData), 0x00, sizeof(pstMetaNaviInfo->ClassifiedData) );

			pstMetaNaviInfo->nMaxBrowsingListNumber	= 0;
			if(pstMetaNaviInfo->pBrowsingItemList != NULL)
			{
				(void) free(pstMetaNaviInfo->pBrowsingItemList);
				pstMetaNaviInfo->pBrowsingItemList	= NULL;
			}

			pstBrowsingFunc->InitBrowse			= NULL;
			pstBrowsingFunc->SelectParentItem	= NULL;
			pstBrowsingFunc->SelectChildItem	= NULL;
		}	

	}

	return nErrorCode;
}

int32_t UI_META_NAVI_StartBrowse( uint32_t  dbNum, uint32_t ListNumInPage )
{
	int32_t nErrorCode = 0;
	/*Get the current folder's information*/
	DEBUG_META_BRWS_PRINTF("dbNum (%d), ListNumInPage (%d)\n", dbNum, ListNumInPage);
	UI_META_NAVI_FUNCs *pstBrowsingFunc = UI_META_NAVI_GetBrowsingFunctions( dbNum );
	if ( NULL == pstBrowsingFunc )
	{
		nErrorCode = UI_META_NAVI_AddNaviInfo( dbNum );
		pstBrowsingFunc = UI_META_NAVI_GetBrowsingFunctions( dbNum );
		if ( NULL == pstBrowsingFunc )
		{
			nErrorCode = -2;
		}
	}

	if(nErrorCode == 0)
	{
		uint32_t lDepth;
		uint32_t lSavedListIndex;
		uint32_t bListInitialized;
		uint32_t lListNumInPage;

		bListInitialized = (uint32_t)UI_META_NAVI_IsListInitialized( dbNum );
		if ( (uint32_t)FALSE ==  bListInitialized )
		{
			if ( NULL != pstBrowsingFunc->InitBrowse )
			{
				nErrorCode = pstBrowsingFunc->InitBrowse(dbNum);
				(void) UI_META_NAVI_SetListNumInPage( dbNum, ListNumInPage );
				(void) UI_META_NAVI_SetListInitializationFlag( dbNum, TRUE );
			}
		}

		// Recover current object index
		lDepth = UI_META_NAVI_GetDepth( dbNum );
		(void)UI_META_NAVI_GetSavedListIndex( dbNum, lDepth );
#if 0
		lSavedListIndex = UI_META_NAVI_GetSavedListIndex( dbNum, lDepth );
		if ( (uint32_t)0 < lSavedListIndex )
		{
			lListNumInPage = UI_META_NAVI_GetListNumInPage( dbNum );
			if(lListNumInPage != (uint32_t)0)
			{
				lSavedListIndex = ((lSavedListIndex-(uint32_t)1) % lListNumInPage) + (uint32_t)1;
			}
		}
		else
		{
			lSavedListIndex = (uint32_t)0;
		}
#endif	
		if ( NULL != pstBrowsingFunc->DrawList )
		{
			nErrorCode = pstBrowsingFunc->DrawList(dbNum);
		}
	}

	if(nErrorCode != 0)
	{
		error("error : %d\n", nErrorCode);
	}
	return nErrorCode;	
}


void UI_META_NAVI_SelectParentItem(uint32_t  dbNum)
{
	UI_META_NAVI_FUNCs *pstBrowsingFunc;
	int32_t nErrorCode;
	DEBUG_META_BRWS_PRINTF("dbNum (%d)\n", dbNum);
	pstBrowsingFunc = UI_META_NAVI_GetBrowsingFunctions( dbNum );
	if ( (NULL != pstBrowsingFunc) && (NULL != pstBrowsingFunc->SelectParentItem) )
	{
		nErrorCode = pstBrowsingFunc->SelectParentItem(dbNum);
	}
	else
	{
		nErrorCode = -2;
	}

	if(nErrorCode != 0)
	{
		error("error : %d\n", nErrorCode);
	}
}

void UI_META_NAVI_SelectChildItem(uint32_t dbNum, uint32_t CurrentIndex)
{
	uint32_t zIndex; 
	UI_META_NAVI_FUNCs *pstBrowsingFunc;
	int32_t nErrorCode = 0;
	DEBUG_META_BRWS_PRINTF("dbNum (%d), CurrentIndex (%d)\n", dbNum, CurrentIndex);
	if ( (uint32_t)0 == UI_META_NAVI_GetTotalItemNumber( dbNum ) )
	{
		nErrorCode = -2;
	}
	else
	{
		zIndex = ( UI_META_NAVI_GetPageIndex( dbNum )*UI_META_NAVI_GetListNumInPage( dbNum )) + CurrentIndex;

		pstBrowsingFunc = UI_META_NAVI_GetBrowsingFunctions( dbNum );
		if (pstBrowsingFunc->SelectChildItem != NULL) /* Redundant Condition - Codesonar */
		{
			nErrorCode = pstBrowsingFunc->SelectChildItem( dbNum, (int32_t)zIndex );
		}
	}

	if(nErrorCode != 0)
	{
		error("error : %d\n", nErrorCode);
	}
}

void UI_META_NAVI_Move(uint32_t  dbNum, uint32_t CurrentIndex)
{
	UI_META_NAVI_FUNCs *pstBrowsingFunc;
	int32_t nErrorCode = 0;
	DEBUG_META_BRWS_PRINTF("dbNum (%d), CurrentIndex (%d)\n", dbNum, CurrentIndex);
	pstBrowsingFunc = UI_META_NAVI_GetBrowsingFunctions( dbNum );
	if ( NULL == pstBrowsingFunc) 
	{
		nErrorCode = -1;
	}
	else
	{
		uint32_t lTotalItemNumber = UI_META_NAVI_GetTotalItemNumber( dbNum );
		
		if( (uint32_t)0 == lTotalItemNumber )
		{
			nErrorCode = -2;
		}
		else
		{
			uint32_t lPageIndex, lListNumInPage;

			lListNumInPage = UI_META_NAVI_GetListNumInPage( dbNum );
			if(lListNumInPage !=(uint32_t) 0)
			{
				lPageIndex = CurrentIndex/lListNumInPage;
			}
			else
			{
				lPageIndex = CurrentIndex;
			}
			(void) UI_META_NAVI_SetPageIndex( dbNum, lPageIndex );

			if ( pstBrowsingFunc->DrawList != NULL)
			{
				nErrorCode = pstBrowsingFunc->DrawList(dbNum);
			}
		}
	}

	if(nErrorCode != 0)
	{
		error("error : %d\n", nErrorCode);
	}
}

#if 0
void UI_META_NAVI_MoveUp(uint32_t  dbNum, uint32_t CurrentIndex, uint32_t ListCount)
{
	UI_META_NAVI_FUNCs *pstBrowsingFunc = NULL;
	int32_t nErrorCode = 0;

	pstBrowsingFunc = UI_META_NAVI_GetBrowsingFunctions( dbNum );
	if ( NULL == pstBrowsingFunc) 
	{
		nErrorCode = -1;
	}
	else
	{
		uint32_t lTotalItemNumber = UI_META_NAVI_GetTotalItemNumber( dbNum );
		
		if( 0 == lTotalItemNumber )
		{
			nErrorCode = -2;
		}
		else
		{
			uint32_t lPageIndex, lListNumInPage;

			lListNumInPage = UI_META_NAVI_GetListNumInPage( dbNum );
			if(lListNumInPage != 0)
			{
				lPageIndex = CurrentIndex/lListNumInPage;
			}
			else
			{
				lPageIndex = CurrentIndex;
			}
			(void) UI_META_NAVI_SetPageIndex( dbNum, lPageIndex );

			if ( pstBrowsingFunc->DrawList != 0 )
			{
				nErrorCode = pstBrowsingFunc->DrawList(dbNum);
			}
		}
	}

	if(nErrorCode != 0)
	{
		error("error : %d\n", nErrorCode);
	}
}

void UI_META_NAVI_MoveDown(uint32_t  dbNum, uint32_t CurrentIndex, uint32_t ListCount)
{
	UI_META_NAVI_FUNCs *pstBrowsingFunc = NULL;
	int32_t nErrorCode = 0;

	pstBrowsingFunc = UI_META_NAVI_GetBrowsingFunctions( dbNum );
	if ( NULL == pstBrowsingFunc) 
	{
		nErrorCode = -1;
	}
	else
	{
		uint32_t lTotalItemNumber = UI_META_NAVI_GetTotalItemNumber( dbNum );
	
		if( 0 == lTotalItemNumber )
		{
			nErrorCode = -2;
		}
		else
		{
			uint32_t lPageIndex, lListNumInPage;

			lListNumInPage = UI_META_NAVI_GetListNumInPage( dbNum );
			if(lListNumInPage != 0)
			{
				lPageIndex = CurrentIndex/lListNumInPage;
			}
			else
			{
				lPageIndex = CurrentIndex;
			}
			(void) UI_META_NAVI_SetPageIndex( dbNum, lPageIndex );

			if ( pstBrowsingFunc->DrawList != 0 )
			{
				nErrorCode = pstBrowsingFunc->DrawList(dbNum);
			}
		}
	}

	if(nErrorCode != 0)
	{
		error("error : %d\n", nErrorCode);
	}
}
#endif

void UI_META_NAVI_ExitBrowse(uint32_t  dbNum, uint32_t nSelectIndex)
{
	uint32_t lDepth;
	DEBUG_META_BRWS_PRINTF("dbNum (%d), SelectIndex (%d) \n", dbNum, nSelectIndex);	
	// Store current object index
	lDepth = UI_META_NAVI_GetDepth( dbNum );

	(void) UI_META_NAVI_SetSavedListIndex( dbNum, lDepth, nSelectIndex );
}

int32_t UI_META_NAVI_HomeBrowse(uint32_t  dbNum, uint32_t ListNumInPage)
{
	UI_META_NAVI_FUNCs *pstBrowsingFunc;
	int32_t nErrorCode = 0;
	DEBUG_META_BRWS_PRINTF("dbNum (%d), ListNumInPage (%d)\n", dbNum, ListNumInPage);
	pstBrowsingFunc = UI_META_NAVI_GetBrowsingFunctions( dbNum );
	if ( NULL == pstBrowsingFunc) 
	{
		if(ListNumInPage == (uint32_t)0)
		{
			nErrorCode = -1;
		}
		else
		{
			nErrorCode = UI_META_NAVI_AddNaviInfo( dbNum );
			pstBrowsingFunc = UI_META_NAVI_GetBrowsingFunctions( dbNum );
			if ( NULL == pstBrowsingFunc )
			{
				nErrorCode = -2;
			}
		}
	}

	if(nErrorCode == 0)
	{
		// Set Depth, RootCategory, CurrentCategory
		(void) UI_META_NAVI_SetCurrentCategory( dbNum, 0 );
		(void) UI_META_NAVI_SetRootCategory( dbNum, 0 );
		(void) UI_META_NAVI_SetDepth( dbNum, 0 );
		(void) UI_META_NAVI_SetPageIndex( dbNum, 0 );

		// Call Init Browser
		if(pstBrowsingFunc->InitBrowse != NULL)
		{
			nErrorCode = pstBrowsingFunc->InitBrowse(dbNum);
			if(ListNumInPage != (uint32_t)0)
			{
				(void) UI_META_NAVI_SetListNumInPage( dbNum, ListNumInPage );
			}
			(void) UI_META_NAVI_SetListInitializationFlag( dbNum, TRUE );
		}

		// Draw Meta List
		if ( NULL != pstBrowsingFunc->DrawList )
		{
			nErrorCode = pstBrowsingFunc->DrawList(dbNum);
		}
	}

	if(nErrorCode != 0)
	{
		error("error : %d\n", nErrorCode);
	}
	return nErrorCode;
}

static void UI_META_NAVI_StartPlay(uint32_t DiskType, uint32_t selectIndex)
{
	if(NULL != MetaSelectTrack)
	{
		MetaSelectTrack(DiskType,selectIndex);
	}
}

///////////////////////////////////////////////////////////////////////////////////
//
//               Example of iPod meta browsing tree
//               
//        
// <Depth>	0				1				2				3				4
//
//		[GENRE]			Genre list		Artist list		Album list		Track list
//
//		[ARTIST]			Artist list		Album list		Track list
//
//		[ALBUM]			Album list	Track list
//
//		[TRACK]			Track list
//
//		[PLAYLIST]		Playlist list	Track list
//
//		[COMPOSER]		Composer list	Album list		Track list
//
//		[PODCAST]		Podcast list	Track list
//
//		[AUDIOBOOK]		Audiobook list
//
///////////////////////////////////////////////////////////////////////////////////

#define UI_META_ROOTCATEGORYDEPTH	(0)

#define UI_META_BACK_INDEX		(0)
#define UI_META_SELECTALL_INDEX	(1)

static uint32_t UI_META_NAVI_DBGEN_IsSelectAllCategory( CategoryType eCategory )
{
	uint32_t bSelectAllCategory;

	switch( eCategory )
	{
		case META_ARTIST:
		case META_ALBUM:
		case META_GENRE:
			bSelectAllCategory = 1;
			break;

		case META_TRACK:
			bSelectAllCategory = 0;
			break;

		default :
			bSelectAllCategory = 1;
			break;
	}
	return bSelectAllCategory;
}

static CategoryType UI_META_NAVI_DBGEN_GetBrowserTopCategory( uint32_t zIndex ) 
{
	CategoryType eCategory;

	switch( zIndex )
	{
		case (uint32_t)UI_META_NAVI_GENRE:
			eCategory = META_GENRE;
			break;

		case (uint32_t)UI_META_NAVI_ARTIST:
			eCategory = META_ARTIST;
			break;

		case (uint32_t)UI_META_NAVI_ALBUM:
			eCategory = META_ALBUM;
			break;

		case (uint32_t)UI_META_NAVI_TRACK:
			eCategory = META_TRACK;
			break;
		default :
			eCategory = META_GENRE;
			break;
	}

	return eCategory;
} 

static const char *UI_META_NAVI_DBGEN_GetDBName( CategoryType eCategory) 
{
	const char *pCategoryName;

	switch( eCategory )
	{
		case META_ARTIST:
			pCategoryName = "Artist\0";
			break;

		case META_ALBUM:
			pCategoryName = "Album\0";
			break;

		case META_GENRE:
			pCategoryName = "Genre\0";
			break;

		case META_TRACK:
			pCategoryName = "Title\0";
			break;
		default :
			pCategoryName = NULL;
			break;
	}

	return pCategoryName;
}

static const char *UI_META_NAVI_DBGEN_GetCategoryName( CategoryType eCategory, uint32_t *length ) 
{
	const char *pCategoryName;

	switch( eCategory )
	{
		case META_ARTIST:
			pCategoryName = "ARTIST\0";
			*length = 6;
			break;

		case META_ALBUM:
			pCategoryName = "ALBUM\0";
			*length = 5;
			break;

		case META_GENRE:
			pCategoryName = "GENRE\0";
			*length = 5;
			break;

		case META_TRACK:
			pCategoryName = "TRACK\0";
			*length = 5;
			break;
		default :
			pCategoryName = NULL;
			*length = 0;
			break;
	}

	return pCategoryName;
}

static const char *UI_META_NAVI_DBGEN_GetMenuCategoryName( CategoryType eCategory, uint32_t *length ) 
{
	const char *pCategoryName;

	switch( eCategory )
	{
		case META_ALL:
			pCategoryName = "CATEGORY\0";
			*length = 8;
			break;

		case META_ARTIST:
			pCategoryName = "ARTIST\0";
			*length = 6;
			break;

		case META_ALBUM:
			pCategoryName = "ALBUM\0";
			*length = 5;
			break;

		case META_GENRE:
			pCategoryName = "GENRE\0";
			*length = 5;
			break;

		case META_TRACK:
			pCategoryName = "TRACK\0";
			*length = 5;
			break;
		default :
			pCategoryName = "No Category\0";
			*length = 11;
			break;
	}

	return pCategoryName;
}

static CategoryType UI_META_NAVI_DBGEN_GetUpperCategory( uint32_t DiskType, CategoryType eCurrnetType, uint32_t nDepth )
{
	CategoryType eUpperCategory;

	if ( ((uint32_t)UI_META_ROOTCATEGORYDEPTH + (uint32_t)1) < nDepth )
	{
		switch( eCurrnetType )
		{
			case META_GENRE:
				eUpperCategory = META_ALL;
				break;
			case META_ARTIST:
				eUpperCategory = META_GENRE;
				break;
			case META_ALBUM:
				eUpperCategory = META_ARTIST;
				break;
			case META_TRACK:
				eUpperCategory = META_ALBUM;
				break;

			default:
				eUpperCategory = META_ALL;
				break;
		}
	}
	else
	{
		eUpperCategory = META_ALL;
	}

	return eUpperCategory;
}

static CategoryType UI_META_NAVI_DBGEN_GetSubCategory( CategoryType eCurrnetType)
{
	CategoryType eSubCategory;
	
	switch( eCurrnetType )
	{
		case META_GENRE:
			eSubCategory = META_ARTIST;
			break;
		case META_ARTIST:
			eSubCategory = META_ALBUM;
			break;
		case META_ALBUM:
			eSubCategory = META_TRACK;
			break;
		default:
			eSubCategory = META_ARTIST;
			break;
	}

	return eSubCategory;
}

static void UI_META_NAVI_DBGEN_DrawMenuName( uint32_t DiskType, CategoryType eCategory )
{
	const char *pName;
	uint32_t lNameLength;
	
	/*Get current menu name*/
	if ( META_ALL  == eCategory)
	{
		pName = "CATEGORY\0";
	}
	else
	{
		pName = UI_META_NAVI_DBGEN_GetMenuCategoryName( eCategory, &lNameLength );
	}

	/*Send Current's folder name on screen*/
	if(NULL != MetaCategoryMenuName )
	{
		MetaCategoryMenuName(DiskType, (uint8_t)eCategory, pName);
	}
}

static int32_t UI_META_NAVI_DBGEN_InitializeBrowseList(uint32_t DiskType)
{
	int32_t nTotalNumber;
	int32_t nErrorCode = 0;
	nTotalNumber = (int32_t)UI_META_NAVI_MAX;

	(void)TCDB_Meta_ResetBrowser(DiskType);
	(void) UI_META_NAVI_SetTotalItemNumber( DiskType, (uint32_t)nTotalNumber );
	(void) UI_META_NAVI_SetDepth( DiskType, 0 );
	(void) UI_META_NAVI_SetPageIndex( DiskType, 0 );

	if(NULL != MetaCategoryIndexChange)
	{
		MetaCategoryIndexChange(DiskType, (uint32_t)nTotalNumber, 0 );
	}

	return nErrorCode;
}

static int32_t UI_META_NAVI_DBGEN_SelectParentItem(uint32_t DiskType)
{
	int32_t nErrorCode = 0;

	DEBUG_META_BRWS_PRINTF("\n");

	if ( INVALID_INDEX== DiskType )
	{
		// iPod is not connected.
		nErrorCode = -1;
	}
	else
	{
		uint32_t lDepth = UI_META_NAVI_GetDepth( DiskType );
		CategoryType eCurrentCategory = (CategoryType)UI_META_NAVI_GetCurrentCategory( DiskType );
		CategoryType eUpperCategory = UI_META_NAVI_DBGEN_GetUpperCategory( DiskType, eCurrentCategory, lDepth );
		uint32_t nPageIndex;
		uint32_t nListNumInPage = UI_META_NAVI_GetListNumInPage( DiskType );
		uint32_t lSavedListIndex;

		if ( (uint32_t)0 < lDepth )
		{
			lDepth--;
			(void) UI_META_NAVI_SetDepth( DiskType, lDepth );
		}

		lSavedListIndex = UI_META_NAVI_GetSavedListIndex( DiskType, lDepth );
		if ( ((uint32_t)0 < lSavedListIndex) && (lSavedListIndex != (uint32_t)INVALID_INDEX) )
		{
			if(nListNumInPage != (uint32_t)0)
			{
				nPageIndex = (lSavedListIndex-(uint32_t)1) / nListNumInPage;
			}
			else
			{
				nPageIndex = (lSavedListIndex-(uint32_t)1);
			}
		}
		else
		{
			nPageIndex = 0;
			lSavedListIndex = 1;
		}

		if ( META_ALL != eCurrentCategory )
		{
			if ( META_ALL == eUpperCategory )
			{
				(void) UI_META_NAVI_SetCurrentCategory( DiskType, (uint32_t)eUpperCategory );
				(void) UI_META_NAVI_SetRootCategory( DiskType, (uint32_t)eUpperCategory );
				(void) UI_META_NAVI_SetDepth( DiskType, lDepth );
				(void) UI_META_NAVI_SetPageIndex( DiskType, nPageIndex );
				(void) UI_META_NAVI_SetTotalItemNumber( DiskType, (uint32_t)UI_META_NAVI_MAX );
				if(NULL != MetaCategoryIndexChange)
				{
					MetaCategoryIndexChange(DiskType,(uint32_t)UI_META_NAVI_MAX, lSavedListIndex-(uint32_t)1  );
				}				
				(void) UI_META_NAVI_DBGEN_DrawList(DiskType);
			}
			else
			{

				nErrorCode = TCDB_Meta_UndoCategory(DiskType);
				if ( DB_SUCCESS == nErrorCode )
				{
					int32_t nTotalNumber;
					nErrorCode = TCDB_Meta_GetNumberCategory(DiskType,(const char *)UI_META_NAVI_DBGEN_GetDBName(eUpperCategory));
					if ( 0 < nErrorCode )
					{
						nTotalNumber = nErrorCode;
						nErrorCode = DB_SUCCESS;
						(void) UI_META_NAVI_SetCurrentCategory( DiskType, (uint32_t)eUpperCategory );
						(void) UI_META_NAVI_SetDepth( DiskType, lDepth );
						(void) UI_META_NAVI_SetPageIndex( DiskType, nPageIndex );
						if(UI_META_NAVI_DBGEN_IsSelectAllCategory(eUpperCategory) != (uint32_t)0)
						{
							(void) UI_META_NAVI_SetTotalItemNumber( DiskType, (uint32_t)nTotalNumber + (uint32_t)ADD_LIST_NUM/*Plus one for 'Select all*/ );

							if(NULL != MetaCategoryIndexChange)
							{
								MetaCategoryIndexChange(DiskType,(uint32_t)nTotalNumber + (uint32_t)ADD_LIST_NUM, lSavedListIndex-(uint32_t)1 );
							}
						}
						else
						{
							(void) UI_META_NAVI_SetTotalItemNumber( DiskType, (uint32_t)nTotalNumber);

							if(NULL != MetaCategoryIndexChange)
							{
								MetaCategoryIndexChange(DiskType,(uint32_t)nTotalNumber, lSavedListIndex-(uint32_t)1 );
							}
						}
						(void) UI_META_NAVI_DBGEN_DrawList(DiskType);
					}
				}
			}

		}
	}

	return nErrorCode;
}


static int32_t UI_META_NAVI_DBGEN_SelectChildItem( uint32_t DiskType, int32_t nSelectIndex )
{
	int32_t nErrorCode = 0;
	uint32_t lTotalItemNumber = UI_META_NAVI_GetTotalItemNumber( DiskType );

	DEBUG_META_BRWS_PRINTF("select index (%d)\n",nSelectIndex);
	if ( INVALID_INDEX== DiskType )
	{
		// iPod is not connected.

		nErrorCode = -1;
	}
	else if ( nSelectIndex < 1 )
	{
		// Invalid parameter

		nErrorCode = -2;
	}
	else if ( lTotalItemNumber < (uint32_t)nSelectIndex )
	{
		// Invalid parameter
		nErrorCode = -3;
	}
	else
	{
		uint32_t lDepth = UI_META_NAVI_GetDepth( DiskType );

		{
			CategoryType eCurrentCategory = (CategoryType)UI_META_NAVI_GetCurrentCategory( DiskType );

			if ( META_ALL == eCurrentCategory )	
			{
				int32_t nTotalNumber;
				nErrorCode = TCDB_Meta_ResetBrowser(DiskType);

				if( DB_SUCCESS== nErrorCode )
				{
					eCurrentCategory = UI_META_NAVI_DBGEN_GetBrowserTopCategory( (uint32_t)nSelectIndex-(uint32_t)1 );

					(void) UI_META_NAVI_SetCurrentCategory( DiskType, (uint32_t)eCurrentCategory );

					(void) UI_META_NAVI_SetRootCategory( DiskType, (uint32_t)eCurrentCategory );

					nErrorCode = TCDB_Meta_GetNumberCategory(DiskType,(const char *)UI_META_NAVI_DBGEN_GetDBName(eCurrentCategory));
					if( 0 < nErrorCode )
					{
						nTotalNumber = nErrorCode;
						nErrorCode = DB_SUCCESS;
					}
					else
					{
						nErrorCode =-1;
					}
				}

				if( DB_SUCCESS == nErrorCode )
				{
					(void) UI_META_NAVI_SetSavedListIndex( DiskType, lDepth, (uint32_t)nSelectIndex );
					(void) UI_META_NAVI_SetDepth( DiskType, (lDepth+(uint32_t)1) );
					(void) UI_META_NAVI_SetPageIndex( DiskType, 0 );
					if(UI_META_NAVI_DBGEN_IsSelectAllCategory(eCurrentCategory) != (uint32_t)0)
					{
						(void) UI_META_NAVI_SetTotalItemNumber( DiskType, (uint32_t)nTotalNumber + (uint32_t)ADD_LIST_NUM/*Plus one for 'Select all*/ );

						if(NULL != MetaCategoryIndexChange)
						{
							MetaCategoryIndexChange(DiskType,(uint32_t)nTotalNumber + (uint32_t)ADD_LIST_NUM, 0 );
						}
					}
					else
					{
						(void) UI_META_NAVI_SetTotalItemNumber( DiskType, (uint32_t)nTotalNumber);

						if(NULL != MetaCategoryIndexChange)
						{
							MetaCategoryIndexChange(DiskType,(uint32_t)nTotalNumber, 0 );
						}						
					}
					(void) UI_META_NAVI_DBGEN_DrawList(DiskType);
				}
			}
			else
			{
				CategoryType eCategory;
				{
					(void) UI_META_NAVI_SetSavedListIndex( DiskType, lDepth, (uint32_t)nSelectIndex );

					if (META_TRACK == eCurrentCategory )
					{
					#if 0
						if ( UI_META_SELECTALL_INDEX == nSelectIndex )
						{
							nErrorCode = IPOD_SelectSortList( pHandle, eCurrentCategory, 0, eSortOrder/*SORTBYARTIST*/ );
						}
					#endif
					}

					/* Somtimes, PlayContol command returns command timeout on iPod classic.*/
					if(UI_META_NAVI_DBGEN_IsSelectAllCategory(eCurrentCategory) != (uint32_t)0)
					{
						if ( UI_META_SELECTALL_INDEX < nSelectIndex )
						{
							nErrorCode = TCDB_Meta_SelectCategory(DiskType,(const char *)UI_META_NAVI_DBGEN_GetDBName(eCurrentCategory), (nSelectIndex-(2+UI_META_BACK_INDEX)));
						}
					}
					else
					{
						/* PlayCurrentSelection is deprecated function. but, it is used for iPod compatible issue */
						if( META_TRACK == eCurrentCategory )
						{
							nErrorCode = TCDB_Meta_MakePlayList(DiskType, nSelectIndex-(1+UI_META_BACK_INDEX));
						}
						else
						{
							nErrorCode = TCDB_Meta_SelectCategory(DiskType,(const char *)UI_META_NAVI_DBGEN_GetDBName(eCurrentCategory), (nSelectIndex-(1+UI_META_BACK_INDEX)));
						}
					}

					if(DB_SUCCESS == nErrorCode) 
					{
						if ( META_TRACK == eCurrentCategory)
						{
							(void) UI_META_NAVI_SetTotalItemNumber( DiskType, lTotalItemNumber); 
							UI_META_NAVI_StartPlay(DiskType, (uint32_t)nSelectIndex-((uint32_t)1+(uint32_t)UI_META_BACK_INDEX));
						}
						else
						{
							int32_t nTotalNumber = 0;
							eCategory = UI_META_NAVI_DBGEN_GetSubCategory( eCurrentCategory );
							nErrorCode = TCDB_Meta_GetNumberCategory(DiskType,(const char *) UI_META_NAVI_DBGEN_GetDBName(eCategory));
							if(0 < nErrorCode )
							{	nTotalNumber = nErrorCode;
								nErrorCode = DB_SUCCESS;
							}
							if ( DB_SUCCESS == nErrorCode )
							{
								(void) UI_META_NAVI_SetCurrentCategory( DiskType, (uint32_t)eCategory );

								(void) UI_META_NAVI_SetDepth( DiskType, (lDepth+(uint32_t)1) );
								(void) UI_META_NAVI_SetPageIndex( DiskType, 0 );
								if(UI_META_NAVI_DBGEN_IsSelectAllCategory(eCategory) != (uint32_t)0)
								{
									(void) UI_META_NAVI_SetTotalItemNumber( DiskType, (uint32_t)nTotalNumber + (uint32_t)ADD_LIST_NUM/*Plus one for 'Select all*/ );

									if(NULL != MetaCategoryIndexChange)
									{
										MetaCategoryIndexChange(DiskType,(uint32_t)nTotalNumber + (uint32_t)ADD_LIST_NUM, 0 );
									}		
								}
								else
								{
									if(nTotalNumber == 0)
									{
										nTotalNumber = 1; /* for display "No File" */
									}
									(void) UI_META_NAVI_SetTotalItemNumber( DiskType, (uint32_t)nTotalNumber);		

									if(NULL != MetaCategoryIndexChange)
									{
										MetaCategoryIndexChange(DiskType,(uint32_t)nTotalNumber, 0 );
									}								
								}
								(void) UI_META_NAVI_DBGEN_DrawList(DiskType);
							}
						}
					}
				}
			}

		}
	}

	return nErrorCode;
}

static int32_t UI_META_NAVI_DBGEN_DrawList( uint32_t DiskType )
{
	int32_t nErrorCode = 0;
	CategoryType eCategory;
	CategoryType eCurrentCategory;
	uint32_t zListIndex;
	uint32_t lNameLength = 0;
	const char *pNameString;
	uint16_t idx;
	uint32_t lPageIndex = UI_META_NAVI_GetPageIndex( DiskType );
	uint32_t lListNumInPage = UI_META_NAVI_GetListNumInPage( DiskType );
	uint32_t lTotalItemNumber = UI_META_NAVI_GetTotalItemNumber( DiskType );
	DBGEN_RECORD_TYPE *pstRecordType = NULL;
	
	eCurrentCategory = (CategoryType)UI_META_NAVI_GetCurrentCategory( DiskType );

	// Draw menu title
	UI_META_NAVI_DBGEN_DrawMenuName( DiskType, eCurrentCategory );
	
	
	/*Set the initial value*/
	idx = (uint16_t)(lPageIndex*lListNumInPage);	/* starting ARRAY idx( 0 ~ )*/

	if((DiskType != INVALID_INDEX) && (META_ALL != eCurrentCategory)) 
	{
		int32_t zStartIndex;
		int32_t nRequestNumber;

		if(UI_META_NAVI_DBGEN_IsSelectAllCategory(eCurrentCategory) != (uint32_t)0)
		{
			if ((uint16_t)0 == idx)
			{
				zStartIndex = (int32_t)idx;
				if ( lTotalItemNumber < lListNumInPage )
				{
					nRequestNumber = (int32_t)lTotalItemNumber-ADD_LIST_NUM;
				}
				else
				{
					nRequestNumber = (int32_t)lListNumInPage-ADD_LIST_NUM;
				}
			}
			else
			{
				zStartIndex = (int32_t)idx-ADD_LIST_NUM;
				if( lTotalItemNumber < ((lPageIndex+(uint32_t)1)*lListNumInPage))
				{
					nRequestNumber = (int32_t)lTotalItemNumber - (int32_t)idx;
				}
				else
				{
					nRequestNumber = (int32_t)lListNumInPage;
				}
			}
		}
		else
		{
			{
				zStartIndex = (int32_t)idx;
				if ( lTotalItemNumber < ((lPageIndex + (uint32_t)1) * lListNumInPage) )
				{
					nRequestNumber = (int32_t)lTotalItemNumber - (int32_t)idx;
				}
				else
				{
					nRequestNumber = (int32_t)lListNumInPage;
				}
			}
		}

		if(nRequestNumber > 0)
		{
			pstRecordType = (DBGEN_RECORD_TYPE *)malloc(sizeof(DBGEN_RECORD_TYPE)* (uint32_t)nRequestNumber);
			(void)memset(pstRecordType, 0x00, sizeof(DBGEN_RECORD_TYPE)*(uint32_t)nRequestNumber);
			if(pstRecordType != NULL)
			{
				int32_t readIndex, i=0;
				DEBUG_META_BRWS_PRINTF("start Index (%d), Request Number (%d)\n", zStartIndex, nRequestNumber);
				for(readIndex = zStartIndex; readIndex <(nRequestNumber+zStartIndex);readIndex++)
				{
					DBGEN_RECORD_TYPE *pRecordType = &pstRecordType[i];
					nErrorCode = TCDB_Meta_GetCategoryListName(DiskType,(const char *) UI_META_NAVI_DBGEN_GetDBName(eCurrentCategory), (uint32_t)readIndex, (pRecordType->Data),MAX_TAG_LENGTH);
					i++;
					if( DB_SUCCESS != nErrorCode)
					{
						break;
					}
				}
			}
		}
	}

	if(nErrorCode == DB_SUCCESS)
	{
		int32_t i = 0;
		for(zListIndex=0 ; zListIndex<lListNumInPage ; zListIndex++)
		{
			if((idx+zListIndex) >= lTotalItemNumber)
			{
				break;
			}

			/*Display file icon and get file's information*/			
			{
				if ( META_ALL == eCurrentCategory )
				{
					eCategory = UI_META_NAVI_DBGEN_GetBrowserTopCategory( idx+zListIndex );
					pNameString = UI_META_NAVI_DBGEN_GetCategoryName( eCategory, &lNameLength );
				}
				else
				{
					if(UI_META_NAVI_DBGEN_IsSelectAllCategory(eCurrentCategory) != (uint32_t)0)
					{
						if( (uint32_t)ALL_INDEX_NUM == ((uint32_t)idx+zListIndex) )
						{
							lNameLength = 11;
							pNameString = (const char *)"Select All\0";
						}
						else
						{
							DBGEN_RECORD_TYPE *pRecordType = &pstRecordType[i];
							lNameLength = strlen(pRecordType->Data);
							pNameString = pRecordType->Data;
							i++;
						}
					}
					else
					{
						DBGEN_RECORD_TYPE *pRecordType = &pstRecordType[i];
						lNameLength = strlen(pRecordType->Data);
						pNameString = pRecordType->Data;
						i++;
					}
				}
			}			

			if ( NULL == pNameString )	
			{
				lNameLength = 8;
				pNameString = (const char *)"No name\0";
			}

			if( NULL != MetaCategoryInfo )
			{
				MetaCategoryInfo(DiskType,(idx+(uint16_t)zListIndex), pNameString, 0);
				DEBUG_META_BRWS_PRINTF("CategoryName : index (%d), Name (%s)\n",(idx+zListIndex), pNameString);
			}
		}
	}

	if(pstRecordType != NULL)
	{
		free(pstRecordType);
	}

	return nErrorCode;
}


#ifdef __cplusplus
}
#endif

/* End Of File */

