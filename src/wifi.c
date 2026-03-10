#include <zephyr/kernel.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/logging/log.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socketutils.h>
#include <zephyr/net/http/client.h>

LOG_MODULE_REGISTER(wifi, LOG_LEVEL_DBG);

#define WIFI_SSID "VOTRE SSID"
#define WIFI_CHANNEL 7
#define WIFI_SECURITY WIFI_SECURITY_TYPE_PSK
#define WIFI_PSK "VOTRE MDP WIFI"

static struct net_mgmt_event_callback wifi_mgmt_cb;
static struct net_if *iface;

static struct k_sem wifi_sem;

static void print_mac_address(void)
{
    uint8_t mac[6];
    int len = net_if_get_link_addr(iface)->len;

    if (len != 6)
    {
        LOG_WRN("Longueur MAC inattendue : %d", len);
        return;
    }

    memcpy(mac, net_if_get_link_addr(iface)->addr, 6);

    LOG_DBG("Adresse MAC : %02X:%02X:%02X:%02X:%02X:%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

static void print_ip_address(void)
{
    const struct in_addr *ip = net_if_ipv4_get_global_addr(iface, NET_ADDR_PREFERRED);
    if (ip)
    {
        char buf[NET_IPV4_ADDR_LEN];
        net_addr_ntop(AF_INET, ip, buf, sizeof(buf));
        LOG_INF("Adresse IP obtenue via DHCP : %s", buf);
    }
    else
    {
        LOG_WRN("Aucune adresse IPv4 disponible");
    }
}

static void wifi_event_handler(struct net_mgmt_event_callback *cb,
                               uint32_t event,
                               struct net_if *iface_event)
{
    if (event == NET_EVENT_WIFI_CONNECT_RESULT)
    {
        LOG_INF("Wi-Fi connecté");
        k_sleep(K_SECONDS(2)); // Laisser le temps au DHCP
        print_mac_address();
        print_ip_address();
        k_sem_give(&wifi_sem);
    }
    else if (event == NET_EVENT_WIFI_DISCONNECT_RESULT)
    {
        LOG_INF("Wi-Fi déconnecté");
    }
}
/**
 * @brief Initialise et lance la connexion Wi‑Fi.
 * @return 0 si succès, <0 sinon
 */
int wifi_connect(void)
{
    iface = net_if_get_wifi_sta();
    struct wifi_connect_req_params params = {
        .ssid = WIFI_SSID,
        .ssid_length = strlen(WIFI_SSID),
        .channel = WIFI_CHANNEL,
        .security = WIFI_SECURITY,
        .psk = WIFI_PSK,
        .psk_length = strlen(WIFI_PSK),
    };

    k_sem_init(&wifi_sem, 0, 1);
    net_mgmt_init_event_callback(&wifi_mgmt_cb,
                                 wifi_event_handler,
                                 NET_EVENT_WIFI_CONNECT_RESULT);
    net_mgmt_add_event_callback(&wifi_mgmt_cb);

    LOG_INF("Demande de connexion au SSID '%s'...\n", WIFI_SSID);
    int err = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface,
                       &params, sizeof(params));
    if (err)
    {
        LOG_INF("net_mgmt a retourné une erreur: %d\n", err);
        return err;
    }

    if (k_sem_take(&wifi_sem, K_SECONDS(10)))
    {
        LOG_INF("Timeout de connexion Wi‑Fi\n");
        return -ETIMEDOUT;
    }

    return 0;
}
