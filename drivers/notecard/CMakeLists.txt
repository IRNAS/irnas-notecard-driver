zephyr_library()

# Add note-c files
set(NOTE_C ${CMAKE_CURRENT_LIST_DIR}/../../third-party/note-c)

zephyr_library_sources(notecard.c notecard_uart.c notecard_i2c.c)
