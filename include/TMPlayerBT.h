/****************************************************************************************
 *   FileName    : TMPlayerBT.h
 *   Description : TMPlayerBT.h
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

#ifndef TMPLAYER_BT_H
#define TMPLAYER_BT_H

#ifdef __cplusplus
extern "C" {
#endif
void TMPlayerBT_Release(void);

void TMPlayerBT_PlayStart(void);
void TMPlayerBT_PlayResume(void);
void TMPlayerBT_PlayRestart(void);
void TMPlayerBT_PlayStop(void);
void TMPlayerBT_PlayPause(void);
void TMPlayerBT_TrackUp(void);
void TMPlayerBT_TrackDown(void);
void TMPlayerBT_TrackMove(int number);
void TMPlayerBT_TrackSeek(unsigned char hour, unsigned char min, unsigned char sec);
void TMPlayerBT_TrackForward(void);
void TMPlayerBT_TrackRewind(void);
void TMPlayerBT_TrackFFREWEnd(void);
void TMPlayerBT_ChangeRepeat(void);
void TMPlayerBT_ChangeShuffle(void);
void TMPlayerBT_RequestList(int number, unsigned int count);
void TMPlayerBT_MetaBrowsingStart(unsigned int listcount);
void TMPlayerBT_MetaBrowsingMove(int index);
void TMPlayerBT_MetaBrowsingSelect(int index);
void TMPlayerBT_MetaBrowsingHome(unsigned int listcount);
void TMPlayerBT_MetaBrowsingUndo(void);
void TMPlayerBT_MetaBrowsingEnd(int index);

void TMPlayerBT_RefreshInfomation(void);
void TMPlayerBT_ClearInformation(void);

void TMPlayerBT_Connected(void);
void TMPlayerBT_Disconnected(void);
void TMPlayerBT_PlayTimeChange(uint32_t time);
void TMPlayerBT_TotalTimeChange(uint32_t totaltime);
void TMPlayerBT_TitleInfo(const char *title, uint32_t length);
void TMPlayerBT_ArtistInfo(const char *artist, uint32_t length);
void TMPlayerBT_AlbumInfo(const char *album, uint32_t length);
void TMPlayerBT_AlbumArtSupport(bool support, const char *uri);
void TMPlayerBT_AlbumArt(uint32_t length);
void TMPlayerBT_PlayStatusChange(uint32_t playstatus);
void TMPlayerBT_BrowsingSupport(bool support, uint32_t list_type);
void TMPlayerBT_SetBrowsingMode(bool folder);
void TMPlayerBT_BrowsingList(uint32_t idx, uint32_t *uid1, uint32_t *uid2, uint32_t *types, const char **names, uint32_t cnt);
void TMPlayerBT_BrowsingChanged(void);
void TMPlayerBT_TrackChange(uint32_t tracknum);
void TMPlayerBT_TotalTrackChange(uint32_t totalnum);
void TMPlayerBT_RepeatMode(uint32_t repeatMode);
void TMPlayerBT_ShuffleMode(uint32_t shuffleMode);
void TMPlayerBT_RepeatChange(uint32_t repeat);
void TMPlayerBT_ShuffleChange(uint32_t shuffle);

#define BRWS_TYPE_FOLDER		0x01
#define BRWS_TYPE_PLAYLIST		0x03

#define ITEM_TYPE_FOLDER		0x00
#define ITEM_TYPE_MEDIA			0x01
#ifdef __cplusplus
}
#endif

#endif


