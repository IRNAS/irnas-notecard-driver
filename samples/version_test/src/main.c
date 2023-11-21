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

const struct device *prv_notecard_dev = DEVICE_DT_GET(DT_NODELABEL(notecard));

int main(void)
{
	LOG_INF("Booted");
	k_msleep(1000);

	notecard_ctrl_take(prv_notecard_dev);
	bool present = notecard_is_present(prv_notecard_dev);
	if (!present) {
		LOG_ERR("Notecard not present, stopping sample.");
		return 0;
	}

	LOG_INF("Notecard is present!");
	notecard_ctrl_release(prv_notecard_dev);

	/* Create a request */
	while (1) {
		notecard_ctrl_take(prv_notecard_dev);
		J *req = NoteNewRequest("card.version");
		J *rsp = NoteRequestResponse(req);
		if (rsp) {
			char *rsp_str = JPrint(rsp);
			LOG_INF("%s", rsp_str);
			NoteFree(rsp_str);
			NoteDeleteResponse(rsp);
		} else {
			LOG_INF("No response");
		}

		notecard_ctrl_release(prv_notecard_dev);
		k_msleep(1000);
	}
}
