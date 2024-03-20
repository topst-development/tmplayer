/****************************************************************************************
 *   FileName    : TMPlayerType.h
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

#ifndef TMPLAYER_TYPE_H
#define TMPLAYER_TYPE_H

typedef enum {
	MultiMediaContentTypeAudio,
	MultiMediaContentTypeVideo,
	TotalMultiMediaContentTypes
} MultiMediaContentType;

typedef enum {
	/* Storage type */
	DeviceSourceMyMusic,
	DeviceSourceUSB1,
	DeviceSourceUSB2,
	DeviceSourceUSB3,
	DeviceSourceSDMMC,
	/* Non Storage type */
	DeviceSourceAUX,
	DeviceSourceiAP2,
	DeviceSourceBluetooth,
	TotalDeviceSources
} DeviceSource;

typedef enum {
	MetaCategoryTitle,
	MetaCategoryArtist,
	MetaCategoryAlbum,
	MetaCategoryGenre,
	MetaCategoryLibrary,
	TotalMetaCategories
} MetaCategory;

typedef enum {
	RepeatModeOff,
	RepeatModeTrack,
	RepeatModeFolder,
	RepeatModeAll,
	TotalRepeatModes
} RepeatMode;

typedef enum {
	ShuffleModeOff,
	ShuffleModeOn,
	TotalShuffleModes
} ShuffleMode;

typedef struct _PlayTime
{
	unsigned char _hour;
	unsigned char _min;
	unsigned char _sec;
} PlayTime;

typedef enum {
	PlayStatusStop,
	PlayStatusPlaying,
	PlayStatusPause,
	TotalPlayStatus
} TCPlayStatus;

typedef enum
{
	META_INVALID_CATEGORY = -1,
	META_ALL, /*Top-level*/
	META_PLAYLIST,
	META_ARTIST,
	META_ALBUM,
	META_GENRE,
	META_TRACK,
	META_COMPOSER,
	META_AUDIOBOOK,
	META_PODCAST,
	META_ITUNESRADIO,
	META_FOLDER,
	TOTAL_META_TYPE
} CategoryType;/*Database category types for commands*/

extern const char *g_TCPlayStatusNames[TotalPlayStatus];

#endif /* MEDIA_TYPE_H */

