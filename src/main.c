/* main.c - Application main entry point */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "scan_bt.h"
#include "auth_controller.h"
#include "led_feedback.h"
#include "wifi.h"
#include "http_client.h"
#include <zephyr/logging/log.h>
#include "auth_worker.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
    struct auth_result_msg msg;
    LOG_INF("BT - Central - Contrôle d'accès Module Integration\n");

    if (wifi_connect() != 0)
    {
        LOG_INF("Erreur Wi-Fi, arrêt.\n");
        return -1;
    }

    led_feedback_init();
    auth_controller_init();
    auth_worker_init();
    bt_scan_thread_start();

    while (1)
    {
        LOG_INF("Alive !");
        k_sleep(K_MSEC(2000));
    }

    return 0;
}