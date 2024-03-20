/****************************************************************************************
 *	 FileName	 : TCiAP2FeatureHid.cpp
 *	 Description : 
 ****************************************************************************************
 *
 *	 TCC Version 1.0
 *	 Copyright (c) Telechips Inc.
 *	 All rights reserved 
 
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <dbus/dbus.h>
#include <vector>
#include "iAP2Const.h"
#include "DBusMsgDef_iAP2.h"
#include "iAP2NativeMsg.h"
#include "TCDBusRawAPI.h"
#include "TCiAP2Manager.h"
#include "TCiAP2FeatureHid.h"

extern int g_debug;
#define IAP2_FEATURE_HID_PRINTF(format, arg...) \
    if (g_debug) \
    { \
        fprintf(stderr, "[iAP2 Feature HID] %s: " format "", __FUNCTION__, ##arg); \
    }
    
iAP2Hid *iAP2Hid::instance;
iAP2Hid::iAP2Hid()
{

}

iAP2Hid::~iAP2Hid()
{

}

iAP2Hid *iAP2Hid::getInstance() 
{
    if (instance == NULL)
    {
        instance = new iAP2Hid();
    }
    return instance;
}

void iAP2Hid::sendHidStart()
{
    iAP2NativeMsg *msg = NULL;
    uint8_t *payload = NULL;

    IAP2_FEATURE_HID_PRINTF("%s\n",__func__);
    
    msg = new iAP2NativeMsg();
    msg->addIntItem(hid_playback_ComponentType,IAP2_PARAM_ENUM,hid_component_MediaPlaybackRemote);
    payload = msg->getRawNativeMsg();
    if(payload != NULL)
    {
        SendDBusiAP2MethodCall(METHOD_iAP2_HID_START, (void *)payload);
        free(payload);
    }
    delete msg;
}

void iAP2Hid::sendHidReport(int key)
{
    iAP2NativeMsg *msg = NULL;
    uint8_t *payload = NULL;

    IAP2_FEATURE_HID_PRINTF("%s key %d \n",__func__,key);
    
    msg = new iAP2NativeMsg();
    msg->addIntItem(hid_playback_ComponentType,IAP2_PARAM_ENUM,hid_component_MediaPlaybackRemote);
    msg->addIntItem(hid_playback_Key,IAP2_PARAM_NUMU32,key);
    payload = msg->getRawNativeMsg();
    if(payload != NULL)
    {
        SendDBusiAP2MethodCall(METHOD_iAP2_HID_REPORT, (void *)payload);
        free(payload);
    }
    delete msg;
}

void iAP2Hid::sendHidStop()
{
    iAP2NativeMsg *msg = NULL;
    uint8_t *payload = NULL;

    IAP2_FEATURE_HID_PRINTF("%s \n",__func__);
    
    msg = new iAP2NativeMsg();
    msg->addIntItem(hid_playback_ComponentType,IAP2_PARAM_ENUM,hid_component_MediaPlaybackRemote);
    payload = msg->getRawNativeMsg();
    if(payload != NULL)
    {
        SendDBusiAP2MethodCall(METHOD_iAP2_HID_STOP, (void *)payload);
        free(payload);
    }
    delete msg;
}


