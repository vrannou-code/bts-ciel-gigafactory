// led_feedback.c
#include "led_feedback.h"
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(led_feedback, LOG_LEVEL_INF);

#define LED_OK_NODE DT_ALIAS(led0)
#define LED_KO_NODE DT_ALIAS(led1)

static const struct gpio_dt_spec led_ok = GPIO_DT_SPEC_GET(LED_OK_NODE, gpios);
static const struct gpio_dt_spec led_ko = GPIO_DT_SPEC_GET(LED_KO_NODE, gpios);

void led_feedback_init(void)
{
    if (!device_is_ready(led_ok.port) || !device_is_ready(led_ko.port))
    {
        LOG_ERR("LED devices not ready");
        return;
    }

    gpio_pin_configure_dt(&led_ok, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led_ko, GPIO_OUTPUT_INACTIVE);
}

void led_show_access_ok(void)
{
    gpio_pin_set_dt(&led_ok, 1);
    gpio_pin_set_dt(&led_ko, 0);
}

void led_show_access_denied(void)
{
    gpio_pin_set_dt(&led_ok, 0);
    gpio_pin_set_dt(&led_ko, 1);
}

void led_show_access_off(void)
{
    gpio_pin_set_dt(&led_ok, 0);
    gpio_pin_set_dt(&led_ko, 0);
}