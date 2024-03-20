/****************************************************************************************
 *   FileName    : TMPlayerAux.c
 *   Description : TMPlayerAux.c
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

#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <alsa/asoundlib.h>
#include <pulse/pulseaudio.h>
#include "TMPlayerAux.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int g_tc_debug;
#define DEBUG_TMPLAYER_AUX_PRINTF(format, arg...) \
	if (g_tc_debug) \
	{ \
		fprintf(stderr, "[TMPLAYER-AUX] %s: " format "", __FUNCTION__, ##arg); \
	}

#define ERROR_TMPLAYER_AUX_PRINTF(format, arg...) \
		fprintf(stderr, "[TMPLAYER-AUX][ERROR] %s: " format "", __FUNCTION__, ##arg);


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define PA_MODULE_LOOPBACK_NAME	"module-loopback"
#define PA_MODULE_LOOPBACK_ARGS	"source=0 sink=2 sink_input_properties=media.role=media latency_msec=1000"

#define AUX_DEVICE_PATH_1 "/sys/devices/platform/aux_detect/aux_detect_status"
#define AUX_DEVICE_PATH_2 "/sys/devices/aux_detect/aux_detect_status"

static const char *pAuxStatusDevice = NULL;


static AuxConnect_cb			auxConnect_cb = NULL;
static AuxDisconnect_cb			auxDisconnect_cb = NULL;

#define AUX_DISCONNTECTED 	0
#define AUX_CONNTECTED 		1

static int aux_task_stop = 0;
static int isAuxThreadCreated = 0;
static int auxDetectEnable = 0;

pthread_t auxStatusThread;

typedef enum
{
    PA_Load_Module,
    PA_Unload_Module,
} PACmdType;

static pa_context *context = NULL;
static pa_mainloop_api *mainloop_api = NULL;
static pa_proplist *proplist = NULL;
static char card_name[64] = "hw:0";

enum source_index {
    SOURCE_INDEX_MIC = 0  ,
    SOURCE_INDEX_AUX      ,
    SOURCE_INDEX_NUM      ,
};

struct codec_command_value_map {
    const char * command;
    const char * value[SOURCE_INDEX_NUM];
};

const struct codec_command_value_map cmd_val_map[2] = {
    {"ADC2 MUX",                  {"AIN2",      "AIN3"}},
    {"SDOUT1 Source Selector",    {"MixerB",    "ADC2"}},
};

static snd_mixer_t *g_mixer_handle = NULL;

static pthread_mutex_t auxMutex;

static int g_auxOn =0;

static int AuxGetConnectStatus(void);
static void * aux_status_task_function(void *arg);


static int codec_mixer_init(void);
static void codec_mixer_deinit(void);
static int set_enumerated_value(snd_mixer_elem_t *elem, const char *value);
static int set_input_source_switch(int index);

static int PACommandClient(PACmdType cmd);
static void quit(int ret);
static void context_drain_complete(pa_context *c, void *userdata);
static void complete_action(void);
static void simple_callback(pa_context *c, int success, void *userdate);
static void exit_signal_callback(pa_mainloop_api *m, pa_signal_event *e, int sig, void *userdata);
static void index_callback(pa_context *c, uint32_t idx, void *userdata);
static void unload_module_callback(pa_context *c, const pa_module_info *i, int is_last, void *userdata);
static void context_state_callback(pa_context *c, void *userdata);
static void PAModuleLoad(void);
static void PAModuleUnload(void);

static int codec_mixer_init(void)
{
    int err = 0;

    if (g_mixer_handle == NULL)
	{
        if ((err = snd_mixer_open(&g_mixer_handle, 0)) < 0)
		{
            ERROR_TMPLAYER_AUX_PRINTF("snd_mixer_open error : %d\n", err);
            return err;
        }

        if ((err = snd_mixer_attach(g_mixer_handle, card_name)) < 0)
		{
            ERROR_TMPLAYER_AUX_PRINTF("snd_mixer_attach error : %d\n", err);
            snd_mixer_close(g_mixer_handle);
            g_mixer_handle = NULL;
            return err;
        }
        if ((err = snd_mixer_selem_register(g_mixer_handle, NULL, NULL)) < 0)
		{
            ERROR_TMPLAYER_AUX_PRINTF("snd_mixer_selem_register error : %d\n", err);
            snd_mixer_close(g_mixer_handle);
            g_mixer_handle = NULL;
            return err;
        }

        err = snd_mixer_load(g_mixer_handle);
        if (err < 0)
		{
            ERROR_TMPLAYER_AUX_PRINTF("Mixer %s load error: %d\n", card_name, err);
            snd_mixer_close(g_mixer_handle);
            g_mixer_handle = NULL;
            return err;
        }
    }
	else
	{
    	ERROR_TMPLAYER_AUX_PRINTF("g_mixer is busy \n");
		err = -1;
	}
    return err;
}

static void codec_mixer_deinit(void)
{
    if (g_mixer_handle != NULL)
	{
        snd_mixer_close(g_mixer_handle);
        g_mixer_handle = NULL;
    }
	else
	{
        ERROR_TMPLAYER_AUX_PRINTF("handle is null!!!\n");
    }
}

static int set_enumerated_value(snd_mixer_elem_t *elem, const char *value)
{
    char item_name[64];
    int ival = 0, i, len, available = 0;
    const int enum_items = snd_mixer_selem_get_enum_items(elem);
    if (enum_items <= 0)
	{
        ERROR_TMPLAYER_AUX_PRINTF("enum items in mixer is invalid: %d\n", enum_items);
        return -1;
    }
    for (i = 0; i < enum_items; i++)
	{
        if (snd_mixer_selem_get_enum_item_name(elem, i, sizeof(item_name)-1, item_name) < 0)
		{
            DEBUG_TMPLAYER_AUX_PRINTF("get enum item name fail.. index = %d\n", i);
            continue;
        }

        len = strlen(item_name) >= strlen(value) ? strlen(item_name) : strlen(value);

        if (!strncmp(item_name, value, len))
		{
            ival = i;
            available = 1;
            DEBUG_TMPLAYER_AUX_PRINTF("find enum item ids : %u\n", ival);
            break;
        }
    }

    if (available)
	{
        if (snd_mixer_selem_set_enum_item(elem, 0, ival) < 0)
		{
            ERROR_TMPLAYER_AUX_PRINTF("fail to set enum item of %d\n", ival);
            return -1;
        }
    }

    return 0;
}

static int set_input_source_switch(int index)
{
    int ret = 0, i = 0;

    snd_mixer_elem_t *elem = NULL;
    snd_mixer_selem_id_t *sid;
    snd_mixer_selem_id_alloca(&sid);

    if (index < 0 || index >= SOURCE_INDEX_NUM)
	{
        ERROR_TMPLAYER_AUX_PRINTF("index is invalid(%d)\n", index);
        return -1;
    }

    for (i = 0; i< 2; i++)
	{
        snd_mixer_selem_id_set_index(sid, 0);
        snd_mixer_selem_id_set_name(sid, cmd_val_map[i].command);
        elem = snd_mixer_find_selem(g_mixer_handle, sid);
        if (!elem)
		{
            ERROR_TMPLAYER_AUX_PRINTF("Unable to find simple control '%s',%u\n", snd_mixer_selem_id_get_name(sid), snd_mixer_selem_id_get_index(sid));
            codec_mixer_deinit();
            return -1;
        }
        if (snd_mixer_selem_is_enumerated(elem))
		{
            ret = set_enumerated_value(elem, cmd_val_map[i].value[index]);
        }
    }

    return ret;
}


static int PACommandClient(PACmdType cmd)
{
    pa_mainloop *m = NULL;
    int ret = 1;
    DEBUG_TMPLAYER_AUX_PRINTF("%s : %d\n", __FUNCTION__, cmd);
    proplist = pa_proplist_new();

    if (!(m = pa_mainloop_new()))
    {
        DEBUG_TMPLAYER_AUX_PRINTF("pa_mainloop_new() failed.\n");
        goto quit;
    }

    mainloop_api = pa_mainloop_get_api(m);
    pa_signal_init(mainloop_api);
    pa_signal_new(SIGINT, exit_signal_callback, NULL);
    pa_signal_new(SIGTERM, exit_signal_callback, NULL);

    if (!(context = pa_context_new_with_proplist(mainloop_api, NULL, proplist)))
    {
        ERROR_TMPLAYER_AUX_PRINTF("pa_context_new() failed.\n");
        goto quit;
    }

    pa_context_set_state_callback(context, context_state_callback, (void *)cmd);
    if (pa_context_connect(context, NULL, 0, NULL) < 0)
    {
        ERROR_TMPLAYER_AUX_PRINTF("pa_context_connect() failed : %s\n",pa_strerror(pa_context_errno(context)));
        goto quit;
    }

    if (pa_mainloop_run(m, &ret) < 0)
    {
        ERROR_TMPLAYER_AUX_PRINTF("pa_mainloop_run() failed\n");
        goto quit;
    }
quit:
    if (context)
        pa_context_unref(context);

    if (m)
    {
        pa_signal_done();
        pa_mainloop_free(m);
    }

    if (proplist)
        pa_proplist_free(proplist);

	DEBUG_TMPLAYER_AUX_PRINTF("End(%d)\n", ret);

    return ret;
}

static void quit(int ret)
{
    mainloop_api->quit(mainloop_api, ret);
}

static void context_drain_complete(pa_context *c, void *userdata)
{
    pa_context_disconnect(c);
}

static void complete_action(void)
{
	DEBUG_TMPLAYER_AUX_PRINTF("\n");

    pa_operation *o;
    if (!(o = pa_context_drain(context, context_drain_complete, NULL)))
	{
        pa_context_disconnect(context);
	}
    else
	{
        pa_operation_unref(o);
	}
}

static void simple_callback(pa_context *c, int success, void *userdate)
{
	DEBUG_TMPLAYER_AUX_PRINTF("\n");
    if (!success)
    {
        ERROR_TMPLAYER_AUX_PRINTF("Failure : %s\n", pa_strerror(pa_context_errno(c)));
        quit(1);
        return;
    }
}

static void exit_signal_callback(pa_mainloop_api *m, pa_signal_event *e, int sig, void *userdata)
{
    DEBUG_TMPLAYER_AUX_PRINTF("Got SIGINT, exiting.\n");
    quit(0);
}

static void index_callback(pa_context *c, uint32_t idx, void *userdata)
{
    if (idx == PA_INVALID_INDEX)
	{
	    ERROR_TMPLAYER_AUX_PRINTF("Failure load Module\n");
        quit(1);
        return;
    }

    DEBUG_TMPLAYER_AUX_PRINTF("%u\n", idx);

    complete_action();
}

static int unloaded = 0;
static int isModuleLoaded = 0;

static void load_module_callback(pa_context *c, const pa_module_info *i, int is_last, void *userdata)
{
    char *module ;
	pa_operation *o = NULL;

	DEBUG_TMPLAYER_AUX_PRINTF("is_last(%d)\n", is_last);

    if (is_last < 0)
    {
        ERROR_TMPLAYER_AUX_PRINTF("Failed to get module information\n");
        quit(1);
        return;
    }

    if (is_last)
    {
	    if (isModuleLoaded == 0)
		{
			DEBUG_TMPLAYER_AUX_PRINTF("Module loaded.(%s)\n", PA_MODULE_LOOPBACK_NAME);
			o = pa_context_load_module(c, PA_MODULE_LOOPBACK_NAME, PA_MODULE_LOOPBACK_ARGS, index_callback, NULL);
		}
		else
		{
			DEBUG_TMPLAYER_AUX_PRINTF("Already module loaded.(%s)\n", PA_MODULE_LOOPBACK_NAME);
			complete_action();
		}
		return;
    }

    if (i->name != NULL)
    {
		//DEBUG_TMPLAYER_AUX_PRINTF("list name(%s)\n", i->name);
        module = strstr(i->name, PA_MODULE_LOOPBACK_NAME);
        if (module != NULL)
        {
			isModuleLoaded = 1;
        }
    }
}


static void unload_module_callback(pa_context *c, const pa_module_info *i, int is_last, void *userdata)
{
    char *module ;

    if (is_last < 0)
    {
        ERROR_TMPLAYER_AUX_PRINTF("Failed to get module information\n");
        quit(1);
        return;
    }


    if (is_last)
    {
	    if (unloaded == 0)
    	{
	        ERROR_TMPLAYER_AUX_PRINTF("Failed to unload module: Module %s not loaded\n", PA_MODULE_LOOPBACK_NAME);
    	}
		complete_action();
		DEBUG_TMPLAYER_AUX_PRINTF("is_last End(%d)\n", is_last);
        return;
    }
    if (i->name != NULL)
    {
        module = strstr(i->name, PA_MODULE_LOOPBACK_NAME);
        if (module != NULL)
        {
            unloaded = 1;
			DEBUG_TMPLAYER_AUX_PRINTF("run pa_operation_unref , index(%d) \n", i->index);
        	pa_context_unload_module(c, i->index, simple_callback, NULL);
        }

    }
}

static void context_state_callback(pa_context *c, void *userdata)
{
    PACmdType cmd = (PACmdType) userdata;
    pa_operation *o = NULL;

    switch(pa_context_get_state(c))
	{
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;

        case PA_CONTEXT_READY:
            switch(cmd)
			{
                case PA_Load_Module:
                    isModuleLoaded = 0;
                    o = pa_context_get_module_info_list(c, load_module_callback, NULL);
                    break;
                case PA_Unload_Module:
					unloaded = 0;
                    o = pa_context_get_module_info_list(c, unload_module_callback, NULL);
                    break;
                default:
                    break;
                    //pa_assert_not_reached();
            }

            if (o)
			{
				pa_operation_unref(o);
            }

            break;
        case PA_CONTEXT_TERMINATED:
            quit(0);
            break;
        case PA_CONTEXT_FAILED:
        default:
            DEBUG_TMPLAYER_AUX_PRINTF("Connection failure: %s\n", pa_strerror(pa_context_errno(c)));
            quit(1);
        }
}


static void PAModuleLoad(void)
{
	(void)PACommandClient(PA_Load_Module);
}

static void PAModuleUnload(void)
{
	(void)PACommandClient(PA_Unload_Module);
}


int AuxPlayInit(void (*connect_cb)(char *key),void (*disconnect_cb)(char *key))
{
	int ret;
	DEBUG_TMPLAYER_AUX_PRINTF("\n");
	if ((connect_cb != NULL) && (disconnect_cb != NULL))
	{
		(void)pthread_mutex_init(&auxMutex, NULL);
	    ret = codec_mixer_init();
	    if (ret < 0)
		{
	        ERROR_TMPLAYER_AUX_PRINTF("mixer init err : %d\n", ret);
	    }
		else
		{

			pAuxStatusDevice = NULL;
			if (access(AUX_DEVICE_PATH_1,R_OK) == 0)
			{
				pAuxStatusDevice = AUX_DEVICE_PATH_1;
			}
			else
			{
				if (access(AUX_DEVICE_PATH_2,R_OK) == 0)
				{
					pAuxStatusDevice = AUX_DEVICE_PATH_2;
				}
			}

			if (pAuxStatusDevice != NULL)
			{
				auxConnect_cb = connect_cb;
				auxDisconnect_cb = disconnect_cb;
				aux_task_stop = 0;
				auxDetectEnable = 0;

				if (pthread_create(&auxStatusThread, NULL, aux_status_task_function, NULL) == 0 )
				{
					isAuxThreadCreated = 1;
					ret = 0;
				}
				else
				{
					ERROR_TMPLAYER_AUX_PRINTF("create aux status thread failed(%d)\n", ret);
					ret = -1;
				}
			}
			else
			{
				ERROR_TMPLAYER_AUX_PRINTF("Cat't find Aux detect device : (%s) or (%s)\n",AUX_DEVICE_PATH_1,AUX_DEVICE_PATH_2);
				ret = -1;
			}
		}
	}
	else
	{
		ERROR_TMPLAYER_AUX_PRINTF("Argument error.\n");
	}
	return ret;
}

void AuxPlayDeinit(void)
{
	int ret;
	DEBUG_TMPLAYER_AUX_PRINTF("\n");

	if (g_auxOn == 1)
	{
		AuxPlayOff();
		g_auxOn = 0;
	}

	if (auxDisconnect_cb != NULL)
	{
		auxDisconnect_cb("aux");
	}
	aux_task_stop = 1;
	auxConnect_cb = NULL;
	auxDisconnect_cb = NULL;
	auxDetectEnable = 0;
	if (isAuxThreadCreated == 1)
	{
		void *res;
		ret = pthread_join(auxStatusThread, &res);
		if (ret !=0)
		{
			ERROR_TMPLAYER_AUX_PRINTF("Meta DB Thread join failed(%d)\n", ret);
		}
		isAuxThreadCreated = 0;
	}

	codec_mixer_deinit();
	pthread_mutex_destroy(&auxMutex);
}

void AuxDetectEnable(void)
{
	auxDetectEnable = 1;
}

int AuxPlayOn(void)
{
	int ret;
	DEBUG_TMPLAYER_AUX_PRINTF("\n");
	(void)pthread_mutex_lock(&auxMutex);
	if (g_auxOn == 0)
	{
		ret =set_input_source_switch(1);
		if (ret <0)
		{
			ERROR_TMPLAYER_AUX_PRINTF("Change Aux mode fail : %d\n", ret);
		}
		else
		{
			PAModuleLoad();
			g_auxOn = 1;
		}
	}
	(void)pthread_mutex_unlock(&auxMutex);
	DEBUG_TMPLAYER_AUX_PRINTF("Done.\n");
	return ret;
}
int AuxPlayOff(void)
{
	int ret;
	DEBUG_TMPLAYER_AUX_PRINTF("\n");
	(void)pthread_mutex_lock(&auxMutex);

	ret =set_input_source_switch(0);
	if (ret <0)
	{
		ERROR_TMPLAYER_AUX_PRINTF("Change mic mode fail : %d\n", ret);
	}

	PAModuleUnload();
	g_auxOn = 0;
	(void)pthread_mutex_unlock(&auxMutex);
	DEBUG_TMPLAYER_AUX_PRINTF("Done.\n");

	return ret;
}

static int AuxGetConnectStatus(void)
{
	int fd;
	char aux_status[1];
	int len;
	int ret = -1;

	fd = open(pAuxStatusDevice,O_RDWR);
	if (fd > 0)
	{
		len = read(fd, aux_status, 1);
		if (len > 0)
		{
			if (aux_status[0] == 48) //0
			{
				ret = 0;
			}
			else if (aux_status[0] == 49) //1
			{
				ret = 1;
			}
			else
			{
				ERROR_TMPLAYER_AUX_PRINTF("Invalid Value\n");
			}
		}
		else
		{
			ERROR_TMPLAYER_AUX_PRINTF("Device(%s) read fail.\n",pAuxStatusDevice);
		}
		close(fd);
	}
	else
	{
		ERROR_TMPLAYER_AUX_PRINTF("Open device(%s) Fail.\n",pAuxStatusDevice);
	}
	return ret;
}

static void * aux_status_task_function(void *arg)
{
	int ret;
	int aux_status = AUX_DISCONNTECTED;

	DEBUG_TMPLAYER_AUX_PRINTF("\n");

	while (aux_task_stop == 0)
	{
		if (auxDetectEnable == 1)
		{
			ret = AuxGetConnectStatus();
			if (ret != aux_status)
			{
				if (ret == AUX_CONNTECTED)
				{
					DEBUG_TMPLAYER_AUX_PRINTF("Aux connected\n");
					aux_status = AUX_CONNTECTED;
					if (auxConnect_cb != NULL)
					{
						auxConnect_cb("aux");
					}
				}
				else if (ret == AUX_DISCONNTECTED)
				{
					DEBUG_TMPLAYER_AUX_PRINTF("Aux disconnected\n");
					aux_status = AUX_DISCONNTECTED;
					if (auxDisconnect_cb != NULL)
					{
						auxDisconnect_cb("aux");
					}
				}
				else
				{
					DEBUG_TMPLAYER_AUX_PRINTF("Unkowon aux status value - (%d).\n", ret);
				}
			}
		}
		usleep(500000);
	}
	(void)ret;
	pthread_exit((void *) 0);
}


#ifdef __cplusplus
}
#endif

