zephyr_library()

file(GLOB sources note-c/*.c)
zephyr_library_sources(${sources})
zephyr_include_directories(note-c)
