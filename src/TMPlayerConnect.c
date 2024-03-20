/****************************************************************************************
 *   FileName    : TMPlayerConnect.c
 *   Description : TMPlayerConnect.c
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

#include <stdint.h>
#include "stdio.h"
#include "TCConnectInterface.h"
#include "TMPlayerConnect.h"

extern int32_t g_tc_debug;
#define DEBUG_TMPLAYER_CONNNECT_PRINTF(format, arg...) \
	if (g_tc_debug != 0) \
	{ \
		(void)fprintf(stderr, "[TC PLAYER CONNNECT] %s: " format "", __FUNCTION__, ##arg); \
	}

static TMPlayerConnectDeviceConnected_cb		TMPlayerConnectDeviceConnected = NULL;
static TMPlayerConnectDeviceDisConnected_cb		TMPlayerConnectDeviceDisConnected = NULL;
static TMPlayerConnectDeviceMounted_cb			TMPlayerConnectDeviceMounted = NULL;
static TMPlayerConnectDeviceMountErrorOccured_cb	TMPlayerConnectDeviceMountErrorOccured = NULL;

int32_t TMPlayerConnect_Initialize(void)
{
	int32_t ret;

	DEBUG_TMPLAYER_CONNNECT_PRINTF("\n");
	TC_ConnectDebugOn(g_tc_debug);
	ret = TC_ConnectInitialize();
	TC_DeviceConnected_notify_cb(TMPlayerConnectDeviceConnected);
	TC_DeviceDisconnected_notify_cb(TMPlayerConnectDeviceDisConnected);
	TC_DeviceMounted_notify_cb(TMPlayerConnectDeviceMounted);
	TC_DeviceMountErrorOccurred_notify_cb(TMPlayerConnectDeviceMountErrorOccured);

	return ret;
}

void TMPlayerConnect_Release(void)
{
	DEBUG_TMPLAYER_CONNNECT_PRINTF("\n");
	TC_ConnectRelease();
}

void TMPlayerConnect_Debug(int32_t state)
{
	DEBUG_TMPLAYER_CONNNECT_PRINTF("%s \n", (state != 0) ? "ON" : "OFF");
	TC_ConnectDebugOn(state);
}

void TMPlayerConnect_SetEventCallback(TMPlayerConnect_DeviceEventCB *cb)
{
	DEBUG_TMPLAYER_CONNNECT_PRINTF("\n");
	TMPlayerConnectDeviceConnected = cb->TMPlayerConnectDeviceConnected;
	TMPlayerConnectDeviceDisConnected = cb ->TMPlayerConnectDeviceDisConnected;
	TMPlayerConnectDeviceMounted = cb->TMPlayerConnectDeviceMounted;
	TMPlayerConnectDeviceMountErrorOccured = cb->TMPlayerConnectDeviceMountErrorOccured;
}
