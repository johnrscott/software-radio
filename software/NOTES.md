# General Notes

This document is the equivalent of the lab book for the software development.

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

Started a new blank project `idf.py create-project sdr` to prototype with the RMT module. The RMT module TX works by specifying pulses as sequences of levels with durations; for example, output 1 for 3 ticks, output 0 for 2 ticks; output 1 for 10 ticks; etc. The format is 15 bits for the duration in ticks, and 1 bit for the level (the most significant bit).

The get a ballpark calculation for the tick period, assume that all centre frequencies from 535 kHz to 1605 kHz should be synthesised, with max error 0.5 kHz, and for each frequency, the I and Q LO signals can be generated 90 deg apart with at most 1 deg error. The required periods (in seconds) are 1/1000n, for n = 535, ... 1605. 

Written a program to compute the error in the frequency synthesised from a particular tick frequency for the two LO signals, and also the phase error
between them. The results show that even the maximum tick frequency of 80 MHz (see technical ref manual 15.2.3) has a maximum phase error of 4% (14.4 degrees), and a maximum frequency error of 0.9%. Can keep only the rows where the number of ticks is a multiple of four (meaning zero phase error), which leaves the following frequencies (this is using a tick period of 80 MHz). `n` is the number of ticks, `f` is the target frequency, and `f_err` is the relative error of the synthesised frequency compared to `f`. This is enough zero-phase-error frequencies to be getting on with for a prototype. Note how all the `n` are multiples of 4.

              f    n         f_err
