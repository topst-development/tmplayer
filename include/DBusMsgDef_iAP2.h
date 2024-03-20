/****************************************************************************************
 *   FileName    : DBusMsgDef_iAP2.h
 *   Description : DBusMsgDef_iAP2.h
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

/*================================== iAP2 FEATURE DEFINE START ==================================*/
#define IAP2_FACILITY_DEST_NAME                 "telechips.iap2.facility"
#define IAP2_FACILITY_OBJECT_PATH               "/telechips/iap2/facility"
#define IAP2_FACILITY_FEATURE_INTERFACE         "iap2facility.feature"

//  iAP2 FEATURE METHOD EVENT DEFINES
#define METHOD_PLAYER_IAP2_CHANGED_PLAYER   "method_player_change_player"

/*=================================== iAP FEATURE DEFINE END ====================================*/
/*================================== iAP2 PROCESS DEFINE START ==================================*/
#define IAP2_PROCESS_DBUS_NAME			        "telechips.iap2.process"
#define IAP2_PROCESS_OBJECT_PATH		        "/telechips/iap2/process"
#define IAP2_PROCESS_INTERFACE			        "iap2process.event"

//  iAP2 METHOD EVENT DEFINES
#define METHOD_iAP2_CONNECT                     "method_iap2_connect"
#define METHOD_iAP2_DISCONNECT                  "method_iap2_disconnect"
#define METHOD_iAP2_DB_GETLIBTYPE               "method_iap2_db_get_libtype"
#define METHOD_iAP2_DB_INIT                     "method_iap2_db_init"
#define METHOD_iAP2_DB_INITHANDLER              "method_iap2_db_init_handler"
#define METHOD_iAP2_DB_INITWITHLIBTYPE          "method_iap2_db_init_with_libtype"
#define METHOD_iAP2_DB_SELECT                   "method_iap2_db_select"
#define METHOD_iAP2_DB_SELECTALL                "method_iap2_db_select_all"
#define METHOD_iAP2_DB_GETNAME                  "method_iap2_db_get_name"
#define METHOD_iAP2_DB_MOVEUPPER                "method_iap2_db_move_upper"
#define METHOD_iAP2_DB_PLAYBACKLISTCOUNT        "method_iap2_db_playback_list_count"
#define METHOD_iAP2_DB_GETIDLISTPLAYBACK        "method_iap2_db_id_list_playback"
#define METHOD_iAP2_DB_PLAYBACKLISTGETNAME      "method_iap2_db_playback_list_get_name"
#define METHOD_iAP2_DB_PLAYBACKLISTGETPROP      "method_iap2_db_playback_list_get_prop"
#define METHOD_iAP2_DB_PLAYBACKLISTGETPROPBYID  "method_iap2_db_playback_list_get_prop_by_ids"
#define METHOD_iAP2_DB_PLAYBACKLISTGETNAMEBYID  "method_iap2_db_playback_list_get_name_by_id"
#define METHOD_iAP2_NOW_PLAYING_START           "method_iap2_now_playing_start"
#define METHOD_iAP2_NOW_PLAYING_STOP            "method_iap2_now_playing_stop"
#define METHOD_iAP2_NOW_PLAYING_SET_INFO        "method_iap2_now_playing_set_info"
#define METHOD_iAP2_HID_START                  "method_iap2_hid_start"
#define METHOD_iAP2_HID_REPORT                  "method_iap2_hid_report"
#define METHOD_iAP2_HID_STOP                  "method_iap2_hid_stop"
#define METHOD_iAP2_INIT_APP                  "method_iap2_init_app"
#define METHOD_iAP2_EAP_SEND_DATA             "method_iap2_eap_send_data"
#define METHOD_iAP2_PLAYMEDIALIBRARY_CURRENT_SELECTION             "method_iap2_playmedialibrary_current_selection"
#define METHOD_iAP2_PLAYMEDIALIBRARY_ITEMS              "method_iap2_playmedialibrary_items"
#define METHOD_iAP2_PLAYMEDIALIBRARY_COLLECTION    "method_iap2_playmedialibrary_collection"
#define METHOD_iAP2_PLAYMEDIALIBRARY_SPECIAL            "method_iap2_playmedialibrary_special"
#define METHOD_iAP2_TEST_PENDING_MSG            "method_iap2_test_pending_msg"
#define METHOD_iAP2_COMMUNICATOIN_START_CALL_STATE            "method_iap2_communication_start_call_state"
#define METHOD_iAP2_COMMUNICATOIN_STOP_CALL_STATE            "method_iap2_communication_stop_call_state"
#define METHOD_iAP2_COMMUNICATOIN_START_UPDATE            "method_iap2_communication_start_update"
#define METHOD_iAP2_COMMUNICATOIN_STOP_UPDATE            "method_iap2_communication_stop_update"
#define METHOD_iAP2_COMMUNICATOIN_INITIATE_CALL            "method_iap2_communication_initiate_call"
#define METHOD_iAP2_COMMUNICATOIN_ACCEPT_CALL            "method_iap2_communication_accept_call"
#define METHOD_iAP2_COMMUNICATOIN_END_CALL            "method_iap2_communication_end_call"
#define METHOD_iAP2_COMMUNICATOIN_SWAP_CALL            "method_iap2_communication_swap_call"
#define METHOD_iAP2_COMMUNICATOIN_MERGE_CALL            "method_iap2_communication_merge_call"
#define METHOD_iAP2_COMMUNICATOIN_HOLD_STATUS            "method_iap2_communication_hold_status"
#define METHOD_iAP2_COMMUNICATOIN_MUTE_STATUS            "method_iap2_communication_mute_status"
#define METHOD_iAP2_COMMUNICATOIN_SEND_DTMF            "method_iap2_communication_send_dtmf"
#define METHOD_iAP2_COMMUNICATOIN_START_LIST_UPDATE            "method_iap2_communication_start_list_update"
#define METHOD_iAP2_COMMUNICATOIN_STOP_LIST_UPDATE            "method_iap2_communication_stop_list_update"
#define METHOD_iAP2_VEHICLESTATUS_UPDATE               "method_iap2_vehiclestatus_update"
#define METHOD_iAP2_POWER_SOURCE_UPDATE            "method_iap2_power_source_update"
#define METHOD_iAP2_APP_LAUNCH                                  "method_iap2_app_launch"
#define METHOD_iAP2_IDENTIFICATION_UPDATE            "method_iap2_identification_update"
#define METHOD_iAP2_LOCATION_DATA            "method_iap2_location_data"


