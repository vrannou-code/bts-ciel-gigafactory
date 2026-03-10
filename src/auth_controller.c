
#include "auth_controller.h"
#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include "http_client.h"

LOG_MODULE_REGISTER(auth_controller, LOG_LEVEL_INF);

K_MSGQ_DEFINE(auth_msgq, sizeof(struct auth_result_msg), 10, 4);

void auth_controller_init(void) {}

bool auth_handle_badge_id(uint32_t badge_id)
{
    LOG_INF("debut auth_handle_badge_id");
    static int64_t last_auth_time = 0;
    static uint32_t last_badge_id = 0;
    const int64_t now = k_uptime_get();
    const int64_t AUTH_DEBOUNCE_MS = 5000;

    if (badge_id == last_badge_id && (now - last_auth_time < AUTH_DEBOUNCE_MS))
    {
        LOG_INF("Badge ignoré (déjà vu récemment): 0x%08X", badge_id);
        return false;
    }

    last_auth_time = now;
    last_badge_id = badge_id;

    // HYPER IMPORTANT POUR PAS QUE CA PLANTE !!!!
    k_msleep(100); // petite pause pour désamorcer radio

    bool granted = http_query_badge(badge_id);

    struct auth_result_msg msg = {
        .badge_id = badge_id,
        .access_granted = granted,
    };

    if (k_msgq_put(&auth_msgq, &msg, K_NO_WAIT) != 0)
    {
        LOG_WRN("auth msgq full, dropping badge_id 0x%08X", badge_id);
    }

    if (granted)
    {
        LOG_INF("Badge autorisé  (ID = 0x%08X)", badge_id);
    }
    else
    {
        LOG_INF("Badge refusé  (ID = 0x%08X)", badge_id);
    }
    LOG_INF("fin auth_handle_badge_id");

    return granted;
}

struct k_msgq *auth_get_msgq(void)
{
    return &auth_msgq;
}
