/****************************************************************************************
 *   FileName    : DBusMsgDef.h
 *   Description : DBusMsgDef.h
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

#ifndef DBUS_MSG_DEF_H
#define DBUS_MSG_DEF_H


/*================================================ TMPLAYER DAEMON DEFINE START ============================================*/

#define TMPLAYER_PROCESS_DBUS_NAME					"telechips.tmplayer.process"
#define TMPLAYER_PROCESS_OBJECT_PATH					"/telechips/tmplayer/process"
#define TMPLAYER_EVENT_INTERFACE						"tmplayer.event"


/*================================================ TMPLAYER METHOD EVENT DEFINE ============================================*/
#define METHOD_TMPLAYER_SET_APP_ID					"method_tmplayer_set_app_id"
#define METHOD_TMPLAYER_CONNECT_DEBUG				"method_tmplayer_connect_debug"
#define METHOD_TMPLAYER_DB_DEBUG						"method_tmplayer_db_debug"
#define METHOD_TMPLAYER_CHANGE_CONTENT_TYPE		"method_tmplayer_change_content_type"
#define METHOD_TMPLAYER_CHANGE_NEXT_DEVICE			"method_tmplayer_change_next_device"
#define METHOD_TMPLAYER_CHANGE_DEVICE				"method_tmplayer_change_device"
#define METHOD_TMPLAYER_SET_BROWSING_MODE			"method_tmplayer_set_browsing_mode"
#define METHOD_TMPLAYER_CHANGE_REPEATMODE			"method_tmplayer_change_repeatmode"
#define METHOD_TMPLAYER_CHANGE_SHUFFLEMODE			"method_tmplayer_change_shufflemode"
#define METHOD_TMPLAYER_TRACK_START					"method_tmplayer_track_start"
#define METHOD_TMPLAYER_TRACK_STOP					"method_tmplayer_track_stop"
#define METHOD_TMPLAYER_TRACK_RESUME				"method_tmplayer_track_resume"
#define METHOD_TMPLAYER_TRACK_PAUSE					"method_tmplayer_track_pause"
#define METHOD_TMPLAYER_TRACK_UP						"method_tmplayer_track_up"
#define METHOD_TMPLAYER_TRACK_DOWN					"method_tmplayer_track_down"
#define METHOD_TMPLAYER_TRACK_MOVE					"method_tmplayer_track_move"
#define METHOD_TMPLAYER_TRACK_SEEK					"method_tmplayer_track_seek"
#define METHOD_TMPLAYER_TRACK_FORWARD				"method_tmplayer_track_forward"
#define METHOD_TMPLAYER_TRACK_REWIND				"method_tmplayer_track_rewind"
#define METHOD_TMPLAYER_TRACK_FFREWEND				"method_tmplayer_track_ffrewend"
#define METHOD_TMPLAYER_UPDATE_DISPLAY_INFO		"method_tmplayer_update_display_info"
#define METHOD_TMPLAYER_UPDATE_EXT_DISPLAY_INFO		"method_tmplayer_update_ext_display_info"
#define METHOD_TMPLAYER_REQUEST_FILELIST				"method_tmplayer_request_filelist"
#define METHOD_TMPLAYER_META_BROWSING_START		"method_tmplayer_meta_browsing_start"
#define METHOD_TMPLAYER_META_BROWSING_REQUEST		"method_tmplayer_meta_browsing_request"
#define METHOD_TMPLAYER_META_BROWSING_SELECT		"method_tmplayer_meta_browsing_select"
#define METHOD_TMPLAYER_META_BROWSING_HOME		"method_tmplayer_meta_browsing_home"
#define METHOD_TMPLAYER_META_BROWSING_UNDO		"method_tmplayer_meta_browsing_undo"
#define METHOD_TMPLAYER_META_BROWSING_END			"method_tmplayer_meta_browsing_end"
#define METHOD_TMPLAYER_REQUEST_DISPLAY			"method_tmplayer_request_display"
#define METHOD_TMPLAYER_GO_LAUNCHER_HOME		"method_tmplayer_go_launcher_home"
#define METHOD_TMPLAYER_GO_LAUNCHER_BACK		"method_tmplayer_go_launcher_back"
#define METHOD_TMPLAYER_GO_AV_ON_OFF			"method_tmplayer_go_av_on_off"
#define METHOD_TMPLAYER_NOTIFY_DISPLAY_DONE		"method_tmplayer_notify_display_done"