//  iAP2 SIGNAL EVENT DEFINES
#define SIGNAL_iAP2_EVENT_PARCEL                "signal_iap2_event_parcel"

typedef enum {
    SignaliAP2EventParcel,
    TotalSignaliAP2Events
} SignaliAP2Event;
extern const char *g_signaliAP2EventNames[TotalSignaliAP2Events];

/*================================== iAP2 PROCESS DEFINE END ====================================*/

/*================================= TM PLAYER iAP2 DEFINE START ==================================*/
#define TMPLAYER_IAP2_INTERFACE                  "tmplayer.iap2"

//  Player iAP2 METHOD EVENT DEFINES
#define METHOD_PLAYER_IAP2_CONNECTED                "method_player_iap2_connected"
#define METHOD_PLAYER_IAP2_NOTIFY                   "method_player_iap2_notify"
#define METHOD_PLAYER_LOG_ENABLE                    "method_player_log_enable"

/*================================= TM PLAYER iAP2 DEFINE END ====================================*/

/*=========================== Apple Device Manager D-Bus Define Start ===========================*/

#define ADM_DBUS_NAME								"telechips.appledevice.manager"
#define ADM_OBJECT_PATH 							"/telechips/appledevice/manager"

#define ADM_MANAGER_INTERFACE						"appledevice.manager"

// Apple Device Manager Daemon Method defines
#define METHOD_ADM_IAP1_INITIALIZE					"method_adm_iap1_initialize"
#define METHOD_ADM_IAP2_INITIALIZE					"method_adm_iap2_initialize"
#define METHOD_ADM_IAP1_RELEASE 					"method_adm_iap1_release"
#define METHOD_ADM_IAP2_RELEASE 					"method_adm_iap2_release"

// Apple Device Manager Daemon Signal defines
#define SIGNAL_ADM_IAP1_CONNECTED					"signal_adm_iap1_connected"
#define SIGNAL_ADM_IAP1_DISCONNECTED				"signal_adm_iap1_disconnected"
#define SIGNAL_ADM_IAP2_CONNECTED					"signal_adm_iap2_connected"
#define SIGNAL_ADM_IAP2_DISCONNECTED				"signal_adm_iap2_disconnected"
#define SIGNAL_ADM_CARPLAY_CONNECTED				"signal_adm_carplay_connected"
#define SIGNAL_ADM_CARPLAY_DISCONNECTED 			"signal_adm_carplay_disconnected"

typedef enum {
	SignalAppleDeviceManageriAP1Connected,
	SignalAppleDeviceManageriAP1Disconnected,
	SignalAppleDeviceManageriAP2Connected,
	SignalAppleDeviceManageriAP2Disconnected,
	SignalAppleDeviceManagerCarPlayConnected,
	SignalAppleDeviceManagerCarPlayDisconnected,
	TotalSignalAppleDeviceManagerEvents
} SignalAppleDeviceManagerEvent;

extern const char *g_signalADMEventNames[TotalSignalAppleDeviceManagerEvents];

/*============================ Apple Device Manager D-Bus Define End ============================*/

