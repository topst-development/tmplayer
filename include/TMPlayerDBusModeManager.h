/****************************************************************************************
 *   FileName    : TMPlayerDBusModeManager.h
 *   Description : 
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

#ifndef _TMPLAYER_DBUS_MODEMANAGER_H_
#define _TMPLAYER_DBUS_MODEMANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

void ReleaseAllResource(int status);

void ModeManagerSignalDBusProcess(unsigned int index, DBusMessage *message);

int SendDBusModeManagerGoHome(void);
int SendDBusModeManagerGoBack(void);

void SendDBusModeManagerRscReleased(int rsc);
int SendDBusModeManagerChangeMode(const char* mode);
void SendDBusModeManagerReleaseResourceDone(void);
void SendDBusModeManagerEndMode(const char *mode);
void SendDBusModeManagerErrorOccur(void);

#define LAUNCHERAPPID									0

#define IDLE											"idle"
#define VIEW											"view"
#define AUDIOPLAY										"audioplay"
#define AUDIOBGPLAY										"audioplaybg"
#define VIDEOPLAY										"videoplay"

#define RSC_NONE										0x0000
#define RSC_DISPLAY										0x0001
#define RSC_AUDIO										0x0002

#define DO_NOTHING						0
#define DO_RELEASE_RESOURCE				1
#define DO_END_MODE_AUDIO				2
#define DO_END_MODE_AUDIO_BG			3
#define DO_END_MODE_VIDEO				4
#define DO_IDLE							5
#define DO_SUSPEND						6

#ifdef __cplusplus
}
#endif

#endif
