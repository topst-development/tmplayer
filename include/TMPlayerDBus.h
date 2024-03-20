/****************************************************************************************
 *   FileName    : TMPlayerDBus.h
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
#ifndef _TMPLAYER_DBUS_H_
#define _TMPLAYER_DBUS_H_


#ifdef __cplusplus
extern "C" {
#endif

void TMPlayerDBusInitialize(void);
void TMPlayerDBusRelease(void);



/************************** UI Send Signal ****************************************/
void SendDBusContentChanged(int32_t contentType);
void SendDBusDeviceChanged(int32_t device, int32_t contentType); 
void SendDBusDeviceConnect(int32_t device);
void SendDBusDeviceDisconnect(int32_t device);
void SendDBusDeviceMountStop(int32_t device);
void SendDBusDeviceReady(int32_t device);
void SendDBusFileDBCompleted(int32_t device);
void SendDBusFileList(int32_t device, int32_t content, int32_t number, char ** fileList, int32_t count);
void SendDBusFileListChanged(int32_t device, int32_t content, int32_t number, int32_t totalnumber);
void SendDBusFileNameChanged(const char* FileName, uint32_t length);
void SendDBusFileNumberChanged(int32_t FileNumber);
void SendDBusFileNumberUpdated(int32_t device, int32_t content, int32_t number, int32_t totalnumber);
void SendDBusFolderNameChanged(const char* FolderName, uint32_t length);
void SendDBusiD3Infomation(int32_t category, const char* info);
void SendDBusAlbumArtKey(int32_t key, uint32_t size);
void SendDBusAlbumArtCompleted(int32_t number, uint32_t length);
void SendDBusAlbumArtShmData(const char *uri, uint32_t size);
void SendDBusNoSong(int32_t device);
void SendDBusNoDevice(void);
void SendDBusMetaAddInfo(uint32_t dbNum, int32_t idx, const char *name, uint32_t type);
void SendDBusMetaCountChagned(uint32_t dbNum, uint32_t totalNum);
void SendDBusMetaCreateTrackList(uint32_t dbNum, int32_t count);
void SendDBusMetaMenuChanged(uint32_t dbNum, uint8_t mode, const char *menu);
void SendDBusRepeatModeChanged(uint32_t repeatMode);
void SendDBusShuffleModeChanged(uint32_t shuffleMode);
void SendDBusPlayStateChanged(int32_t status);
void SendDBusPlayFailed(int32_t code);
void SendDBusSamplerate(int32_t samplerate);
void SendDBusPlayTimeChanged(uint8_t hour, uint8_t min, uint8_t sec);
void SendDBusPlaySeekCompleted(void);
void SendDBusResetInfomation(void);
void SendDBusTotalNumberChanged(int32_t device, int32_t content, int32_t number);
void SendDBusTotalTimeChanged(uint8_t hour, uint8_t min, uint8_t sec);
void SendDBusDisplayOn(void);
void SendDBusDisplayOff(void);

#ifdef __cplusplus
}
#endif



#endif
