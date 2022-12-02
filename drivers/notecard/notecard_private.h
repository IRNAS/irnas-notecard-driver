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

#include <zephyr/drivers/i2c.h>

#define DT_DRV_COMPAT irnas_notecard

#define NOTECARD_BUS_UART DT_ANY_INST_ON_BUS_STATUS_OKAY(uart)
#define NOTECARD_BUS_I2C  DT_ANY_INST_ON_BUS_STATUS_OKAY(i2c)

union notecard_bus {
#if NOTECARD_BUS_UART
	const struct device *uart;
#endif
#if NOTECARD_BUS_I2C
	struct i2c_dt_spec i2c;
#endif
};

#if NOTECARD_BUS_UART
extern void notecard_uart_attach_bus_api(const union notecard_bus *bus);
#endif

#if NOTECARD_BUS_I2C
extern void notecard_i2c_attach_bus_api(const union notecard_bus *bus);
#endif

struct notecard_config {
	union notecard_bus bus;
	void (*attach_bus_api)(const union notecard_bus *bus);
};

#ifdef __cplusplus
}
#endif

#endif /* NOTECARD_PRIVATE_H */
