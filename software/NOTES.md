# General Notes

This document is the equivalent of the lab book for the software.

## 30/07/2023

The [LEDC](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html) driver controls the PWM outputs, which will be used to generate the in-phase and quadrature local oscillators for the (switching) mixers. The esp32-c3 has no analog output, so a smoothed PWM output will also be used for automatic gain control.

Copied the `peripherals/ledc/ledc_basic` example to this folder to test out whether it can produce two out-of-phase signals for the local oscillators. The technical reference manual (chapter 32 LED PWM Controller (LEDC)) states that output signals can have adjustable phases.

Ran

```bash
. export.sh
cd ledc_basic
idf.py set-target esp32c3 # This line failed, and then I didn't find it in the readme, so maybe it isn't necessary
idf.py build
```

The build worked fine, and I changed the gpio to 8 (trying to preserve pins from another project). I found that when I ran `set-target` again, it worked, so maybe it was the `idf.py full-clean` that I did that fixed it. To flash, I ran:

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

That worked straight away!