typedef enum{
	MethodTMPlayerSetAppID,
	MethodTMPlayerConnectDebug,
	MethodTMPlayerDBDebug,
	MethodTMPlayerChangeContentType,
	MethodTMPlayerChangeNextDevice,
	MethodTMPlayerChangeDevice,
	MethodTMPlayerSetBrowsingMode,
	MethodTMPlayerChangeRepeatMode,
	MethodTMPlayerChangeShuffleMode,
	MethodTMPlayerTrackStart,
	MethodTMPlayerTrackStop,
	MethodTMPlayerTrackResume,
	MethodTMPlayerTrackPause,
	MethodTMPlayerTrackUp,
	MethodTMPlayerTrackDown,
	MethodTMPlayerTrackMove,
	MethodTMPlayerTrackSeek,
	MethodTMPlayerTrackForward,
	MethodTMPlayerTrackRewind,
	MethodTMPlayerTrackFFREWEnd,
	MethodTMPlayerUpdateVideoDisplayInfo,
	MethodTMPlayerUpdateExtDisplayInfo,
	MethodTMPlayerRequestList,
	MethodTMPlayerMetaBrowsingStart,
	MethodTMPlayerMetaBrowsingRequest,
	MethodTMPlayerMetaBrowsingSelect,
	MethodTMPlayerMetaBrowsingHome,
	MethodTMPlayerMetaBrowsingUndo,
	MethodTMPlayerMetaBrowsingEnd,
	MethodTMPlayerRequestDisplay,
	MethodTMPlayerGoLauncherHome,
	MethodTMPlayerGoLauncherBack,
	MethodTMPlayerGoAVOnOff,
	MethodTMPlayerNotifyDisplayDone,
	TotalMethodTMPlayerEvent
}MethodTMPlayerEvent;
extern const char* g_methodTMPlayerEventNames[TotalMethodTMPlayerEvent];

/*================================================ TMPLAYER SIGNAL EVENT DEFINE ============================================*/
#define SIGNAL_TMPLAYER_PLAY_FAILED					"signal_tmplayer_play_failed"
#define SIGNAL_TMPLAYER_PLAYSTATE_CHANGED			"signal_tmplayer_playstate_changed"
#define SIGNAL_TMPLAYER_PLAYTIME_CHANGED			"signal_tmplayer_playtime_changed"
#define SIGNAL_TMPLAYER_REPEATMODE_CHANGED			"signal_tmplayer_repeatmode_changed"
#define SIGNAL_TMPLAYER_SHUFFLEMODE_CHANGED			"signal_tmplayer_shufflemode_changed"
#define SIGNAL_TMPLAYER_TOTALTIME_CHANGED			"signal_tmplayer_totaltime_chagned"
#define SIGNAL_TMPLAYER_SEEK_COMPLETED				"signal_tmplayer_seek_completed"
#define SIGNAL_TMPLAYER_ID3_INFOMATION				"signal_tmplayer_id3_infomation"
#define SIGNAL_TMPLAYER_ALBUMART_KEY				"signal_tmplayer_albumart_key"
#define SIGNAL_TMPLAYER_ALBUMART_COMPLETED			"signal_tmplayer_albumart_completed"
#define SIGNAL_TMPLAYER_NO_DEVICE					"signal_tmplayer_no_device"
#define SIGNAL_TMPLAYER_CONTENT_CHANGED				"signal_tmplayer_content_changed"
#define SIGNAL_TMPLAYER_DEVICE_CHANGED				"signal_tmplayer_device_changed"
#define SIGNAL_TMPLAYER_DEVICE_CONNECTED			"signal_tmplayer_device_connected"
#define SIGNAL_TMPLAYER_DEVICE_DISCONNECTED			"signal_tmplayer_device_disconnect"
#define SIGNAL_TMPLAYER_DEVICE_MOUNT_STOP			"signal_tmplayer_device_mount_stop"
#define SIGNAL_TMPLAYER_DEVICE_READY				"signal_tmplayer_device_ready"
#define SIGNAL_TMPLAYER_NO_SONG						"signal_tmplayer_no_song"
#define SIGNAL_TMPLAYER_RESET_INFORMATION			"signal_tmplayer_reset_information"
#define SIGNAL_TMPLAYER_FILENUMBER_CHANGED			"signal_tmplayer_filenumber_changed"
#define SIGNAL_TMPLAYER_FILENUMBER_UPDATED			"signal_tmplayer_filenumber_updated"
#define SIGNAL_TMPLAYER_TOTALNUMBER_CHAGNED			"signal_tmplayer_totalnumber_changed"
#define SIGNAL_TMPLAYER_FILE_DB_COMPLETED			"signal_tmplayer_file_db_completed"
#define SIGNAL_TMPLAYER_FILENAME_CHANGED			"signal_tmplayer_filename_changed"
#define SIGNAL_TMPLAYER_FOLDERNAME_CHANGED			"signal_tmplayer_foldername_changed"
#define SIGNAL_TMPLAYER_FILELIST					"signal_tmplayer_filelist"
#define SIGNAL_TMPLAYER_FILELIST_CHANGED			"signal_tmplayer_filelist_changed"
#define SIGNAL_TMPLAYER_META_COUNT_CHANGED			"signal_tmplayer_meta_count_changed"
#define SIGNAL_TMPLAYER_META_MENU_CHANGED			"signal_tmplayer_meta_menu_changed"
#define SIGNAL_TMPLAYER_META_ADD_INFO				"signal_tmplayer_meta_add_info"
#define SIGNAL_TMPLAYER_META_CREATE_TRACKLIST		"signal_tmplayer_meta_create_tracklist"
#define SIGNAL_TMPLAYER_DISPLAY_ON					"signal_tmplayer_display_on"
#define SIGNAL_TMPLAYER_DISPLAY_OFF					"signal_tmplayer_display_off"
#define SIGNAL_TMPLAYER_ALBUMART_SHMDATA			"signal_tmplayer_albumart_shmdata"
#define SIGNAL_TMPLAYER_SAMPLERATE					"signal_tmplayer_samplerate"

