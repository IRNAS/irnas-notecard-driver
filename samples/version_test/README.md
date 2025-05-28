# Version test sample

Sample was tested on `nrf52840_nrf52840` board.

Two different configurations are possible:

```bash
# If using i2c communication
east build -b nrf52840dk/nrf52840 -- -DDTC_OVERLAY_FILE=notecard_over_i2c.overlay

# If using uart communication
east build -b nrf52840dk/nrf52840 -- -DDTC_OVERLAY_FILE=notecard_over_uart.overlay
```
