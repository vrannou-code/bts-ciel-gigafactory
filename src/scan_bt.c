#include "scan_bt.h"

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/logging/log.h>
#include "auth_controller.h"

#include "auth_worker.h"

// LOG_MODULE_REGISTER(scan_bt, LOG_LEVEL_INF);
LOG_MODULE_REGISTER(scan_bt, LOG_LEVEL_NONE);

#define SCAN_STACK_SIZE 1024
#define SCAN_THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(scan_stack_area, SCAN_STACK_SIZE);
static struct k_thread scan_thread;

static struct bt_conn *default_conn;
static uint32_t badge_id = 0;

static const struct bt_le_scan_param scan_param = {
    .type = BT_HCI_LE_SCAN_ACTIVE,
    .options = BT_LE_SCAN_OPT_NONE,
    .interval = 0x0010,
    .window = 0x0010,
};

static bool parse_manufacturer_data(struct bt_data *data, void *user_data)
{
    if (data->type != BT_DATA_MANUFACTURER_DATA || data->data_len < 6)
    {
        return true;
    }

    const uint8_t *payload = data->data;
    uint16_t company_id = sys_get_le16(payload);
    badge_id = sys_get_be32(&payload[2]);

    LOG_INF("Manufacturer ID: 0x%04X, Badge ID: 0x%08X\n", company_id, badge_id);
    return false;
}

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
                         struct net_buf_simple *ad)
{
    char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));

    LOG_INF("Device found: %s (RSSI %d, type = 0x%02X)\n", addr_str, rssi, type);

    if (type == BT_GAP_ADV_TYPE_ADV_IND || type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND)
    {
        LOG_INF("  → Connectable advertising\n");
    }

    if (type == BT_GAP_ADV_TYPE_ADV_SCAN_IND || type == BT_GAP_ADV_TYPE_SCAN_RSP)
    {
        LOG_INF("  → Scannable advertising\n");
    }

    if (type == BT_GAP_ADV_TYPE_SCAN_RSP)
    {
        LOG_INF("→ Ignoré : paquet Scan Response\n");
        return;
    }

    if (rssi < RSSI_THRESHOLD)
    {
        return;
    }

    bt_data_parse(ad, parse_manufacturer_data, NULL);

    if (badge_id != 0)
    {

        auth_worker_submit(badge_id);
    }

    badge_id = 0;
}

static void start_scan(void)
{
    int err = bt_le_scan_start(&scan_param, device_found);

    if (err)
    {
        LOG_INF("Scanning failed to start (err %d)\n", err);
        return;
    }

    LOG_INF("Scanning successfully started\n");
}

void bt_scan_thread_fn(void *a, void *b, void *c)
{
    int err = bt_enable(NULL);
    if (err)
    {
        LOG_INF("Bluetooth init failed (err %d)\n", err);
        return;
    }

    LOG_INF("Bluetooth initialized\n");
    start_scan();

    while (1)
    {
        k_sleep(K_SECONDS(1));
    }
}

void bt_scan_thread_start(void)
{
    k_thread_create(&scan_thread, scan_stack_area, SCAN_STACK_SIZE,
                    bt_scan_thread_fn, NULL, NULL, NULL,
                    SCAN_THREAD_PRIORITY, 0, K_NO_WAIT);
}
