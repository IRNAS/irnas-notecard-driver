/** @file notecard.c
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2022 Irnas. All rights reserved.
 * Author: Marko Sagadin <marko@irnas.eu>
 */
#include "private/notecard_private.h"

#include <notecard.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <note.h>

#include <stdlib.h>

LOG_MODULE_REGISTER(note);

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
		LOG_INF("%s", message);
		return 1;
	}

	return 0;
}

/**
 * @brief Zephyr-specific `malloc` function required by the note-c lib.
 */
static void *zephyr_malloc(size_t size)
{
	void *ptr = k_heap_alloc(&notecard_heap, size, K_FOREVER);
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

static int notecard_init(const struct device *dev)
{
	k_heap_init(&notecard_heap, notecard_heap_memory, CONFIG_NOTECARD_HEAP_SIZE);
	k_mutex_init(&notecard_mutex);

	NoteSetFnDebugOutput(zephyr_log_print);

	/* Set platform specific hooks. */
	NoteSetFn(zephyr_malloc, zephyr_free, zephyr_delay, zephyr_millis);
	return 0;
}

void notecard_ctrl_take(const struct device *dev)
{
	k_mutex_lock(&notecard_mutex, K_FOREVER);

	const struct notecard_config *config = dev->config;
	config->attach_bus_api(&config->bus);
}

void notecard_ctrl_release(const struct device *dev)
{
	k_mutex_unlock(&notecard_mutex);
}

#define NOTECARD_CONFIG_UART(inst)                                                                 \
	{                                                                                          \
		.bus.uart = DEVICE_DT_GET(DT_BUS(DT_DRV_INST(inst))),                              \
		.attach_bus_api = notecard_uart_attach_bus_api,                                    \
	}

#define NOTECARD_CONFIG_I2C(inst)                                                                  \
	{                                                                                          \
		.bus.i2c = I2C_DT_SPEC_INST_GET(inst),                                             \
		.attach_bus_api = notecard_i2c_attach_bus_api,                                     \
	}

#define NOTECARD_DEFINE(inst)                                                                      \
	static const struct notecard_config notecard_config_##inst =                               \
		COND_CODE_1(DT_INST_ON_BUS(inst, uart), (NOTECARD_CONFIG_UART(inst)),              \
			    (NOTECARD_CONFIG_I2C(inst)));                                          \
                                                                                                   \
	DEVICE_DT_INST_DEFINE(inst, &notecard_init, NULL, NULL, &notecard_config_##inst,           \
			      POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, NULL);

DT_INST_FOREACH_STATUS_OKAY(NOTECARD_DEFINE);
