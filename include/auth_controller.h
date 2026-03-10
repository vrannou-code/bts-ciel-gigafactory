#ifndef AUTH_CONTROLLER_H
#define AUTH_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>
#include <zephyr/kernel.h>

struct auth_result_msg {
    uint32_t badge_id;
    bool access_granted;
};

void auth_controller_init(void);
bool auth_handle_badge_id(uint32_t badge_id);
struct k_msgq *auth_get_msgq(void);

#endif // AUTH_CONTROLLER_H