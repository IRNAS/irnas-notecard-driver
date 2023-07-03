/** @file notecard_i2c.c
 *
 * @brief
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2022 Irnas. All rights reserved.
 * Author: Marko Sagadin <marko@irnas.eu>
 */

#include "notecard_private.h"

#if NOTECARD_BUS_I2C

#include <zephyr/drivers/i2c.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/__assert.h>

#include <note.h>

#include <stddef.h>
#include <stdint.h>

static const size_t REQUEST_HEADER_SIZE = 2;

static const struct device *i2c_dev;

LOG_MODULE_DECLARE(note);

const char *notecard_i2c_receive(uint16_t device_address, uint8_t *buffer, uint16_t size,
				 uint32_t *available)
{
	/* Notecard might need some time to process previous request, 1ms seems to be sufficient. */
	k_msleep(1);

	/* Let the Notecard know that we are getting ready to read some data */
	uint8_t sizebuf[2] = {0, (uint8_t)size};

	if (i2c_write(i2c_dev, sizebuf, sizeof(sizebuf), device_address) != 0) {
		return "i2c: Unable to initate read from the Notecard\n";
	}

	/* Read from the Notecard and copy the response bytes into the response buffer */
	uint8_t read_buf[256];

	if (i2c_read(i2c_dev, read_buf, size + REQUEST_HEADER_SIZE, device_address) != 0) {
		return "i2c: Unable to receive data from the Notecard.\n";
	} else {
		*available = (uint32_t)read_buf[0];
		uint8_t bytes_to_read = read_buf[1];
		for (size_t i = 0; i < bytes_to_read; i++) {
			buffer[i] = read_buf[i + 2];
		}

		return NULL;
	}
}

bool notecard_i2c_reset(uint16_t device_address)
{
	/* Nothing is needed here */
	ARG_UNUSED(device_address);

	return true;
}

const char *notecard_i2c_transmit(uint16_t device_address, uint8_t *buffer, uint16_t size)
{
	__ASSERT(size < 256, "i2c transmit size needs to be less than 256");

	uint8_t write_buf[256];

	write_buf[0] = (uint8_t)size;

	for (size_t i = 0; i < size; i++) {
		write_buf[i + 1] = buffer[i];
	}

	/* Notecard might need some time to process previous request, 1ms seems to be sufficient. */
	k_msleep(1);

	return i2c_write(i2c_dev, write_buf, size + 1, device_address) == 0
		       ? NULL
		       : "i2c: Unable to transmit data to the Notecard\n";
}

void notecard_i2c_attach_bus_api(const struct notecard_bus *bus)
{
	i2c_dev = bus->dev.i2c.bus;

	/* Not setting this to true will add a ton of delays into transmit code. */
	NoteTurboIO(true);

	/* Give note-c uart hooks.
	 * Second argument tells note-c how large chunks can be send over i2c. */
	NoteSetFnI2C(bus->dev.i2c.addr, NOTE_I2C_MAX_MAX, notecard_i2c_reset, notecard_i2c_transmit,
		     notecard_i2c_receive);
}

#endif /* NOTECARD_BUS_I2C */
