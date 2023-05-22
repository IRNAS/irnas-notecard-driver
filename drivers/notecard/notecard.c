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
 *
 * Below logic cleans up the message by removing trailing whitespace characters before printing.
 */
static size_t zephyr_log_print(const char *message)
{
	char cleaned_msg[256];
	// size_t bytes_left = strlen(message);
	// size_t chunk_size = 255;
	// int i = 0;
	// while (bytes_left) {
	// 	/* Handle the case where the size of the image sent is not divisable with
	// 	 * chunk_size.
	// 	 */
	// 	size_t bytes_to_send = bytes_left >= chunk_size ? chunk_size : bytes_left;
	// 	strncpy(cleaned_msg, &message[i * chunk_size], bytes_to_send);
	// 	cleaned_msg[bytes_to_send] = '\0';
	// 	LOG_RAW("%s", cleaned_msg);
	// 	bytes_left -= bytes_to_send;
	// 	i++;
	// }

	if (strlen(message) > 256) {
		strncpy(cleaned_msg, message, 256);
		cleaned_msg[255] = '\0';
		LOG_DBG("%s", cleaned_msg);
		return 0;
	}

	char const *end = message;

	while (!(*end == '\0' || *end == '\r')) {
		end++;
	}

	strncpy(cleaned_msg, message, end - message);
	cleaned_msg[end - message] = '\0';

	if (strlen(cleaned_msg) > 0) {
		LOG_DBG("%s", cleaned_msg);
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
	if (attn_pin_state && data->attn_cb_data.cb) {
		data->attn_cb_data.cb(dev, data->attn_cb_data.user_data);
	}

	/* "Re-enable back" interrupt, but for a different level */
	gpio_pin_interrupt_configure_dt(&config->attn_p_gpio, attn_pin_state
								      ? GPIO_INT_LEVEL_INACTIVE
								      : GPIO_INT_LEVEL_ACTIVE);
}

static int prv_configure_interrupt_gpio(struct gpio_callback *gpio_cb,
					const struct gpio_dt_spec *gpio)
{
	int rc;
	/* Configure GPIO interrupt pin */
	if (!device_is_ready(gpio->port)) {
		LOG_ERR("attn_p_gpio is not ready");
		return -ENODEV;
	}

	rc = gpio_pin_configure_dt(gpio, GPIO_INPUT);
	if (rc) {
		LOG_ERR("failed to configure attn_p_gpio pin %d (err=%d)", gpio->pin, rc);
		return rc;
	}

	/* Prepare GPIO callback for interrupt pin */
	gpio_init_callback(gpio_cb, attn_pin_cb_handler, BIT(gpio->pin));

	rc = gpio_add_callback(gpio->port, gpio_cb);
	if (rc) {
		LOG_ERR("failed to add callback (err=%d)", rc);
		return rc;
	}
	/* Regardles to what the trigger is set, we do not want interrupt to fire at init, below
	 * line ensures that. */
	gpio_flags_t interrupt_type =
		gpio_pin_get_dt(gpio) ? GPIO_INT_LEVEL_INACTIVE : GPIO_INT_LEVEL_ACTIVE;

	rc = gpio_pin_interrupt_configure_dt(gpio, interrupt_type);
	if (rc) {
		LOG_ERR("failed to configure attn_p_gpio pin %d as interrupt (err=%d)", gpio->pin,
			rc);
	}

	return rc;
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

	return config->attn_gpio_in_use
		       ? prv_configure_interrupt_gpio(&data->gpio_cb, &config->attn_p_gpio)
		       : 0;
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

size_t notecard_available_memory(void)
{
	struct object_header {
		struct object_header *prev;
		size_t length;
	};

	/*  Allocate progressively smaller and smaller chunks */
	struct object_header *prev_obj = NULL;
	static long int max_size = 8192;
	for (long int i = max_size; i >= (long int)sizeof(struct object_header);
	     i = i - sizeof(struct object_header)) {

		while (1) {
			struct object_header *obj;
			obj = k_heap_alloc(&notecard_heap, i, K_NO_WAIT);
			if (obj == NULL) {
				break;
			}
			obj->prev = prev_obj;
			obj->length = i;
			prev_obj = obj;
		}
	}

	/* Free the objects backwards */
	size_t total = 0;
	while (prev_obj != NULL) {
		struct object_header *obj = prev_obj;
		prev_obj = obj->prev;
		total += obj->length;
		k_heap_free(&notecard_heap, obj);
	}

	return total;
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
