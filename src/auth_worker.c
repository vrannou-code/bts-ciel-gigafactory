#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "auth_controller.h"

LOG_MODULE_REGISTER(auth_worker, LOG_LEVEL_INF);

#define QUEUE_SIZE 5
K_MSGQ_DEFINE(badge_queue, sizeof(uint32_t), QUEUE_SIZE, 4);

#define STACK_SIZE 1024
#define PRIORITY 5
K_THREAD_STACK_DEFINE(worker_stack, STACK_SIZE);
static struct k_thread worker_thread;

static void auth_worker_thread(void *a, void *b, void *c)
{
    uint32_t badge_id;

    while (1)
    {
        if (k_msgq_get(&badge_queue, &badge_id, K_FOREVER) == 0)
        {
            LOG_INF("Traitement déporté du badge 0x%08X", badge_id);
            auth_handle_badge_id(badge_id);
        }
    }
}

void auth_worker_init(void)
{
    k_thread_create(&worker_thread, worker_stack, STACK_SIZE,
                    auth_worker_thread, NULL, NULL, NULL,
                    PRIORITY, 0, K_NO_WAIT);
}

void auth_worker_submit(uint32_t badge_id)
{
    if (k_msgq_put(&badge_queue, &badge_id, K_NO_WAIT) != 0)
    {
        LOG_WRN("Queue saturée, badge ignoré: 0x%08X", badge_id);
    }
}
