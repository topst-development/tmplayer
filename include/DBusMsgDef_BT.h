/****************************************************************************************
 *   FileName    : DBusMsgDef_BT.h
 *   Description : DBusMsgDef_BT.h
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
#ifndef DBUS_MSG_DEF_BT_H
#define DBUS_MSG_DEF_BT_H

/*================================== BLUETOOTH PROCESS DEFINE START ==================================*/
#define BLUETOOTH_APPLICATION_DBUS_NAME				"telechips.bluetooth.application"
#define BLUETOOTH_APPLICATION_OBJECT_PATH			"/telechips/bluetooth/application"
#define BLUETOOTH_APPLICATION_INTERFACE				"bluetoothapplication.event"

// FOR TMPlayer SIGNAL EVENT DEFINES
#define SIGNAL_BLUETOOTH_TMP_CONNECTED				"signal_tmp_bluetooth_connected"
#define SIGNAL_BLUETOOTH_TMP_DISCONNECTED			"signal_tmp_bluetooth_disconnected"
#define SIGNAL_BLUETOOTH_TMP_PLAYTIME_CHANGED		"signal_tmp_bluetooth_playtime_changed"
#define SIGNAL_BLUETOOTH_TMP_TOTALTIME_CHANGED		"signal_tmp_bluetooth_totaltime_changed"
#define SIGNAL_BLUETOOTH_TMP_TITLENAME_CHANGED		"signal_tmp_bluetooth_titlename_changed"
#define SIGNAL_BLUETOOTH_TMP_ARTISTNAME_CHANGED		"signal_tmp_bluetooth_artistname_changed"
#define SIGNAL_BLUETOOTH_TMP_ALBUMNAME_CHANGED		"signal_tmp_bluetooth_albumname_changed"
#define SIGNAL_BLUETOOTH_TMP_ALBUMART_SUPPORT		"signal_tmp_bluetooth_albumart_support"
#define SIGNAL_BLUETOOTH_TMP_ALBUMART_CHANGED		"signal_tmp_bluetooth_albumart_changed"
#define SIGNAL_BLUETOOTH_TMP_PLAYSTATUS_CHANGED		"signal_tmp_bluetooth_playstatus_changed"
#define SIGNAL_BLUETOOTH_TMP_BROWSING_SUPPORT		"signal_tmp_bluetooth_browsing_support"
#define SIGNAL_BLUETOOTH_TMP_BROWSING_LIST			"signal_tmp_bluetooth_browsing_list"
#define SIGNAL_BLUETOOTH_TMP_BROWSING_CHANGED		"signal_tmp_bluetooth_browsing_changed"
#define SIGNAL_BLUETOOTH_TMP_TRACKNUMBER_CHANGED	"signal_tmp_bluetooth_tracknumber_changed"
#define SIGNAL_BLUETOOTH_TMP_TOTALNUMBER_CHANGED	"signal_tmp_bluetooth_totalnumber_changed"
#define SIGNAL_BLUETOOTH_TMP_REPEAT_CHANGED			"signal_tmp_bluetooth_repeat_changed"
#define SIGNAL_BLUETOOTH_TMP_SHUFFLE_CHANGED		"signal_tmp_bluetooth_shuffle_changed"

typedef enum {
    SignalBTEventConnected,
    SignalBTEventDisconnected,
    SignalBTEventPlayTimeChanged,
    SignalBTEventTotalTimeChanged,
    SignalBTEventTitleNameChanged,
    SignalBTEventArtistNameChanged,
    SignalBTEventAlbumNameChanged,
    SignalBTEventAlbumArtSupport,
    SignalBTEventAlbumArtChanged,
    SignalBTEventPlayStatusChanged,
	SignalBTEventBrowsingSupport,
	SignalBTEventBrowsingList,
	SignalBTEventBrowsingChanged,
    SignalBTEventTrackNumberChanged,
    SignalBTEventTotalNumberChanged,
    SignalBTEventRepeatChanged,
    SignalBTEventShuffleChanged,
    TotalSignalBTEvents
} SignalBTEvent;
extern const char *g_signalBTEventNames[TotalSignalBTEvents];

// FOR TMPlayer METHOD EVENT DEFINES
#define BLUETOOTH_TMP_MANAGER_INTERFACE				"tmplayer.event"
#define METHOD_BLUETOOTH_TMP_TRACK_START			"method_tmp_bluetooth_track_start"
#define METHOD_BLUETOOTH_TMP_TRACK_STOP				"method_tmp_bluetooth_track_stop"
#define METHOD_BLUETOOTH_TMP_TRACK_RESUME			"method_tmp_bluetooth_track_resume"
#define METHOD_BLUETOOTH_TMP_TRACK_PAUSE			"method_tmp_bluetooth_track_pause"
#define METHOD_BLUETOOTH_TMP_TRACK_UP				"method_tmp_bluetooth_track_up"
#define METHOD_BLUETOOTH_TMP_TRACK_DOWN				"method_tmp_bluetooth_track_down"
#define METHOD_BLUETOOTH_TMP_TRACK_FF				"method_tmp_bluetooth_track_ff"
#define METHOD_BLUETOOTH_TMP_TRACK_REW				"method_tmp_bluetooth_track_rew"
#define METHOD_BLUETOOTH_TMP_TRACK_FFREWEND			"method_tmp_bluetooth_track_ffrewend"
#define METHOD_BLUETOOTH_TMP_CHANGE_REPEAT			"method_tmp_bluetooth_change_repeat"
#define METHOD_BLUETOOTH_TMP_CHANGE_SHUFFLE			"method_tmp_bluetooth_change_shuffle"
#define METHOD_BLUETOOTH_TMP_BROWSING_TYPE			"method_tmp_bluetooth_browsing_type"
#define METHOD_BLUETOOTH_TMP_BROWSING_LIST			"method_tmp_bluetooth_browsing_list"
#define METHOD_BLUETOOTH_TMP_BROWSING_PLAY			"method_tmp_bluetooth_browsing_play"
#define METHOD_BLUETOOTH_TMP_BROWSING_CHANGE		"method_tmp_bluetooth_browsing_change"

/*================================== BLUETOOTH PROCESS DEFINE END ====================================*/

#endif
