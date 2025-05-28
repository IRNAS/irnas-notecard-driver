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

LOG_MODULE_REGISTER(main);

const struct device *notecard_dev = DEVICE_DT_GET(DT_NODELABEL(notecard));

void callback(const struct device *dev, void *user_data)
{
	const struct device *user_dev = user_data;
	LOG_INF("Hello from %s", user_dev->name);
}

int main(void)
{
	/* In last argument pass the device as user data just for demonstration purposes. */
	notecard_attn_cb_register(notecard_dev, callback, (void *)notecard_dev);

	/* Create a request */
	while (1) {
		/* Arm attn pin (it will go low) and set it to timeout after 1 second (it goes high
		 * then). */
		notecard_ctrl_take(notecard_dev);
		J *req = NoteNewRequest("card.attn");
		JAddStringToObject(req, "mode", "arm");
		JAddNumberToObject(req, "seconds", 1);

		J *rsp = NoteRequestResponseWithRetry(req, 3);
		char *rsp_str = JPrint(rsp);
		if (rsp) {
			LOG_INF("%s", rsp_str);
			JFree(rsp_str);
		} else {
			LOG_INF("No response");
		}
		NoteDeleteResponse(rsp);

		notecard_ctrl_release(notecard_dev);
		LOG_INF("Sleeping for 3 s");
		k_msleep(3000);
	}
}
