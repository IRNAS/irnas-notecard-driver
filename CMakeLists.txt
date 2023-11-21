# This CMake file is picked by the Zephyr build system because it is defined as
# the module CMake entry point (see zephyr/module.yml).
add_subdirectory(drivers)
add_subdirectory_ifdef(CONFIG_NOTE_C_LIB third-party)
