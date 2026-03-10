#ifndef AUTH_WORKER_H
#define AUTH_WORKER_H

#include <stdint.h>

void auth_worker_init(void);
void auth_worker_submit(uint32_t badge_id);

#endif // AUTH_WORKER_H
