/****************************************************************************************
 *   FileName    : TMPlayeriAP2.h
 *   Description : TMPlayeriAP2.h
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

#ifndef TMPLAYER_IAP2_H
#define TMPLAYER_IAP2_H

#ifdef __cplusplus
extern "C" {
#endif

void TMPlayeriAP2_Initialize(void);
void TMPlayeriAP2_Release(void);

void TMPlayeriAP2_PlayStart(void);
void TMPlayeriAP2_PlayResume(void);
void TMPlayeriAP2_PlayRestart(void);
void TMPlayeriAP2_PlayStop(void);
void TMPlayeriAP2_PlayPause(void);
void TMPlayeriAP2_TrackUp(void);
void TMPlayeriAP2_TrackDown(void);
void TMPlayeriAP2_TrackSeek(unsigned char hour, unsigned char min, unsigned char sec);
void TMPlayeriAP2_TrackForward(void);
void TMPlayeriAP2_TrackRewind(void);
void TMPlayeriAP2_TrackFFREWEnd(void);
void TMPlayeriAP2_ChangeRepeat(void);
void TMPlayeriAP2_ChangeShuffle(void);
void TMPlayeriAP2_MetaBrowsingStart(unsigned int listcount);
void TMPlayeriAP2_MetaBrowsingUp(int index);
void TMPlayeriAP2_MetaBrowsingDown(int index);
void TMPlayeriAP2_MetaBrowsingSelect(int index);
void TMPlayeriAP2_MetaBrowsingHome(unsigned int listcount);
void TMPlayeriAP2_MetaBrowsingUndo(void);
void TMPlayeriAP2_MetaBrowsingEnd(int index);

void TMPlayeriAP2_RefreshInfomation(void);
void TMPlayeriAP2_ClearInformation(void);

#ifdef __cplusplus
}
#endif

#endif


