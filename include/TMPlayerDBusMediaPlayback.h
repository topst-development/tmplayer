/****************************************************************************************
 *   FileName    : TMPlayerDBusMediaPlayback.h
 *   Description : TMPlayerDBusMediaPlayback.h
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
#ifndef TMPLAYER_DBUS_MEDIAPLAYBACK_H_
#define TMPLAYER_DBUS_MEDIAPLAYBACK_H_

#ifdef __cplusplus
extern "C" {
#endif

void MediaPlaybackSignalDBusProcess(uint32_t id, DBusMessage *message);

/************************** Media Playback Daemon Method Call ****************************************/
int32_t SendDBusMediaStart(int32_t content, const char * path, uint8_t hour, uint8_t min, uint8_t sec, int32_t playID, char pause);
int32_t SendDBusMediaStop(int32_t playID);
int32_t SendDBusMediaPause(int32_t playID);
int32_t SendDBusMediaResume(int32_t playID);
int32_t SendDBusMediaSeek(uint8_t hour, uint8_t min, uint8_t sec, int32_t playID);
void SendDBusMediaGetAlbumArtKey();
int32_t SendDBusMediaGetStatus(void);
void SendDBusMediaSetDisplay(int32_t x, int32_t y, int32_t width, int32_t height);
void SendDBusMediaSetDualDisplay(int32_t x, int32_t y, int32_t width, int32_t height);
int32_t SendDBusMediaGetCurrentPlayID(void);
void NotifyMediaReleased(void);
int32_t GetPreviousID(void);
void ClearPreviousID(void);

#define MULTIMEDIA_NORMAL			0
#define MULTIMEDIA_BUSY				(-1) // Media Playback Resource is busy, need restarting media

#ifdef __cplusplus
}
#endif

#endif
