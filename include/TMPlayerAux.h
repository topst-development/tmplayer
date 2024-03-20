/****************************************************************************************
 *   FileName    : TMPlayerAux.h
 *   Description : TMPlayerAux.h
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

#ifndef TMPLAYER_AUX_H
#define TMPLAYER_AUX_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*AuxConnect_cb)(char *key);
typedef void (*AuxDisconnect_cb)(char *key);


int AuxPlayInit(void (*connect_cb)(char *key),void (*disconnect_cb)(char *key));
void AuxPlayDeinit(void);

int AuxPlayOn(void);
int AuxPlayOff(void);
void AuxDetectEnable(void);


#ifdef __cplusplus
}
#endif

#endif

