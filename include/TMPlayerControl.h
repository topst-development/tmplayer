/****************************************************************************************
 *   FileName    : TMPlayerContorol.h
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
#ifndef _TMPLAYER_CONTROL_H_
#define _TMPLAYER_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

void TMPlayerControl_Play(MultiMediaContentType content, const char *path, unsigned char hour, unsigned char min, unsigned char sec, int playID, char pause);
void TMPlayerControl_Stop(int playID);
void TMPlayerControl_MediaRelease(int playID);
void TMPlayerControl_Resume(int playID);
void TMPlayerControl_Pause(int playID);
void TMPlayerControl_Seek(unsigned char hour, unsigned char min, unsigned char sec, int playID);
void TMPlayerControl_SetDisplay(int x, int y, int witdh, int height);
void TMPlayerControl_SetDualDisplay(int x, int y, int witdh, int height);

#ifdef __cplusplus
}
#endif


#endif
