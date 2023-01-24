# IRNAS' NoteCard Driver

This repository contains a Zephyr driver for Blues' Notecard modules.

## Important

Please read the documentation in [notecard.h](./drivers/notecard/notecard.h) and
see how to use the API in the samples.

## Usage

To use add below two snippets into your project's west.yml file and run
`west update`:

1. In `remotes` section, if not already added:

```yaml
- name: irnas
  url-base: https://github.com/irnas
```

2. In the `projects` section, select revision you need:

```yaml
- name: irnas-notecard-driver
    repo-path: irnas-notecard-driver
    path: irnas/irnas-notecard-driver
    remote: irnas
    revision: v1.0.0
```

### Device tree

To enable this driver you need to add one (or both) of the below snippets into
your DTS or overlay file, other fields (besides label) should not change.

Check the `dts/bindings` for other possible properties.

For i2c communication:

```yaml
&i2c0 {
	notecard: notecard@17 {
		compatible = "irnas,notecard";
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
		compatible = "irnas,notecard";
	};
};
```

### Kconfig

No extra Kconfig options need to be enabled to use this driver, adding above
snippet in device tree is enough. Check the
[`driver/notecard/Kconfig`](./driver/notecard/Kconfig) file for other available
Kconfig options.