4      539000.0  148  2.858146e-03
5      540000.0  148  1.001001e-03
6      541000.0  148  8.492781e-04
7      542000.0  148  2.692730e-03
19     554000.0  144  2.807862e-03
20     555000.0  144  1.001001e-03
21     556000.0  144  7.993605e-04
22     557000.0  144  2.593258e-03
35     570000.0  140  2.506266e-03
36     571000.0  140  7.505629e-04
37     572000.0  140  9.990010e-04
38     573000.0  140  2.742458e-03
52     587000.0  136  2.104419e-03
53     588000.0  136  4.001601e-04
54     589000.0  136  1.298312e-03
55     590000.0  136  2.991027e-03
69     604000.0  132  3.411599e-03
70     605000.0  132  1.753068e-03
71     606000.0  132  1.000100e-04
72     607000.0  132  1.547601e-03
73     608000.0  132  3.189793e-03
88     623000.0  128  3.210273e-03
89     624000.0  128  1.602564e-03
90     625000.0  128  0.000000e+00
91     626000.0  128  1.597444e-03
92     627000.0  128  3.189793e-03
108    643000.0  124  3.361260e-03
109    644000.0  124  1.803246e-03
110    645000.0  124  2.500625e-04
111    646000.0  124  1.298312e-03
112    647000.0  124  2.841901e-03
129    664000.0  120  4.016064e-03
130    665000.0  120  2.506266e-03
131    666000.0  120  1.001001e-03
132    667000.0  120  4.997501e-04
133    668000.0  120  1.996008e-03
134    669000.0  120  3.487793e-03
152    687000.0  116  3.864880e-03
153    688000.0  116  2.405774e-03
154    689000.0  116  9.509034e-04
155    690000.0  116  4.997501e-04
156    691000.0  116  1.946205e-03
157    692000.0  116  3.388479e-03
177    712000.0  112  3.210273e-03
178    713000.0  112  1.803246e-03
179    714000.0  112  4.001601e-04
180    715000.0  112  9.990010e-04
181    716000.0  112  2.394254e-03
182    717000.0  112  3.785615e-03
203    738000.0  108  3.713741e-03
204    739000.0  108  2.355536e-03
205    740000.0  108  1.001001e-03
206    741000.0  108  3.498775e-04
207    742000.0  108  1.697115e-03
208    743000.0  108  3.040726e-03
209    744000.0  108  4.380725e-03
231    766000.0  104  4.217714e-03
232    767000.0  104  2.908434e-03
233    768000.0  104  1.602564e-03
234    769000.0  104  3.000900e-04
235    770000.0  104  9.990010e-04
236    771000.0  104  2.294722e-03
237    772000.0  104  3.587086e-03
262    797000.0  100  3.764115e-03
263    798000.0  100  2.506266e-03
264    799000.0  100  1.251564e-03
265    800000.0  100  1.455192e-16
266    801000.0  100  1.248439e-03
267    802000.0  100  2.493766e-03
268    803000.0  100  3.735990e-03
269    804000.0  100  4.975124e-03
295    830000.0   96  4.016064e-03
296    831000.0   96  2.807862e-03
297    832000.0   96  1.602564e-03
298    833000.0   96  4.001601e-04
299    834000.0   96  7.993605e-04
300    835000.0   96  1.996008e-03
301    836000.0   96  3.189793e-03
302    837000.0   96  4.380725e-03
330    865000.0   92  5.277708e-03
331    866000.0   92  4.116879e-03
332    867000.0   92  2.958728e-03
333    868000.0   92  1.803246e-03
334    869000.0   92  6.504228e-04
335    870000.0   92  4.997501e-04
336    871000.0   92  1.647282e-03
337    872000.0   92  2.792182e-03
338    873000.0   92  3.934459e-03
339    874000.0   92  5.074122e-03
369    904000.0   88  5.631537e-03
370    905000.0   88  4.520342e-03
371    906000.0   88  3.411599e-03
372    907000.0   88  2.305302e-03
373    908000.0   88  1.201442e-03
374    909000.0   88  1.000100e-04
375    910000.0   88  9.990010e-04
376    911000.0   88  2.095599e-03
377    912000.0   88  3.189793e-03
378    913000.0   88  4.281589e-03
379    914000.0   88  5.370997e-03
412    947000.0   84  5.682104e-03
413    948000.0   84  4.621258e-03
414    949000.0   84  3.562647e-03
415    950000.0   84  2.506266e-03
416    951000.0   84  1.452106e-03
417    952000.0   84  4.001601e-04
418    953000.0   84  6.495778e-04
419    954000.0   84  1.697115e-03
420    955000.0   84  2.742458e-03
421    956000.0   84  3.785615e-03
422    957000.0   84  4.826591e-03
423    958000.0   84  5.865394e-03
459    994000.0   80  6.036217e-03
460    995000.0   80  5.025126e-03
461    996000.0   80  4.016064e-03
462    997000.0   80  3.009027e-03
463    998000.0   80  2.004008e-03
464    999000.0   80  1.001001e-03
465   1000000.0   80  0.000000e+00
466   1001000.0   80  9.990010e-04
467   1002000.0   80  1.996008e-03
468   1003000.0   80  2.991027e-03
469   1004000.0   80  3.984064e-03
470   1005000.0   80  4.975124e-03
471   1006000.0   80  5.964215e-03
511   1046000.0   76  6.339942e-03
512   1047000.0   76  5.378776e-03
513   1048000.0   76  4.419446e-03
514   1049000.0   76  3.461944e-03
515   1050000.0   76  2.506266e-03
516   1051000.0   76  1.552406e-03
517   1052000.0   76  6.003602e-04
518   1053000.0   76  3.498775e-04
519   1054000.0   76  1.298312e-03
520   1055000.0   76  2.244949e-03
521   1056000.0   76  3.189793e-03
522   1057000.0   76  4.132849e-03
523   1058000.0   76  5.074122e-03
524   1059000.0   76  6.013618e-03
569   1104000.0   72  6.441224e-03
570   1105000.0   72  5.530417e-03
571   1106000.0   72  4.621258e-03
572   1107000.0   72  3.713741e-03
573   1108000.0   72  2.807862e-03
574   1109000.0   72  1.903617e-03
575   1110000.0   72  1.001001e-03
576   1111000.0   72  1.000100e-04
577   1112000.0   72  7.993605e-04
578   1113000.0   72  1.697115e-03
579   1114000.0   72  2.593258e-03
580   1115000.0   72  3.487793e-03
581   1116000.0   72  4.380725e-03
582   1117000.0   72  5.272058e-03
583   1118000.0   72  6.161797e-03
633   1168000.0   68  7.252216e-03
634   1169000.0   68  6.390580e-03
635   1170000.0   68  5.530417e-03
636   1171000.0   68  4.671724e-03
637   1172000.0   68  3.814495e-03
638   1173000.0   68  2.958728e-03
639   1174000.0   68  2.104419e-03
640   1175000.0   68  1.251564e-03
641   1176000.0   68  4.001601e-04
642   1177000.0   68  4.497976e-04
643   1178000.0   68  1.298312e-03
644   1179000.0   68  2.145387e-03
645   1180000.0   68  2.991027e-03
646   1181000.0   68  3.835234e-03
647   1182000.0   68  4.678013e-03
648   1183000.0   68  5.519368e-03
649   1184000.0   68  6.359300e-03
650   1185000.0   68  7.197816e-03
706   1241000.0   64  7.252216e-03
707   1242000.0   64  6.441224e-03
708   1243000.0   64  5.631537e-03
709   1244000.0   64  4.823151e-03
710   1245000.0   64  4.016064e-03
711   1246000.0   64  3.210273e-03
712   1247000.0   64  2.405774e-03
713   1248000.0   64  1.602564e-03
714   1249000.0   64  8.006405e-04
715   1250000.0   64  0.000000e+00
716   1251000.0   64  7.993605e-04
717   1252000.0   64  1.597444e-03
718   1253000.0   64  2.394254e-03
719   1254000.0   64  3.189793e-03
720   1255000.0   64  3.984064e-03
721   1256000.0   64  4.777070e-03
722   1257000.0   64  5.568815e-03
723   1258000.0   64  6.359300e-03
724   1259000.0   64  7.148531e-03
788   1323000.0   60  7.810532e-03
789   1324000.0   60  7.049345e-03
790   1325000.0   60  6.289308e-03
791   1326000.0   60  5.530417e-03
792   1327000.0   60  4.772670e-03
793   1328000.0   60  4.016064e-03
794   1329000.0   60  3.260597e-03
795   1330000.0   60  2.506266e-03
796   1331000.0   60  1.753068e-03
797   1332000.0   60  1.001001e-03
798   1333000.0   60  2.500625e-04
799   1334000.0   60  4.997501e-04
800   1335000.0   60  1.248439e-03
801   1336000.0   60  1.996008e-03
802   1337000.0   60  2.742458e-03
803   1338000.0   60  3.487793e-03
804   1339000.0   60  4.232014e-03
805   1340000.0   60  4.975124e-03
806   1341000.0   60  5.717127e-03
807   1342000.0   60  6.458023e-03
808   1343000.0   60  7.197816e-03
809   1344000.0   60  7.936508e-03
882   1417000.0   56  8.166146e-03
883   1418000.0   56  7.455168e-03
884   1419000.0   56  6.745193e-03
885   1420000.0   56  6.036217e-03
886   1421000.0   56  5.328240e-03
887   1422000.0   56  4.621258e-03
888   1423000.0   56  3.915270e-03
889   1424000.0   56  3.210273e-03
890   1425000.0   56  2.506266e-03
891   1426000.0   56  1.803246e-03
892   1427000.0   56  1.101211e-03
893   1428000.0   56  4.001601e-04
894   1429000.0   56  2.999100e-04
895   1430000.0   56  9.990010e-04
896   1431000.0   56  1.697115e-03
897   1432000.0   56  2.394254e-03
898   1433000.0   56  3.090420e-03
899   1434000.0   56  3.785615e-03
900   1435000.0   56  4.479841e-03
901   1436000.0   56  5.173100e-03
902   1437000.0   56  5.865394e-03
903   1438000.0   56  6.556726e-03
904   1439000.0   56  7.247096e-03
905   1440000.0   56  7.936508e-03
906   1441000.0   56  8.624963e-03
989   1524000.0   52  9.489198e-03
990   1525000.0   52  8.827238e-03
991   1526000.0   52  8.166146e-03
992   1527000.0   52  7.505919e-03
993   1528000.0   52  6.846557e-03
994   1529000.0   52  6.188057e-03
995   1530000.0   52  5.530417e-03
996   1531000.0   52  4.873637e-03
997   1532000.0   52  4.217714e-03
998   1533000.0   52  3.562647e-03
999   1534000.0   52  2.908434e-03
1000  1535000.0   52  2.255074e-03
1001  1536000.0   52  1.602564e-03
1002  1537000.0   52  9.509034e-04
1003  1538000.0   52  3.000900e-04
1004  1539000.0   52  3.498775e-04
1005  1540000.0   52  9.990010e-04
1006  1541000.0   52  1.647282e-03
1007  1542000.0   52  2.294722e-03
1008  1543000.0   52  2.941323e-03
1009  1544000.0   52  3.587086e-03
1010  1545000.0   52  4.232014e-03
1011  1546000.0   52  4.876107e-03
1012  1547000.0   52  5.519368e-03
1013  1548000.0   52  6.161797e-03
1014  1549000.0   52  6.803397e-03
1015  1550000.0   52  7.444169e-03
1016  1551000.0   52  8.084114e-03
1017  1552000.0   52  8.723236e-03
1018  1553000.0   52  9.361534e-03

## 02/08/2023

Got a simple copy encoder working (see `sdr.c`); this allows you to just define the 16 bit period-and-bit format in a C array and then send it to the RMT module using `rmt_transmit`. The two relevant snippets are:

```c
// This is the raw 16-bit data
uint32_t raw_rmt_data[3] = {
	0xffff7fff, // One bit high, one bit low
	0 // End of transmission marker
};

// Send the data (tx_config uses -1 for infinite loop)
ESP_ERROR_CHECK(rmt_transmit(channel_handle, copy_encoder,
				 &raw_rmt_data, sizeof(raw_rmt_data),
				 &tx_config));
```

Verified the results on an oscilloscope. Also verified that increasing the clock frequency to 80000000 also increases the frequency of the square wave, although a quick calculation appears to show the tick frequency is 160 MHz -- probably a factor of two error somewhere (or in the measurement).
