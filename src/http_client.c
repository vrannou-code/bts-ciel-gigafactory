#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>

#include "http_client.h"

#define HTTP_HOST "ADR IP DU SRV FLASK"
#define HTTP_PORT "5000"
#define HTTP_PATH "/check_badge"

#define DEVICE_ID "disco-zephyr"
#define ZONE "zone-admin"

#define PAYLOAD_BUFFER_SIZE 512
#define REQUEST_BUFFER_SIZE 512
#define RESPONSE_BUFFER_SIZE 512

static char payload[PAYLOAD_BUFFER_SIZE];
static char request[REQUEST_BUFFER_SIZE];
static char response[RESPONSE_BUFFER_SIZE];

LOG_MODULE_REGISTER(http_client, LOG_LEVEL_INF);

static int connect_http_server(const char *host, const char *port)
{
    struct addrinfo hints = {0}, *res;
    int sock, ret;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    ret = getaddrinfo(host, port, &hints, &res);
    if (ret != 0)
    {
        LOG_INF("getaddrinfo() failed: %d\n", ret);
        return -1;
    }

    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0)
    {
        LOG_INF("socket() failed\n");
        freeaddrinfo(res);
        return -1;
    }

    ret = connect(sock, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
    if (ret < 0)
    {
        LOG_INF("connect() failed\n");
        close(sock);
        return -1;
    }

    return sock;
}

bool http_query_badge(uint32_t badge_id)
{
    int ret, total = 0;
    LOG_INF("debut http_query_badge");

    snprintf(payload, sizeof(payload),
         "{\"badgeNumber\":\"%08X\","
         "\"device_id\":\"%s\","
         "\"zone\":\"%s\"}",
         badge_id, DEVICE_ID, ZONE);

    int len = snprintf(request, sizeof(request),
                       "POST " HTTP_PATH " HTTP/1.1\r\n"
                       "Host: " HTTP_HOST "\r\n"
                       "Content-Type: application/json\r\n"
                       "Content-Length: %zu\r\n"
                       "Connection: close\r\n"
                       "\r\n"
                       "%s",
                       strlen(payload), payload);

    if (len >= sizeof(request))
    {
        LOG_WRN("Request too large!");
        return false;
    }

    int sock = connect_http_server(HTTP_HOST, HTTP_PORT);
    if (sock < 0)
    {
        return false;
    }

    ret = send(sock, request, len, 0);
    if (ret <= 0)
    {
        LOG_WRN("send() failed");
        close(sock);
        return false;
    }

    k_msleep(200); // Laisse le temps à es-WiFi

    int64_t t_start = k_uptime_get();
    const int64_t timeout_ms = 3000;

    while (total < sizeof(response) - 1)
    {
        if (k_uptime_get() - t_start > timeout_ms)
        {
            LOG_WRN("HTTP timeout atteint, sortie de boucle recv()");
            break;
        }

        ret = recv(sock, response + total, sizeof(response) - 1 - total, MSG_DONTWAIT);

        if (ret == 0)
            break;
        if (ret < 0)
        {
            k_msleep(100);
            continue;
        }
        total += ret;
    }

    shutdown(sock, SHUT_RDWR);
    k_msleep(100);
    close(sock);
    k_msleep(500); // stable pour eswifi

    k_sleep(K_SECONDS(2));
    response[total] = '\0';

    // LOG_INF("Response:\n%s", response);
    LOG_INF("fin http_query_badge");
    if (strstr(response, "\"access\":true"))
    {
        return true;
    }

    return false;
}