typedef enum{
	SignalTMPlayerPlayFailed,
	SignalTMPlayerPlayStateChanged,
	SignalTMPlayerPlayTimeChanged,
	SignalTMPlayerRepeatModeChanged,
	SignalTMPlayerShuffleModeChanged,
	SignalTMPlayerTotalTimeChanged,
	SignalTMPlayerSeekCompleted,
	SignalTMPlayerID3Information,
	SignalTMPlayerAlbumArtkey,
	SignalTMPlayerAlbumArtCompleted,
	SignalTMPlayerNoDevice,
	SignalTMPlayerContentChanged,
	SignalTMPlayerDeviceChagned,
	SignalTMPlayerDeviceConnected,
	SignalTMPlayerDeviceDisConnected,
	SignalTMPlayerDeviceMountStop,
	SignalTMPlayerDeviceReady,
	SignalTMPlayerNoSong,
	SignalTMPlayerResetInormation,
	SignalTMPlayerFileNumberChanged,
	SignalTMPlayerFileNumberUpdated,
	SignalTMPlayerTotalNumberChanged,
	SignalTMPlayerFileDBCompleted,
	SignalTMPlayerFileNameChanged,
	SignalTMPlayerFolderNameChanged,
	SignalTMPlayerFileList,
	SignalTMPlayerFileListChanged,
	SignalTMPlayerMetaCountChanged,
	SignalTMPlayerMetaMenuChanged,
	SignalTMPlayerMetaAddInfo,
	SignalTMPlayerMetaCreateTrackList,
	SignalTMPlayerDisplayOn,
	SignalTMPlayerDisplayOff,
	SignalTMPlayerAlbumArtShmData,
	SignalTMPlayerSamplerate,
	TotalSignalTMPlayerEvent
}SignalTMPlayerEvent;
extern const char* g_signalTMPlayerEventNames[TotalSignalTMPlayerEvent];

/*================================================ TMPLAYER DAEMON DEFINE END ================================================*/
/*================================================ MEDIAPLAYBACK DAEMON DEFINE START ================================================*/

#define MEDIAPLAYBACK_PROCESS_DBUS_NAME					"telechips.mediaplayback.process"
#define MEDIAPLAYBACK_PROCESS_OBJECT_PATH				"/telechips/mediaplayback/process"
#define MEDIAPLAYBACK_EVENT_INTERFACE					"mediaplayback.event"

