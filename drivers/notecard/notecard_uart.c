/** @file notecard_uart.c
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2022 Irnas. All rights reserved.
 * Author: Marko Sagadin <marko@irnas.eu>
 */

#include "notecard_private.h"

#if NOTECARD_BUS_UART

#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

#include <note.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SERIAL_PEEK_EMPTY_MASK 0xFF00

static uint16_t prv_peek_buf = SERIAL_PEEK_EMPTY_MASK;

/* Local pointer to the uart device that is currently used for communication  */
static const struct device *prv_uart_dev;

LOG_MODULE_REGISTER(notecard_uart);

static bool prv_rx_available(void)
{
	bool result;

	if (SERIAL_PEEK_EMPTY_MASK & prv_peek_buf) {
		/* Peek buffer is empty */
		unsigned char next_char;
		int status = uart_poll_in(prv_uart_dev, &next_char);
		switch (status) {
		case 0:
			prv_peek_buf = next_char;
			result = true;
			break;
		default:
			/* Nothing to poll, or some error was thrown by uart_poll_in */
			result = false;
			break;
		}
	} else {
		/* Peek buffer is full */
		result = true;
	}
	return result;
}

static char prv_receive(void)
{
	char result;
	if (!(SERIAL_PEEK_EMPTY_MASK & prv_peek_buf)) {
		/* Peek buffer is full */
		result = prv_peek_buf;
		prv_peek_buf = SERIAL_PEEK_EMPTY_MASK;
	} else if (uart_poll_in(prv_uart_dev, (unsigned char *)&result)) {
		result = '\0';
	}

	return result;
}

static bool prv_reset(void)
{
	char c;

	while (!uart_poll_in(prv_uart_dev, &c)) {
	}

	return true;
}

static void prv_transmit(uint8_t *text_, size_t len_, bool flush_)
{
	ARG_UNUSED(flush_); /* `uart_poll_out` blocks (i.e. always flushes) */

	for (size_t i = 0; i < len_; ++i) {
		uart_poll_out(prv_uart_dev, text_[i]);
		/* 100 us delay is needed to prevent overwhelming the nrfx implementation of
		 * uart_poll_out and entering 1ms sleep between each call. */
		k_busy_wait(100);
	}
}

void notecard_uart_attach_bus_api(const struct notecard_bus *bus)
{
	prv_uart_dev = bus->dev.uart;

	/* Give note-c uart hooks. */
	NoteSetFnSerial(prv_reset, prv_transmit, prv_rx_available, prv_receive);
}

#endif /* NOTECARD_BUS_UART */
