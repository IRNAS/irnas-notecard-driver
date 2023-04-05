/** @file notecard.c
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2022 Irnas. All rights reserved.
 * Author: Marko Sagadin <marko@irnas.eu>
 */
#include "notecard_private.h"

#include <notecard.h>

#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>

#include <note.h>

#include <stdlib.h>

LOG_MODULE_REGISTER(note, CONFIG_NOTECARD_LOG_LEVEL);

static uint8_t notecard_heap_memory[CONFIG_NOTECARD_HEAP_SIZE];
static struct k_heap notecard_heap;
static struct k_mutex notecard_mutex;

/**
 * @brief Zephyr-specific `delay` function required by the note-c lib.
 *
 * Puts the current thread to sleep.
 */
static void zephyr_delay(uint32_t ms)
{
	k_msleep(ms);
}

/**
 * @brief Zephyr-specific `milllis` function required by the note-c lib.
 *
 * @return Return value of k_uptime_get
 */
static uint32_t zephyr_millis(void)
{
	return (uint32_t)k_uptime_get();
}

/**
 * @brief Zephyr-specific `log print` function required by the note-c lib.
 */
static size_t zephyr_log_print(const char *message)
{
	if (message) {
		LOG_DBG("%s", message);
		return 1;
	}

	return 0;
}

/**
 * @brief Zephyr-specific `malloc` function required by the note-c lib.
 */
static void *zephyr_malloc(size_t size)
{
	void *ptr = k_heap_alloc(&notecard_heap, size, K_NO_WAIT);
	if (!ptr) {
		LOG_ERR("Memory allocation failed!");
	}

	return ptr;
}

/**
 * @brief Zephyr-specific `free` function required by the note-c lib.
 */
static void zephyr_free(void *mem)
{
	k_heap_free(&notecard_heap, mem);
}

static void attn_pin_cb_handler(const struct device *port, struct gpio_callback *cb,
				gpio_port_pins_t pins)
{
	struct notecard_data *data = CONTAINER_OF(cb, struct notecard_data, gpio_cb);
	const struct device *dev = data->dev;
	const struct notecard_config *config = dev->config;

	/* Disable interrupt */
	gpio_pin_interrupt_configure_dt(&config->attn_p_gpio, GPIO_INT_DISABLE);

	int attn_pin_state = gpio_pin_get_dt(&config->attn_p_gpio);

	/* Call callback only, if the state changed, attn pin state is high and callback was
	 * given.*/
	if (data->prev_attn_pin_state != attn_pin_state) {
		data->prev_attn_pin_state = attn_pin_state;
		if (attn_pin_state && data->attn_cb_data.cb) {
			data->attn_cb_data.cb(dev, data->attn_cb_data.user_data);
		}
	}

	/* "Re-enable back" interrupt, but for a different level */
	gpio_pin_interrupt_configure_dt(&config->attn_p_gpio, data->prev_attn_pin_state
								      ? GPIO_INT_LEVEL_INACTIVE
								      : GPIO_INT_LEVEL_ACTIVE);
}

static int configure_attn_p_gpio(const struct device *dev)
{
	const struct notecard_config *config = dev->config;
	struct notecard_data *data = dev->data;

	/* Configure GPIO interrupt pin */
	if (!device_is_ready(config->attn_p_gpio.port)) {
		LOG_ERR("GPIO for attn_p_gpio not ready");
		return -ENODEV;
	}

	int rc = gpio_pin_configure_dt(&config->attn_p_gpio, GPIO_INPUT);
	if (rc) {
		LOG_ERR("failed to configure attn_p_gpio pin %d (err=%d)", config->attn_p_gpio.pin,
			rc);
		return rc;
	}

	/* Prepare GPIO callback for interrupt pin */
	gpio_init_callback(&data->gpio_cb, attn_pin_cb_handler, BIT(config->attn_p_gpio.pin));

	rc = gpio_add_callback(config->attn_p_gpio.port, &data->gpio_cb);
	if (rc) {
		LOG_ERR("failed to add callback (err=%d)", rc);
		return rc;
	}

	rc = gpio_pin_interrupt_configure_dt(&config->attn_p_gpio, GPIO_INT_LEVEL_ACTIVE);
	if (rc) {
		LOG_ERR("failed to configure attn_p_gpio pin %d as interrupt (err=%d)",
			config->attn_p_gpio.pin, rc);
		return rc;
	}

	return 0;
}

