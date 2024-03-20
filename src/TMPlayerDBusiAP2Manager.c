/****************************************************************************************
 *	 FileName	 : TMPlayerDBusiAP2Manager.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <dbus/dbus.h>
#include <stdint.h>

#include "DBusMsgDef.h"
#include "DBusMsgDef_iAP2.h"
#include "TCDBusRawAPI.h"
#include "TCiAP2Manager.h"
#include "TMPlayerDBusiAP2Manager.h"

extern int g_tc_debug;
#define DEBUG_TMPLAYER_DBUS_IAP2_PRINTF(format, arg...) \
	if (g_tc_debug) \
	{ \
		fprintf(stderr, "[TM PLAYER DBUS iAP2 MANAGER] %s: " format "", __FUNCTION__, ##arg); \
	}

void SendDBusiAP1Initialize(void)
{
	DBusMessage *message;
	DEBUG_TMPLAYER_DBUS_IAP2_PRINTF("\n");
	
	message = CreateDBusMsgMethodCall(ADM_DBUS_NAME, ADM_OBJECT_PATH,
									  ADM_MANAGER_INTERFACE,
									  METHOD_ADM_IAP1_INITIALIZE,
									  DBUS_TYPE_INVALID);
	if (message != NULL)
	{
		if (!SendDBusMessage(message, NULL))
		{
			fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}

		dbus_message_unref(message);
	}
	else
	{
		fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}
}

void SendDBusiAP2Initialize(void)
{
	DBusMessage *message;
	DEBUG_TMPLAYER_DBUS_IAP2_PRINTF("\n");
	
	message = CreateDBusMsgMethodCall(ADM_DBUS_NAME, ADM_OBJECT_PATH,
									  ADM_MANAGER_INTERFACE,
									  METHOD_ADM_IAP2_INITIALIZE,
									  DBUS_TYPE_INVALID);
	if (message != NULL)
	{
		if (!SendDBusMessage(message, NULL))
		{
			fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}

		dbus_message_unref(message);
	}
	else
	{
		fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}
}

void SendDBusiAP1Release(void)
{
	DBusMessage *message;
	DEBUG_TMPLAYER_DBUS_IAP2_PRINTF("\n");
	
	message = CreateDBusMsgMethodCall(ADM_DBUS_NAME, ADM_OBJECT_PATH,
									  ADM_MANAGER_INTERFACE,
									  METHOD_ADM_IAP1_RELEASE,
									  DBUS_TYPE_INVALID);
	if (message != NULL)
	{
		if (!SendDBusMessage(message, NULL))
		{
			fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}

		dbus_message_unref(message);
	}
	else
	{
		fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}
}

void SendDBusiAP2Release(void)
{
	DBusMessage *message;
	DEBUG_TMPLAYER_DBUS_IAP2_PRINTF("\n");
	
	message = CreateDBusMsgMethodCall(ADM_DBUS_NAME, ADM_OBJECT_PATH,
									  ADM_MANAGER_INTERFACE,
									  METHOD_ADM_IAP2_RELEASE,
									  DBUS_TYPE_INVALID);
	if (message != NULL)
	{
		if (!SendDBusMessage(message, NULL))
		{
			fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
		}

		dbus_message_unref(message);
	}
	else
	{
		fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
	}
}

void SendDBusChangePlayer(void)
{
    DEBUG_TMPLAYER_DBUS_IAP2_PRINTF("%s\n", __FUNCTION__);

    DBusMessage *message = CreateDBusMsgMethodCall(IAP2_FACILITY_DEST_NAME, IAP2_FACILITY_OBJECT_PATH,
                                                                            IAP2_FACILITY_FEATURE_INTERFACE,
                                                                            METHOD_PLAYER_IAP2_CHANGED_PLAYER,
                                                                            DBUS_TYPE_INVALID);
    if (message != NULL)
    {
        DBusPendingCall *pending = NULL;
        if (SendDBusMessage(message, &pending))
        {
            if (pending != NULL)
            {
                if (GetArgumentFromDBusPendingCall(pending, DBUS_TYPE_INVALID))
                {
                    ;
                }
                else
                {
                    fprintf(stderr, "%s: SendDBusMessage no response \n", __FUNCTION__);
                }
            }
            else
            {
                fprintf(stderr, "%s: SendDBusMessage pending is null \n", __FUNCTION__);
            }
        }
        else
        {
            fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
        }
        dbus_message_unref(message);
    }
    else
    {
        fprintf(stderr, "%s: CreateDBusMsgMethodCall failed\n", __FUNCTION__);
    }
}

void DBusMethodiAP2Notify(DBusMessage *message)
{
    DEBUG_TMPLAYER_DBUS_IAP2_PRINTF("\n");

    if (message != NULL)
    {
        iAP2NotifyFromProcess(message);
    }
    else
    {
        fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
    }
}

void DBusMethodPlayerLogEnable(DBusMessage *message)
{
    if (message != NULL)
    {
        dbus_bool_t enable = FALSE;
        if (GetArgumentFromDBusMessage(message,
                                        DBUS_TYPE_BOOLEAN, &enable,
										DBUS_TYPE_INVALID))
        {
            iAP2LogEnable(enable);
            DEBUG_TMPLAYER_DBUS_IAP2_PRINTF(" %s\n", (enable == 0) ? "false" : "true");
        }
    }
    else
    {
        fprintf(stderr, "%s: mesage is NULL\n", __FUNCTION__);
    }
}

static void DBusMethodSendReply(DBusMessage *returnMessage)
{
    DEBUG_TMPLAYER_DBUS_IAP2_PRINTF("[DBUS METHOD] %s: return message[%p]\n", __FUNCTION__, returnMessage);

    if (returnMessage != NULL)
    {
        if (!SendDBusMessage(returnMessage, NULL))
        {
            DEBUG_TMPLAYER_DBUS_IAP2_PRINTF("%s: fail SendDBusMessage: d-bus system Error\n", __FUNCTION__);
        }
        dbus_message_unref(returnMessage);
    }
    else
    {
        DEBUG_TMPLAYER_DBUS_IAP2_PRINTF("%s: fail assembling returnMessage: d-bus system Error\n", __FUNCTION__);
    }
}

void DBusMethodChangePlayer(DBusMessage *message)
{
    DEBUG_TMPLAYER_DBUS_IAP2_PRINTF("%s\n", __FUNCTION__);

    if(message != NULL)
    {
        DBusMessage *replyMessage = CreateDBusMsgMethodReturn(message, DBUS_TYPE_INVALID);
        if (replyMessage != NULL)
        {
            DBusMethodSendReply(replyMessage);
        }
        else
        {
            fprintf(stderr, "%s replyMessage is NULL\n", __FUNCTION__);
        }
    }
}

void AppleDeviceManagerSignalDBusProcess(unsigned int id, DBusMessage *message)
{
    if (id < TotalSignalAppleDeviceManagerEvents)
    {
        DEBUG_TMPLAYER_DBUS_IAP2_PRINTF("RECEIVED SIGNAL[%s(%d)]\n", g_signalADMEventNames[id], id);

        if (message != NULL)
        {
            int ret = 0;
            int fWithEaNative = 0;
            const char *serial = NULL;
            switch(id)
            {
                case SignalAppleDeviceManageriAP2Connected:
                case SignalAppleDeviceManagerCarPlayConnected:
                    ret = GetArgumentFromDBusMessage(message, DBUS_TYPE_INT32, &fWithEaNative, DBUS_TYPE_STRING, &serial, DBUS_TYPE_INVALID);
                    if (ret)
                    {
                        DEBUG_TMPLAYER_DBUS_IAP2_PRINTF("%s serial %s\n", __FUNCTION__, (char *) serial);
                    }
                    else
                    {
                        fprintf(stderr, "ERROR: %s, check if the adm version is greater than 1.2.5\n", __FUNCTION__);
                    }
                    break;
                default:
                    ret = GetArgumentFromDBusMessage(message, DBUS_TYPE_INVALID);
                    break;
            }
            
            if (ret)
            {
                switch (id)
                {
                    case SignalAppleDeviceManageriAP2Connected:
                        iAP2EventConnected();
                        break;
                    case SignalAppleDeviceManageriAP2Disconnected:
                        iAP2EventDisconnected();
                        break;
                    case SignalAppleDeviceManagerCarPlayConnected:
                        break;
                    case SignalAppleDeviceManagerCarPlayDisconnected:
                        break;    
                    default:
                        fprintf(stderr, "%s: unknown signal id(%d)\n", __FUNCTION__, id);
                        break;
                }
            }
            else
            {
                fprintf(stderr, "%s: GetArgumentFromDBusMessage failed\n", __FUNCTION__);
            }

        }
    }
    else
    {
        DEBUG_TMPLAYER_DBUS_IAP2_PRINTF("RECEIVED SIGNAL[%d]\n", id);
    }
}


