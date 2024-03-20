/****************************************************************************************
 *	 FileName	 : TMPlayerDBusBTManager.h
 *	 Description : TMPlayerDBusBTManager.h
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

#ifndef TMPLAYER_DBUS_BT_MANAGER_H
#define TMPLAYER_DBUS_BT_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

void SendDBusBluetoothTrackStart(void);
void SendDBusBluetoothTrackStop(void);
void SendDBusBluetoothTrackResume(void);
void SendDBusBluetoothTrackPause(void);
void SendDBusBluetoothTrackUp(void);
void SendDBusBluetoothTrackDown(void);
void SendDBusBluetoothForward(void);
void SendDBusBluetoothRewind(void);
void SendDBusBluetoothFFREWEnd(void);
void SendDBusBluetoothChangeRepeat(void);
void SendDBusBluetoothChangeShuffle(void);
void SendDBusBluetoothBrowsingType(unsigned int list_type);
bool SendDBusBluetoothBrowsingList(unsigned int number, unsigned int *count);
void SendDBusBluetoothBrowsingPlay(unsigned int number, unsigned int uid1, unsigned int uid2);
void SendDBusBluetoothBrowsingChange(uint32_t direction, uint32_t index, uint32_t uid1, uint32_t uid2);

void BluetoothSignalDBusProcess(unsigned int id, DBusMessage *message);

#ifdef __cplusplus
}
#endif

#endif
