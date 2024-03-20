/****************************************************************************************
 *   FileName    : TMPlayerContorol.c
 *   Description : 
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

#include <stdio.h>
#include <stdint.h>
#include <dbus/dbus.h>
#include "TMPlayerType.h"
#include "TMPlayerControl.h"
#include "TMPlayerDBusMediaPlayback.h"

#define MULTIMEDIA_BUSY (-1)

#define MULTIMEDIA_IDLE (-1)
#define MULTIMEDIA_INVALID_ID (-2)

extern int g_tc_debug;
#define DEBUG_TMPLAYER_CONTROL_PRINTF(format, arg...) \
	if (g_tc_debug) \
	{ \
		fprintf(stderr, "[TMPLAYER CONTROL] %s: " format "", __FUNCTION__, ##arg); \
	}

void TMPlayerControl_Play(MultiMediaContentType content, const char * path, unsigned char hour, unsigned char min, unsigned char sec, int playID, char pause)
{
	int ret, prevID = 0, busy = 0;

	busy = SendDBusMediaStart(content, path, hour, min, sec, playID, pause);
	if(busy == MULTIMEDIA_BUSY)
	{
		prevID = GetPreviousID();
		ret = SendDBusMediaStop(prevID);
		if (ret == MULTIMEDIA_IDLE)
		{
			(void)SendDBusMediaStart(content, path, hour, min, sec, playID, pause);
		}
		else if (ret == MULTIMEDIA_INVALID_ID)
		{
			DEBUG_TMPLAYER_CONTROL_PRINTF("Media Playback Play ID is invalid: %d", prevID);
		}
	}
}

void TMPlayerControl_Stop(int playID)
{
	int ret, cur_playID;

	ret = SendDBusMediaStop(playID);
	if (ret == MULTIMEDIA_INVALID_ID)
	{
		DEBUG_TMPLAYER_CONTROL_PRINTF("Invalid Play ID: %d", playID);
		cur_playID = SendDBusMediaGetCurrentPlayID();
		ret = SendDBusMediaStop(cur_playID);

		if (ret == MULTIMEDIA_INVALID_ID)
		{
			DEBUG_TMPLAYER_CONTROL_PRINTF("Media Playback Play ID is invalid: %d", cur_playID);
		}
	}
}

void TMPlayerControl_MediaRelease(int playID)
{
	int ret, cur_playID;

	ClearPreviousID();
	ret = SendDBusMediaStop(playID);
	if (ret == MULTIMEDIA_IDLE) // If Media Playback already released
	{
		NotifyMediaReleased();
	}
	else if (ret == MULTIMEDIA_INVALID_ID)
	{
		DEBUG_TMPLAYER_CONTROL_PRINTF("Invalid Play ID: %d", playID);
		cur_playID = SendDBusMediaGetCurrentPlayID();
		ret = SendDBusMediaStop(cur_playID);

		if (ret == MULTIMEDIA_IDLE)
		{
			NotifyMediaReleased();
		}
		if (ret == MULTIMEDIA_INVALID_ID)
		{
			DEBUG_TMPLAYER_CONTROL_PRINTF("Media Playback Play ID is invalid: %d", cur_playID);
		}
	}
}

void TMPlayerControl_Resume(int playID)
{
	int ret, cur_playID;

	ret = SendDBusMediaResume(playID);
	if (ret == MULTIMEDIA_IDLE)
	{
		DEBUG_TMPLAYER_CONTROL_PRINTF("Resource Released");
	}
	else if (ret == MULTIMEDIA_INVALID_ID)
	{
		DEBUG_TMPLAYER_CONTROL_PRINTF("Invalid Play ID: %d", playID);
		cur_playID = SendDBusMediaGetCurrentPlayID();
		SendDBusMediaStop(cur_playID);
	}
}

void TMPlayerControl_Pause(int playID)
{
	int ret, cur_playID;

	ret = SendDBusMediaPause(playID);
	if (ret == MULTIMEDIA_IDLE)
	{
		DEBUG_TMPLAYER_CONTROL_PRINTF("Resource Released");
	}
	else if (ret == MULTIMEDIA_INVALID_ID)
	{
		DEBUG_TMPLAYER_CONTROL_PRINTF("Invalid Play ID: %d", playID);
		cur_playID = SendDBusMediaGetCurrentPlayID();
		SendDBusMediaStop(cur_playID);
	}
}

void TMPlayerControl_Seek(unsigned char hour, unsigned char min, unsigned char sec, int playID)
{
	int ret, cur_playID;

	ret = SendDBusMediaSeek(hour, min, sec, playID);
	if (ret == MULTIMEDIA_IDLE)
	{
		DEBUG_TMPLAYER_CONTROL_PRINTF("Resource Released");
	}
	else if (ret == MULTIMEDIA_INVALID_ID)
	{
		DEBUG_TMPLAYER_CONTROL_PRINTF("Invalid Play ID: %d", playID);
		cur_playID = SendDBusMediaGetCurrentPlayID();
		SendDBusMediaStop(cur_playID);
	}
}

void TMPlayerControl_SetDisplay(int x, int y, int width, int height)
{
	SendDBusMediaSetDisplay(x, y, width, height);
}

void TMPlayerControl_SetDualDisplay(int x, int y, int width, int height)
{
	SendDBusMediaSetDualDisplay(x, y, width, height);
}
