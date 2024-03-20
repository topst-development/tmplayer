#ifndef DBUS_MSG_DEF_MM_H
#define DBUS_MSG_DEF_MM_H

#define MODEMANAGER_PROCESS_DBUS_NAME					"telechips.mode.manager"
#define MODEMANAGER_PROCESS_OBJECT_PATH					"/telechips/mode/manager"
#define MODEMANAGER_EVENT_INTERFACE						"mode.manager"

/********************************METHOD*************************************************/
#define CHANGE_MODE										"change_mode"
#define RELEASE_RESOURCE_DONE							"release_resource_done"
#define END_MODE										"end_mode"
#define MODE_ERROR_OCCURED								"mode_error_occured"

typedef enum{
		Change_Mode,
		Release_Resource_Done,
		End_Mode,
		Mode_Error_Occured,
		TotalMethodModeManagerEvent
}MethodModeManagerEvent;
extern const char* g_methodModeManagerEventNames[TotalMethodModeManagerEvent];

/********************************SIGNAL*************************************************/
#define CHANGED_MODE									"changed_mode"
#define RELEASE_RESOURCE								"release_resource"
#define ENDED_MODE										"ended_mode"
#define SUSPEND_MODE									"suspend_mode"
#define RESUME_MODE										"resume_mode"

typedef enum{
		Changed_Mode,
		Release_Resource,
		EndedMode,
		SuspendMode,
		ResumeMode,
		TotalSignalModeManagerEvent
}SignalModeManagerEvent;
extern const char* g_signalModeManagerEventNames[TotalSignalModeManagerEvent];

#endif
