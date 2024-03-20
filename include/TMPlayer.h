/****************************************************************************************
 *   FileName    : TMPlayer.h
 *   Description : 
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

#ifndef TMPLAYER_H
#define TMPLAYER_H


#define KEY_STRING_SIZE		10

#ifdef __cplusplus
extern "C" {
#endif

int TMPlayer_Initialize();
void TMPlayer_Release();

void DeviceConnectProcess(char *key);
void DeviceDisconnectProcess(char *key);
void DeviceMountProcess(char *key, char *mountPath);
void TMPlayer_SetBrowsingMode(unsigned int device, unsigned char content, bool meta);
void TMPlayer_SetRepeatMode(unsigned int device, unsigned char content, unsigned int mode);
void TMPlayer_SetShuffleMode(unsigned int device, unsigned char content, unsigned int mode);
void TMPlayer_ChangeContentTypeProcess(void);
void TMPlayer_ChangeNextDeviceProcess(bool play);
void TMPlayer_ChangeDeviceProcess(int device, bool play);
void TMPlayer_ChangeRepeatModeProcess(void);
void TMPlayer_ChangeShuffleModeProcess(void);
void TMPlayer_FileListRequestProcess(unsigned int device, unsigned char content, int number, unsigned int count);
void TMPlayer_MetaBrowsingStartBrowsing(unsigned int device, unsigned int count);
void TMPlayer_MetaBrowsingRequestList(unsigned int device, int index);
void TMPlayer_MetaBrowsingSelectItem(unsigned int device, int index);
void TMPlayer_MetaBrowsingMoveToHome(unsigned int device, unsigned int count);
void TMPlayer_MetaBrowsingMoveToPreviousMenu(unsigned int device);
void TMPlayer_MetaBrowsingEndBrowsing(unsigned int device, int index);
void TMPlayer_TrackStartProcess(void);
void TMPlayer_TrackRestartProcess(void);
void TMPlayer_TrackUpProcess(void);
void TMPlayer_TrackDownProcess(void);
void TMPlayer_TrackMoveProcess(int number);
void TMPlayer_TrackStopProcess(void);
void TMPlayer_TrackResumeProcess(void);
void TMPlayer_TrackPauseProcess(void);
void TMPlayer_TrackSeekProcess(unsigned char hour, unsigned char min, unsigned char sec);
void TMPlayer_TrackForwardProcess(void);
void TMPlayer_TrackRewindProcess(void);
void TMPlayer_TrackFFREWEndProcess(void);
void TMPlayer_UpdateTimeProcess(unsigned char hour, unsigned char min, unsigned char sec);
int TMPlayer_GetPlayID(void);
int32_t TMPlayer_GetDevice(void);
void TMPlayer_DisplayOn(bool play);
void TMPlayer_DisplayOff(void);
void TMPlayer_AudioOn(bool background);
void TMPlayer_AudioOff(void);
void TMPlayer_VideoOn(void);
void TMPlayer_RequestDisplay(void);
void TMPlayer_GoLauncherHome(void);
void TMPlayer_GoLauncherBack(void);
void TMPlayer_GoAVOnOff(void);
void TMPlayer_NotifyAuxReady(void);
void TMPlayer_Suspend(void);
void TMPlayer_Resume(void);
#ifdef HAVE_IAP2
void TMPlayer_NotifyiAP2Ready(void);
#endif
#ifdef HAVE_BT
void TMPlayer_NotifyBTReady(void);
#endif
void TMPlayer_DetectEnable(void);
int32_t TMPlayer_RequestChangeDevice(int device, bool play);
void TMPlayer_SetPlayStatus(int32_t status);
#ifdef HAVE_BT
void TMPlayer_SetPlayStatus_BT(int32_t status);
#endif
#ifdef HAVE_IAP2
void TMPlayer_SetPlayStatus_iAP2(int32_t status);
#endif

void MetaCategoryMenuName(unsigned int dbNum, unsigned char mode, const char *menu);
void MetaCategoryIndexChange(unsigned int dbNum, unsigned int totalNum, unsigned int currentNum);
void MetaCategoryInfo(unsigned int dbNum, unsigned short index, const char *name, unsigned int type);
void MetaSelectTrack(unsigned int dbNum, unsigned int selectIndex);

#ifdef __cplusplus
}
#endif

#endif
