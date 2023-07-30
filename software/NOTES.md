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

That worked straight away! (678746c). 

The frequency of the output PWM for the local oscillators needs to be the RF frequency, which for AM is 535 kHz to 1605 kHz. The frequency output by this example is 5 kHz. The next step is to generate two signals out of phase by 90deg.

Reading through the LEDC API documentation, there seems no mention of adjusting the phase between two channels. It seems like maybe the phase is [not supported](https://esp32.com/viewtopic.php?t=30621) in LEDC, even though the hardware module supports it (according to the technical reference manual -- see in particular Figure 32-4, which shows `hpoint` and `lpoint` defining where the PWM outputs goes high and low again). The forum link above mentioned mcpwm (the motor control driver), but I don't think that peripheral exists in esp32-c3. Additionally, they do mention the hpoint, and seem to suggest it might be available in the LEDC driver, so it might be worth doing some investigating before scrapping the driver and controlling the peripheral directly.

Another option is to use the remote control peripheral (chapter 33 of the datasheet), which has two output channels and more flexibility regarding the output waveform generated.  I copied the `peripherals/rmt/dshot_esc` example to this folder to try out. I modified the GPIO output to 8, and built and ran:

```bash
idf.py set-target esp32c3
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```
That example worked fine. The RMT module is a bit more complicated than PWM, due to the need for an encoder to convert data to the right format for the RMT periperhal. Details are provided in the [RMT API page](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/rmt.html). 
