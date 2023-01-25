/** @file main.c
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2022 Irnas. All rights reserved.
 * Author: Marko Sagadin <marko@irnas.eu>
 */

#include <notecard.h>

#include <note.h>

#include <zephyr/device.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>

#include <stdlib.h>

LOG_MODULE_REGISTER(main);

const struct device *notecard_dev = DEVICE_DT_GET(DT_NODELABEL(notecard));

void main(void)
{
	LOG_INF("Booted");

	/* Create a request */
	while (1) {
		notecard_ctrl_take(notecard_dev);
		J *req = NoteNewRequest("card.aux");
		JAddStringToObject(req, "mode", "gpio");

		const char *aux_pins_usage[] = {"off", "off", "off", "off"};
		J *str_arr = JCreateStringArray(aux_pins_usage, 4);
		JAddItemToObject(req, "usage", str_arr);

		J *rsp = NoteRequestResponse(req);
		char *rsp_str = JPrint(rsp);
		if (rsp) {
			LOG_INF("%s", rsp_str);
			JFree(rsp_str);
		} else {
			LOG_INF("No response");
		}

		NoteDeleteResponse(rsp);

		notecard_ctrl_release(notecard_dev);
		k_msleep(5000);
	}
}
