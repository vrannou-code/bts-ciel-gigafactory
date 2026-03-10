// scan_bt.h
#ifndef SCAN_BT_H
#define SCAN_BT_H

#include <zephyr/types.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>

#define RSSI_THRESHOLD -50
#define BADGE_ID_AUTORISE 0x01020304

void bt_scan_thread_start(void); 

#endif // SCAN_BT_H