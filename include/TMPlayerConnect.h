/****************************************************************************************
 *   FileName    : TMPlayerConnect.h
 *   Description : TMPlayerConnect.h
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
#ifndef TMPLAYER_CONNECT_H_
#define TMPLAYER_CONNECT_H_

#ifndef NULL
#define NULL 0
#endif

#ifdef __cplusplus
extern "C" {
#endif	

typedef void (*TMPlayerConnectDeviceConnected_cb)(char* key);
typedef void (*TMPlayerConnectDeviceDisConnected_cb)(char* key);
typedef void (*TMPlayerConnectDeviceMounted_cb)(char* key, char* mountpath);
typedef void (*TMPlayerConnectDeviceMountErrorOccured_cb)(char* key);

typedef struct TCConnect_DeviceEventCB{
	TMPlayerConnectDeviceConnected_cb			TMPlayerConnectDeviceConnected;
	TMPlayerConnectDeviceDisConnected_cb		TMPlayerConnectDeviceDisConnected;
	TMPlayerConnectDeviceMounted_cb			TMPlayerConnectDeviceMounted;
	TMPlayerConnectDeviceMountErrorOccured_cb	TMPlayerConnectDeviceMountErrorOccured;
}TMPlayerConnect_DeviceEventCB;

int32_t TMPlayerConnect_Initialize(void);
void TMPlayerConnect_Release(void);
void TMPlayerConnect_Debug(int32_t state);
void TMPlayerConnect_SetEventCallback(TMPlayerConnect_DeviceEventCB *cb);

#ifdef __cplusplus
}
#endif	

#endif