/*=========================== MEDIAPLAYBACK SIGNAL EVENT DEFINES ============================*/
#define SIGNAL_MEDIAPLAYBACK_PLAYING					"signal_mediaplayback_playing"
#define SIGNAL_MEDIAPLAYBACK_STOPPED					"signal_mediaplayback_stopped"
#define SIGNAL_MEDIAPLAYBACK_PAUSED					"signal_mediaplayback_paused"
#define SIGNAL_MEDIAPLAYBACK_DURATION				"signal_mediaplayback_duration"
#define SIGNAL_MEDIAPLAYBACK_PLAYPOSITION			"signal_mediaplayback_playpostion"
#define SIGNAL_MEDIAPLAYBACK_TAGINFO				"signal_mediaplayback_taginfo"
#define SIGNAL_MEDIAPLAYBACK_ALBUMART_COMPLETED	"signal_mediaplayback_albumart_completed"
#define SIGNAL_MEDIAPLAYBACK_PLAY_ENDED				"signal_mediaplayback_play_ended"
#define SIGNAL_MEDIAPLAYBACK_SEEK_COMPLETED			"signal_mediaplayback_seek_completed"
#define SIGNAL_MEDIAPLAYBACK_ERROR					"signal_mediaplayback_error"
#define SIGNAL_MEDIAPLAYBACK_SAMPLERATE				"signal_mediaplayback_samplerate"

typedef enum {
	SignalMediaPlaybackPlaying,
	SignalMediaPlaybackStopped,
	SignalMediaPlaybackPaused,
	SignalMediaPlaybackDuration,
	SignalMediaPlaybackPlayPostion,
	SignalMediaPlaybackTagInfo,
	SignalMediaPlaybackAlbumArtCompleted,
	SignalMediaPlaybackPlayEnded,
	SignalMediaPlaybackSeekCompleted,
	SignalMediaPlaybackError,
	SignalMediaPlaybackSamplerate,
	TotalSignalMediaPlaybackEvents
} SignalMediaPlaybackEvent;
extern const char *g_signalMediaPlaybackEventNames[TotalSignalMediaPlaybackEvents];

/*=========================== MEDIAPLAYBACK METHOD EVENT DEFINES ============================*/
#define METHOD_MEDIAPLAYBACK_PLAY_START				"method_mediaplayback_play_start"
#define METHOD_MEDIAPLAYBACK_PLAY_STOP				"method_mediaplayback_play_stop"
#define METHOD_MEDIAPLAYBACK_PLAY_PAUSE				"method_mediaplayback_play_pause"
#define METHOD_MEDIAPLAYBACK_PLAY_RESUME			"method_mediaplayback_play_resume"
#define METHOD_MEDIAPLAYBACK_PLAY_SEEK				"method_mediaplayback_play_seek"
#define METHOD_MEDIAPLAYBACK_SET_DISPLAY			"method_mediaplayback_set_display"
#define METHOD_MEDIAPLAYBACK_SET_DUAL_DISPLAY		"method_mediaplayback_set_dual_display"
#define METHOD_MEDIAPLAYBACK_SET_DEBUG				"method_mediaplayback_set_debug"
#define METHOD_MEDIAPLAYBACK_GET_STATUS				"method_mediaplayback_get_status"
#define METHOD_MEDIAPLAYBACK_GET_ALBUMART_KEY		"method_mediaplayback_get_albumart_key"
#define METHOD_MEDIAPLAYBACK_GET_PLAY_ID			"method_mediaplayback_get_play_id"


typedef enum {
	MethodMediaPlaybackPlayStart,
	MethodMediaPlaybackPlayStop,
	MethodMediaPlaybackPlayPause,
	MethodMediaPlaybackPlayResume,
	MethodMediaPlaybackPlaySeek,
	MethodMediaPlaybackSetDisplay,
	MethodMediaPlaybackSetDualDisplay,
	MethodMediaPlaybackSetDebug,
	MethodMediaPlaybackGetStatus,
	MethodMediaPlaybackGetAlbumArtKey,
	MethodMediaPlaybackGetPlayID,
	TotalMethodMediaPlaybackEvents
} MethodMediaPlaybackEvent;
extern const char *g_methodMediaPlaybackEventNames[TotalMethodMediaPlaybackEvents];
/*================================================ MEDIAPLAYBACK DAEMON DEFINE END ================================================*/


#endif // DBUS_MSG_DEF_H
