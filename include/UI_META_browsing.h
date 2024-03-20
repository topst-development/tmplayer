/****************************************************************************************
 *   FileName    : UI_META_browsing.h
 *   Description : UI_META_browsing.h
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
/*****************************************************************************
******************************************************************************
*
*        File : UI_META_browsing.h
*
* Description : UI_META_browsing.h
*
*      Author : APLT
*
*      Status : NONE
*
*     History : Preliminary release 2004-4-07
*    
******************************************************************************
******************************************************************************/

#ifndef UI_META_NAVIGATION_H__
#define UI_META_NAVIGATION_H__


#if defined(__cplusplus)
extern "C"
{
#endif

#define UI_META_NAVI_EXTERN	extern

typedef void (*MetaCategoryMenuName_cb)( uint32_t dbNum, uint8_t mode, const char *pNenu);
typedef void (*MetaCategoryIndexChange_cb)(uint32_t dbNum, uint32_t totalNum, uint32_t currentNum);
typedef void (*MetaCategoryInfo_cb)(uint32_t dbNum, uint16_t index, const char *pName, uint32_t type);
typedef void (*MetaSelectTrack_cb)(uint32_t dbNum, uint32_t selectIndex);

typedef struct g_TCDB_MetaBrwsEventCB{
	MetaCategoryMenuName_cb			MetaCategoryMenuName;
	MetaCategoryIndexChange_cb		MetaCategoryIndexChange;
	MetaCategoryInfo_cb				MetaCategoryInfo;
	MetaSelectTrack_cb				MetaSelectTrack;	
}TCDB_MetaBrwsEventCB;


void UI_META_SetEventCallbackFunctions(TCDB_MetaBrwsEventCB *cb);

UI_META_NAVI_EXTERN int32_t UI_META_NAVI_ClearAllNaviInfo( void );		//Meta Brwosing Initialize 

UI_META_NAVI_EXTERN int32_t UI_META_NAVI_AddNaviInfo( uint32_t  dbNum );
UI_META_NAVI_EXTERN int32_t UI_META_NAVI_DeleteNaviInfo( uint32_t dbNum );		//Device Remove

UI_META_NAVI_EXTERN int32_t UI_META_NAVI_StartBrowse(uint32_t  dbNum, uint32_t ListNumInPage);	//Meta Browsing Start and Restart
UI_META_NAVI_EXTERN void UI_META_NAVI_SelectChildItem(uint32_t  dbNum, uint32_t CurrentIndex);		
UI_META_NAVI_EXTERN void UI_META_NAVI_SelectParentItem(uint32_t  dbNum);
UI_META_NAVI_EXTERN void UI_META_NAVI_Move(uint32_t  dbNum, uint32_t CurrentIndex);
UI_META_NAVI_EXTERN void UI_META_NAVI_ExitBrowse(uint32_t  dbNum, uint32_t nSelectIndex);			// ex, mode change 
UI_META_NAVI_EXTERN int32_t UI_META_NAVI_HomeBrowse(uint32_t  dbNum, uint32_t ListNumInPage);	// Go Home Menu


#define FIRST_SONG_INDEX (0x00)
#define FIRST_SONG  (0x01)

#define UI_META_NAVI_MAX_NUM	(TotalDeviceSources)

#define UI_META_NAVI_MAX_DEPTH	(5)

#define UI_META_NAVI_ITEM_EMPTY_VALUE	(0xffff)

#if defined(__cplusplus)
};
#endif

#endif /* UI_NAVIGATION_H__ */

/* End Of File */

