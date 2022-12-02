/** @file notecard.h
 *
 * @brief Driver for Blues Wireless' Notecard communication modules.
 *
 * This driver depends on the usage of note-c library provided by the Blues Wireless:
 * https://github.com/blues/note-c
 *
 * This driver supports communication with connected notecard over i2c or uart.
 *
 * Due to the design of note-c library there are several limitations / things to keep in mind that
 * affect how developer should use this driver:
 *
 * - Both note-c and cJSON library (used by note-c) require dynamic memory to store communication
 *   data, create json objects and json strings. This driver uses Zephyr's k_heap to dynamically
 *   allocate memory when needed. The maximum amount of heap is statically allocated and can be
 *   configured with CONFIG_NOTECARD_HEAP_SIZE Kconfig option.
 *
 * - Although is possible to have two or more notecards connected to the board, it is not possible
 *   to talk to them at concurrently (even if they are on attached to different communication
 *   buses). Entire note-c lib is written with one notecard in mind as all context data (platform
 *   specific hooks) is stored in global variables, instead of note-c functions taking a context
 *   pointer.
 *
 * - To solve this issue this driver introduces a concept of control ownership. Driver can give
 *   control to only one device at a time. When device receives control its communication bus is
 *   activated and use of note-c functions that requires communication is possible. After the device
 *   is done with communication it has to give back control to the driver, thus deactivating its
 *   communication bus.
 *
 * - List of function that try to start communication with the notecard (may not be complete):
 *	- NoteRequest
 *	- NoteRequestWithRetry
 *	- NoteRequestResponse
 *	- NoteRequestResponseWithRetry
 * @par
 * COPYRIGHT NOTICE: (c) 2022 Irnas. All rights reserved.
 * Author: Marko Sagadin <marko@irnas.eu>
 */

#ifndef NOTECARD_H
#define NOTECARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/device.h>
/**
 * @brief Take control with the notecard device
 *
 * This function should be always called before calling any devices that try to communicate with the
 * notecard.
 *
 * If another notecard device tries to take control, this function will block until the first
 * notecard releses control.
 *
 * @param[in] device
 */
void notecard_ctrl_take(const struct device *device);

/**
 * @brief Release control from notecard device
 *
 * @param[in] device
 */
void notecard_ctrl_release(const struct device *device);

#ifdef __cplusplus
}
#endif

#endif /* NOTECARD_H */
