# IRNAS' NoteCard Driver

This repository contains a Zephyr driver for Blues' Notecard modules.

## Important

Please read the documentation in [notecard.h](./drivers/notecard/notecard.h) and
see how to use the API in the samples.

This driver is still work in progress!.

## Setup

If not already set up, install west and other required tools. Follow the steps
[here](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_installing.html)
from
[Install the required tools](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_installing.html#install-the-required-tools)
up to (including)
[Install west](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/nrf/gs_installing.html#install-the-required-tools).

Then follow these steps:

```bash
west init -m https://github.com/IRNAS/<repo-name> <repo-name>
cd <repo-name>/
west update
# remember to source zephyr env
source zephyr/zephyr-env.sh
```