static int notecard_init(const struct device *dev)
{
	k_heap_init(&notecard_heap, notecard_heap_memory, CONFIG_NOTECARD_HEAP_SIZE);
	k_mutex_init(&notecard_mutex);

	NoteSetFnDebugOutput(zephyr_log_print);

	/* Set platform specific hooks. */
	NoteSetFn(zephyr_malloc, zephyr_free, zephyr_delay, zephyr_millis);

	struct notecard_data *data = dev->data;
	const struct notecard_config *config = dev->config;

	/* Dev is later required in notecard_delayed_work_handler and attn_pin_cb_handler, cause it
	 * can not be fetched with CONTAINTER_OF macro. */
	data->dev = dev;

	/* Configure interrupt pin */
	return config->attn_gpio_in_use ? configure_attn_p_gpio(dev) : 0;
}

void notecard_ctrl_take(const struct device *dev)
{
	k_mutex_lock(&notecard_mutex, K_FOREVER);
	struct notecard_data *data = dev->data;

	if (data->post_take_cb_data.cb) {
		data->post_take_cb_data.cb(dev, data->post_take_cb_data.user_data);
	}

	const struct notecard_config *config = dev->config;
	config->bus.attach_bus_api(&config->bus);
}

void notecard_ctrl_release(const struct device *dev)
{
	struct notecard_data *data = dev->data;

	if (data->pre_release_cb_data.cb) {
		data->pre_release_cb_data.cb(dev, data->pre_release_cb_data.user_data);
	}

	k_mutex_unlock(&notecard_mutex);
}

void notecard_attn_cb_register(const struct device *dev, notecard_cb_t attn_cb, void *user_data)
{
	__ASSERT(attn_cb, "Callback pointer needs to be provided");

	const struct notecard_config *config = dev->config;
	__ASSERT(config->attn_gpio_in_use, "attn-p-gpio was not provided in dts");
	ARG_UNUSED(config);

	/* Set callback and user data */
	struct notecard_data *data = dev->data;

	data->attn_cb_data.cb = attn_cb;
	data->attn_cb_data.user_data = user_data;
}

void notecard_post_take_cb_register(const struct device *dev, notecard_cb_t post_take_cb,
				    void *user_data)
{
	__ASSERT(post_take_cb, "Callback pointer needs to be provided");

	/* Set callback and user data */
	struct notecard_data *data = dev->data;

	data->post_take_cb_data.cb = post_take_cb;
	data->post_take_cb_data.user_data = user_data;
}

void notecard_pre_release_cb_register(const struct device *dev, notecard_cb_t pre_release_cb,
				      void *user_data)
{
	__ASSERT(pre_release_cb, "Callback pointer needs to be provided");

	/* Set callback and user data */
	struct notecard_data *data = dev->data;

	data->pre_release_cb_data.cb = pre_release_cb;
	data->pre_release_cb_data.user_data = user_data;
}

#define DT_DRV_COMPAT blues_notecard

#define NOTECARD_CONFIG_UART(inst)                                                                 \
	{                                                                                          \
		.dev.uart = DEVICE_DT_GET(DT_BUS(DT_DRV_INST(inst))),                              \
		.attach_bus_api = notecard_uart_attach_bus_api,                                    \
	}

#define NOTECARD_CONFIG_I2C(inst)                                                                  \
	{                                                                                          \
		.dev.i2c = I2C_DT_SPEC_INST_GET(inst),                                             \
		.attach_bus_api = notecard_i2c_attach_bus_api,                                     \
	}

#define NOTECARD_DEFINE(inst)                                                                      \
	static const struct notecard_config notecard_config_##inst = {                             \
		.bus = COND_CODE_1(DT_INST_ON_BUS(inst, uart), (NOTECARD_CONFIG_UART(inst)),       \
				   (NOTECARD_CONFIG_I2C(inst))),                                   \
		.attn_p_gpio = GPIO_DT_SPEC_INST_GET_OR(inst, attn_p_gpios, {}),                   \
		.attn_gpio_in_use = DT_INST_NODE_HAS_PROP(inst, attn_p_gpios)                      \
                                                                                                   \
	};                                                                                         \
                                                                                                   \
	static struct notecard_data notecard_data_##inst;                                          \
                                                                                                   \
	DEVICE_DT_INST_DEFINE(inst, &notecard_init, NULL, &notecard_data_##inst,                   \
			      &notecard_config_##inst, POST_KERNEL, CONFIG_NOTECARD_INIT_PRIORITY, \
			      NULL);

DT_INST_FOREACH_STATUS_OKAY(NOTECARD_DEFINE);
