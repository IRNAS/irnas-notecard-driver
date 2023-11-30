# IRNAS' NoteCard Driver

This repository contains a Zephyr driver for Blues' Notecard modules.

## Important

Please read the documentation in [notecard.h](./drivers/include/notecard.h) and
see how to use the API in the samples.

## Usage

To use add below two snippets into your project's west.yml file and run
`west update`:

1. In `remotes` section, if not already added:

```yaml
- name: irnas
  url-base: https://github.com/irnas
```

2. In the `projects` section, (select revision you need, latest is recommended):

```yaml
- name: irnas-notecard-driver
    repo-path: irnas-notecard-driver
    path: irnas/irnas-notecard-driver
    remote: irnas
    revision: <revision>
    submodules: true
```

### Device tree

To enable this driver you need to add one (or both) of the below snippets into
your DTS or overlay file, other fields (besides label) should not change.

Check the `dts/bindings` for other possible properties.

For i2c communication:

```yaml
&i2c0 {
    notecard: notecard@17 {
        compatible = "blues,notecard";
        reg = <0x17>;
    };
};
```

For uart communication:

```yaml
&uart0 {
    compatible = "nordic,nrf-uarte";
    current-speed = <115200>;
    pinctrl-0 = <&uart0_default>;
    pinctrl-1 = <&uart0_sleep>;
    pinctrl-names = "default", "sleep";

    notecard: notecard {
        compatible = "blues,notecard";
    };
};
```

### Kconfig

No extra Kconfig options need to be enabled to use this driver, adding above
snippet in device tree is enough. Check the
[`driver/notecard/Kconfig`](./driver/notecard/Kconfig) file for other available
Kconfig options.

### MCUBoot

When using MCUBoot in the project you probably do not want to compile Notecard
driver in the bootloader image.

To accomplish this create `child_image` folder in your application folder.
Inside it create file `mcuboot.conf` with below content:

```
# This Kconfig fragment is merged when building mcuboot image.

# Explicitly disable some drivers, they are otherwise enabled implicitly
# through dts.
CONFIG_NOTECARD=n
```

## `note-c` library

This driver depends on a `note-c` library which is contained in the project as a
git submodule. Whenever this driver is enabled (located in the device tree)
`note-c` is automatically added to the project.

If user has the need to only have the `note-c` library in the project, but not
the Notecard driver, he or she can enable `CONFIG_NOTE_C_LIB` Kconfig symbol to
get access just to the `note-c` library in the project.

He or she might be writing unit tests that test the JSON parsing logic on the
`native_posx` board, where Notecard driver is not needed.
