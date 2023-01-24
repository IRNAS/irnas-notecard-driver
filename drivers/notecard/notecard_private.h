/** @file notecard_private.h
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2023 Irnas. All rights reserved.
 * Author: Marko Sagadin <marko@irnas.eu>
 */

#ifndef NOTECARD_PRIVATE_H
#define NOTECARD_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <notecard.h>

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>

#define DT_DRV_COMPAT	  blues_notecard
#define NOTECARD_BUS_UART DT_ANY_INST_ON_BUS_STATUS_OKAY(uart)
#define NOTECARD_BUS_I2C  DT_ANY_INST_ON_BUS_STATUS_OKAY(i2c)

union notecard_bus_device {
#if NOTECARD_BUS_UART
	const struct device *uart;
#endif
#if NOTECARD_BUS_I2C
	struct i2c_dt_spec i2c;
#endif
};

struct notecard_bus {
	union notecard_bus_device dev;
	void (*attach_bus_api)(const struct notecard_bus *bus);
};

#if NOTECARD_BUS_UART
extern void notecard_uart_attach_bus_api(const struct notecard_bus *bus);
#endif

#if NOTECARD_BUS_I2C
extern void notecard_i2c_attach_bus_api(const struct notecard_bus *bus);
#endif

struct notecard_config {
	struct notecard_bus bus;
	struct gpio_dt_spec attn_p_gpio;
	bool attn_gpio_in_use;
};

struct notecard_data {
	/* Internal gpio_cb structure */
	struct gpio_callback gpio_cb;
	/* Callback that was registered with notecard_attn_cb_register(). */
	attn_cb_t callback;
	/* User data that was given with callback registration. */
	void *user_data;
	/* Pointer to the container device. */
	const struct device *dev;

	/* State of the attn pin at previous edge transition, needed to properly debounce
	 * interrupts. */
	int prev_attn_pin_state;
};

#ifdef __cplusplus
}
#endif

#endif /* NOTECARD_PRIVATE_H */
