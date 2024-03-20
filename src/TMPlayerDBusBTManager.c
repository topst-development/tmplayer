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
#include "DBusMsgDef_BT.h"
#include "TCDBusRawAPI.h"
#include "TMPlayerBT.h"
#include "TMPlayerDBusBTManager.h"

extern int g_tc_debug;
#define DEBUG_TMPLAYER_DBUS_BT_PRINTF(format, arg...) \
	if (g_tc_debug) \
	{ \
		fprintf(stderr, "[TM PLAYER DBUS BT MANAGER] %s: " format "", __FUNCTION__, ##arg); \
	}

void SendDBusBluetoothTrackStart(void)
{
    DEBUG_TMPLAYER_DBUS_BT_PRINTF("%s\n", __FUNCTION__);

    DBusMessage *message = CreateDBusMsgMethodCall(BLUETOOTH_APPLICATION_DBUS_NAME, 
    												BLUETOOTH_APPLICATION_OBJECT_PATH,
                                                    BLUETOOTH_TMP_MANAGER_INTERFACE,
                                                    METHOD_BLUETOOTH_TMP_TRACK_START,
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

void SendDBusBluetoothTrackStop(void)
{
    DEBUG_TMPLAYER_DBUS_BT_PRINTF("%s\n", __FUNCTION__);

    DBusMessage *message = CreateDBusMsgMethodCall(BLUETOOTH_APPLICATION_DBUS_NAME, 
    												BLUETOOTH_APPLICATION_OBJECT_PATH,
                                                    BLUETOOTH_TMP_MANAGER_INTERFACE,
                                                    METHOD_BLUETOOTH_TMP_TRACK_STOP,
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

void SendDBusBluetoothTrackResume(void)
{
    DEBUG_TMPLAYER_DBUS_BT_PRINTF("%s\n", __FUNCTION__);

    DBusMessage *message = CreateDBusMsgMethodCall(BLUETOOTH_APPLICATION_DBUS_NAME, 
    												BLUETOOTH_APPLICATION_OBJECT_PATH,
                                                    BLUETOOTH_TMP_MANAGER_INTERFACE,
                                                    METHOD_BLUETOOTH_TMP_TRACK_RESUME,
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

void SendDBusBluetoothTrackPause(void)
{
    DEBUG_TMPLAYER_DBUS_BT_PRINTF("%s\n", __FUNCTION__);

    DBusMessage *message = CreateDBusMsgMethodCall(BLUETOOTH_APPLICATION_DBUS_NAME, 
    												BLUETOOTH_APPLICATION_OBJECT_PATH,
                                                    BLUETOOTH_TMP_MANAGER_INTERFACE,
                                                    METHOD_BLUETOOTH_TMP_TRACK_PAUSE,
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

void SendDBusBluetoothTrackUp(void)
{
    DEBUG_TMPLAYER_DBUS_BT_PRINTF("%s\n", __FUNCTION__);

    DBusMessage *message = CreateDBusMsgMethodCall(BLUETOOTH_APPLICATION_DBUS_NAME, 
    												BLUETOOTH_APPLICATION_OBJECT_PATH,
                                                    BLUETOOTH_TMP_MANAGER_INTERFACE,
                                                    METHOD_BLUETOOTH_TMP_TRACK_UP,
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

void SendDBusBluetoothTrackDown(void)
{
    DEBUG_TMPLAYER_DBUS_BT_PRINTF("%s\n", __FUNCTION__);

    DBusMessage *message = CreateDBusMsgMethodCall(BLUETOOTH_APPLICATION_DBUS_NAME, 
    												BLUETOOTH_APPLICATION_OBJECT_PATH,
                                                    BLUETOOTH_TMP_MANAGER_INTERFACE,
                                                    METHOD_BLUETOOTH_TMP_TRACK_DOWN,
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

void SendDBusBluetoothForward(void)
{
    DEBUG_TMPLAYER_DBUS_BT_PRINTF("%s\n", __FUNCTION__);

    DBusMessage *message = CreateDBusMsgMethodCall(BLUETOOTH_APPLICATION_DBUS_NAME,
    												BLUETOOTH_APPLICATION_OBJECT_PATH,
                                                    BLUETOOTH_TMP_MANAGER_INTERFACE,
                                                    METHOD_BLUETOOTH_TMP_TRACK_FF,
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

void SendDBusBluetoothRewind(void)
{
    DEBUG_TMPLAYER_DBUS_BT_PRINTF("%s\n", __FUNCTION__);

    DBusMessage *message = CreateDBusMsgMethodCall(BLUETOOTH_APPLICATION_DBUS_NAME,
    												BLUETOOTH_APPLICATION_OBJECT_PATH,
                                                    BLUETOOTH_TMP_MANAGER_INTERFACE,
                                                    METHOD_BLUETOOTH_TMP_TRACK_REW,
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

void SendDBusBluetoothFFREWEnd(void)
{
    DEBUG_TMPLAYER_DBUS_BT_PRINTF("%s\n", __FUNCTION__);

    DBusMessage *message = CreateDBusMsgMethodCall(BLUETOOTH_APPLICATION_DBUS_NAME,
    												BLUETOOTH_APPLICATION_OBJECT_PATH,
                                                    BLUETOOTH_TMP_MANAGER_INTERFACE,
                                                    METHOD_BLUETOOTH_TMP_TRACK_FFREWEND,
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

void SendDBusBluetoothChangeRepeat(void)
{
    DEBUG_TMPLAYER_DBUS_BT_PRINTF("%s\n", __FUNCTION__);

    DBusMessage *message = CreateDBusMsgMethodCall(BLUETOOTH_APPLICATION_DBUS_NAME, 
    												BLUETOOTH_APPLICATION_OBJECT_PATH,
                                                    BLUETOOTH_TMP_MANAGER_INTERFACE,
                                                    METHOD_BLUETOOTH_TMP_CHANGE_REPEAT,
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

void SendDBusBluetoothChangeShuffle(void)
{
    DEBUG_TMPLAYER_DBUS_BT_PRINTF("%s\n", __FUNCTION__);

    DBusMessage *message = CreateDBusMsgMethodCall(BLUETOOTH_APPLICATION_DBUS_NAME, 
    												BLUETOOTH_APPLICATION_OBJECT_PATH,
                                                    BLUETOOTH_TMP_MANAGER_INTERFACE,
                                                    METHOD_BLUETOOTH_TMP_CHANGE_SHUFFLE,
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

void SendDBusBluetoothBrowsingType(unsigned int list_type)
{
    DEBUG_TMPLAYER_DBUS_BT_PRINTF("%s type: %d\n", __FUNCTION__, list_type);

    DBusMessage *message = CreateDBusMsgMethodCall(BLUETOOTH_APPLICATION_DBUS_NAME,
    												BLUETOOTH_APPLICATION_OBJECT_PATH,
                                                    BLUETOOTH_TMP_MANAGER_INTERFACE,
                                                    METHOD_BLUETOOTH_TMP_BROWSING_TYPE,
													DBUS_TYPE_UINT32, &list_type,
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

bool SendDBusBluetoothBrowsingList(unsigned int number, unsigned int *count)
{
	unsigned int ret_cnt_d = 0;
	unsigned int end = number + *count - 1;
	dbus_bool_t ret_d = FALSE;
	bool ret = false;

    DEBUG_TMPLAYER_DBUS_BT_PRINTF("%s number: %d, count: %d, end: %d\n", __FUNCTION__, number, *count, end);

    DBusMessage *message = CreateDBusMsgMethodCall(BLUETOOTH_APPLICATION_DBUS_NAME,
    												BLUETOOTH_APPLICATION_OBJECT_PATH,
                                                    BLUETOOTH_TMP_MANAGER_INTERFACE,
                                                    METHOD_BLUETOOTH_TMP_BROWSING_LIST,
													DBUS_TYPE_UINT32, &number,
													DBUS_TYPE_UINT32, &end,
                                                    DBUS_TYPE_INVALID);
    if (message != NULL)
    {
		DBusPendingCall *pending = NULL;
		if (SendDBusMessage(message, &pending))
		{
			if (pending != NULL)
			{
				if (GetArgumentFromDBusPendingCall(pending,
												DBUS_TYPE_BOOLEAN, &ret_d,
												DBUS_TYPE_UINT32, &ret_cnt_d,
												DBUS_TYPE_INVALID))
				{
					if (ret_d == TRUE)
					{
						ret = true;
						*count = ret_cnt_d;
					}
					DEBUG_TMPLAYER_DBUS_BT_PRINTF("%s: GetArgmentFromDBusPendingCall, ret: %s, %d\n",
															__FUNCTION__, ret ? "true" : "false", *count);
				}
				else
				{
					fprintf(stderr, "%s: SendDBusMessage failed\n", __FUNCTION__);
				}
				dbus_pending_call_unref(pending);
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

	return ret;
}

void SendDBusBluetoothBrowsingPlay(unsigned int number, unsigned int uid1, unsigned int uid2)
{
    DEBUG_TMPLAYER_DBUS_BT_PRINTF("Track No: %d\n", number);

    DBusMessage *message = CreateDBusMsgMethodCall(BLUETOOTH_APPLICATION_DBUS_NAME,
    												BLUETOOTH_APPLICATION_OBJECT_PATH,
                                                    BLUETOOTH_TMP_MANAGER_INTERFACE,
                                                    METHOD_BLUETOOTH_TMP_BROWSING_PLAY,
													DBUS_TYPE_UINT32, &number,
													DBUS_TYPE_UINT32, &uid1,
													DBUS_TYPE_UINT32, &uid2,
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

void SendDBusBluetoothBrowsingChange(uint32_t direction, uint32_t index, uint32_t uid1, uint32_t uid2)
{
    DEBUG_TMPLAYER_DBUS_BT_PRINTF("Direction: %d, Index: %d, UID: %x %x\n", direction, index, uid1, uid2);

    DBusMessage *message = CreateDBusMsgMethodCall(BLUETOOTH_APPLICATION_DBUS_NAME,
    												BLUETOOTH_APPLICATION_OBJECT_PATH,
                                                    BLUETOOTH_TMP_MANAGER_INTERFACE,
                                                    METHOD_BLUETOOTH_TMP_BROWSING_CHANGE,
													DBUS_TYPE_UINT32, &direction,
													DBUS_TYPE_UINT32, &index,
													DBUS_TYPE_UINT32, &uid1,
													DBUS_TYPE_UINT32, &uid2,
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

void BluetoothSignalDBusProcess(unsigned int id, DBusMessage *message)
{
    if (id < TotalSignalBTEvents)
    {
        //DEBUG_TMPLAYER_DBUS_BT_PRINTF("RECEIVED SIGNAL[%s(%d)]\n", g_signalBTEventNames[id], id);

        if (message != NULL)
        {
            switch(id)
            {
                case SignalBTEventConnected:
                {
					GetArgumentFromDBusMessage(message, DBUS_TYPE_INVALID);
					TMPlayerBT_Connected();
				}
				break;
                case SignalBTEventDisconnected:
                {
					GetArgumentFromDBusMessage(message, DBUS_TYPE_INVALID);
					TMPlayerBT_Disconnected();
				}
				break;
                case SignalBTEventPlayTimeChanged:
				{
					uint32_t time;

					GetArgumentFromDBusMessage(message, DBUS_TYPE_UINT32, &time, DBUS_TYPE_INVALID);
					TMPlayerBT_PlayTimeChange(time);
				}
				break;
                case SignalBTEventTotalTimeChanged:
				{
					uint32_t totaltime;

					GetArgumentFromDBusMessage(message, DBUS_TYPE_UINT32, &totaltime, DBUS_TYPE_INVALID);
					TMPlayerBT_TotalTimeChange(totaltime);
				}
				break;
                case SignalBTEventTitleNameChanged:
				{
					uint32_t length;	const char *title;

					GetArgumentFromDBusMessage(message, DBUS_TYPE_STRING, &title, DBUS_TYPE_UINT32, &length, DBUS_TYPE_INVALID);
					TMPlayerBT_TitleInfo(title, length);
				}
				break;
                case SignalBTEventArtistNameChanged:
				{
					uint32_t length;	const char *artist;

					GetArgumentFromDBusMessage(message, DBUS_TYPE_STRING, &artist, DBUS_TYPE_UINT32, &length, DBUS_TYPE_INVALID);
					TMPlayerBT_ArtistInfo(artist, length);
				}
				break;
                case SignalBTEventAlbumNameChanged:
				{
					uint32_t length;	const char *album;

					GetArgumentFromDBusMessage(message, DBUS_TYPE_STRING, &album, DBUS_TYPE_UINT32, &length, DBUS_TYPE_INVALID);
					TMPlayerBT_AlbumInfo(album, length);
				}
				break;
                case SignalBTEventAlbumArtSupport:
				{
					dbus_bool_t support;	const char *uri;

					GetArgumentFromDBusMessage(message, DBUS_TYPE_BOOLEAN, &support, DBUS_TYPE_STRING, &uri, DBUS_TYPE_INVALID);

					if (support == TRUE)
					{
						TMPlayerBT_AlbumArtSupport(true, uri);
					}
					else
					{
						TMPlayerBT_AlbumArtSupport(false, uri);
					}
				}
				break;
                case SignalBTEventAlbumArtChanged:
				{
					uint32_t length;

					GetArgumentFromDBusMessage(message, DBUS_TYPE_UINT32, &length, DBUS_TYPE_INVALID);
					TMPlayerBT_AlbumArt(length);
				}
				break;
                case SignalBTEventPlayStatusChanged:
				{
					uint32_t playstatus;

					GetArgumentFromDBusMessage(message, DBUS_TYPE_UINT32, &playstatus, DBUS_TYPE_INVALID);
					TMPlayerBT_PlayStatusChange(playstatus);
				}
				break;
				case SignalBTEventBrowsingSupport:
				{
					dbus_bool_t support;	uint32_t list_type;

					GetArgumentFromDBusMessage(message, DBUS_TYPE_BOOLEAN, &support, DBUS_TYPE_UINT32, &list_type, DBUS_TYPE_INVALID);

					if (support == TRUE)
					{
						TMPlayerBT_BrowsingSupport(true, list_type);
					}
					else
					{
						TMPlayerBT_BrowsingSupport(false, list_type);
					}

				}
				break;
				case SignalBTEventBrowsingList:
				{
					uint32_t idx;		uint32_t *uid1, *uid2;	int32_t *types;		const char **names;
					uint32_t cnt1, cnt2, cnt3, cnt4;

					GetArgumentFromDBusMessage(message, DBUS_TYPE_UINT32, &idx,
												DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &uid1, &cnt1,
												DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &uid2, &cnt2,
												DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32, &types, &cnt3,
												DBUS_TYPE_ARRAY, DBUS_TYPE_STRING, &names, &cnt4,
												DBUS_TYPE_INVALID);
					if (cnt1 == cnt2)
					{
						TMPlayerBT_BrowsingList(idx, uid1, uid2, types, names, cnt1);
					}
					else
					{
						fprintf(stderr, "%s: Invalid counts (%d:%d)\n", __FUNCTION__, cnt1, cnt2);
					}
				}
				break;
				case SignalBTEventBrowsingChanged:
				{
					GetArgumentFromDBusMessage(message, DBUS_TYPE_INVALID);
					TMPlayerBT_BrowsingChanged();
				}
				break;
                case SignalBTEventTrackNumberChanged:
				{
					uint32_t tracknum;

					GetArgumentFromDBusMessage(message, DBUS_TYPE_UINT32, &tracknum, DBUS_TYPE_INVALID);
					TMPlayerBT_TrackChange(tracknum);
				}
				break;
                case SignalBTEventTotalNumberChanged:
				{
					uint32_t totalnum;

					GetArgumentFromDBusMessage(message, DBUS_TYPE_UINT32, &totalnum, DBUS_TYPE_INVALID);
					TMPlayerBT_TotalTrackChange(totalnum);
				}
				break;
                case SignalBTEventRepeatChanged:
				{
					uint32_t repeat;

					GetArgumentFromDBusMessage(message, DBUS_TYPE_UINT32, &repeat, DBUS_TYPE_INVALID);
					TMPlayerBT_RepeatChange(repeat);
				}
				break;
                case SignalBTEventShuffleChanged:
				{
					uint32_t shuffle;

					GetArgumentFromDBusMessage(message, DBUS_TYPE_UINT32, &shuffle, DBUS_TYPE_INVALID);
					TMPlayerBT_ShuffleChange(shuffle);
				}
				break;
                default:
                    GetArgumentFromDBusMessage(message, DBUS_TYPE_INVALID);
                    fprintf(stderr, "%s: unknown signal id(%d)\n", __FUNCTION__, id);
	                break;
            }
        }
    }
    else
    {
        DEBUG_TMPLAYER_DBUS_BT_PRINTF("RECEIVED SIGNAL[%d]\n", id);
    }
}
