# ESP32 development 

This folder contains ESP32 development information. ESP32 devices are 32-bit microcontrollers containing WiFi and Bluetooth modules, and targetting IOT-like applications.

## Installing the ESP32-C3 toolchain

The toolchain for programming ESP32 devices is free and open-source. Start [here](https://idf.espressif.com/), and pick the ESP32 device of interest. That should link to a getting started page describing how to install the toolchain for the device of interest.

It is possible to install the toolchain through VSCode. This page describes how to install the toolchain manually on Linux (without VSCode) for ESP32-C3 devices. The relevant page is linked in the `manual installation` section of the page (linking to [this page](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/get-started/linux-macos-setup.html) for ESP32-C3 devices.

First, install some prerequisites:

```bash
sudo apt-get install git wget cmake flex bison gperf python3 python3-venv ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
```

Next, download the ESP-IDF repository (IDF stands for IOT Development Framework). You can put this anywhere (the final installed location will be `~/.espressif`)

```bash
mkdir -p ~/opt && cd ~/opt
git clone --recursive https://github.com/espressif/esp-idf.git
```

Now install the toolchain for the ESP32-C3 device as follows:

```bash
cd ~/opt/esp-idf
./install.sh esp32c3
```

It is also necessary to install the IDF compiler tools using the `idf_tools.py` script:

```bash
cd ~/opt/esp-idf
python3 tools/idf_tools.py install
```

If you forget to do this, there is no issue -- you will get an error like `ERROR: tool esp32ulp-elf has no installed versions. [...]` when you attempt to source the tools in a later step. At that point, you can run the script above to fix the issue.

Each time you want to use the toolchain, you will need to source some environment variables. This should not be added to the global `~/.bashrc`, in order to avoiding importing the tools into the environment every time a terminal opens. Instead, run the `export.sh` script in this folder:

```bash
. export.sh
```

After sourcing .bashrc (or reopening the terminal), run `esp_idf` to load all the required tools into the environment. If you get the line `Done! You can now compile ESP-IDF projects.` then everything worked.

## Updating your cmake version

When the toolchain is sourced, you may get a warning about a supported but outdated version of cmake (e.g. `Using a supported version of tool cmake found in PATH: 3.16.3. However the recommended version is 3.24.0.`). The easiest way to solve this is by removing the package manager version of cmake and installing the latest version manually from the cmake website:

```bash
sudo apt remove cmake
cd ~/src/
# Replace with the latest version in the lines below
wget https://github.com/Kitware/CMake/releases/download/v3.25.1/cmake-3.25.1-linux-x86_64.sh
chmod u+x cmake-3.25.1-linux-x86_64.sh
./cmake-3.25.1-linux-x86_64.sh --prefix=$HOME/opt --exclude-subdir
```

You may also need to add $HOME/opt/bin to your path in .bashrc if you have not done already.

Now when you run `esp_idf`, you should not get any warnings relating to the CMake version.

## Building an example project

The official document is very good -- look there for a more detailed/comprehensive description of how everything works. The following is just a summary of how the build process works.

There are many example projects in the directory `~/opt/esp-idf/examples`. Navigate to any of them (for example `cd get-started/blink`). Each directory contains a README.md describing the example. The build process is based on CMake, so you should see a CMakeLists.txt in the directory. To build, run

```bash
idf.py set-target esp32c3 # Set the target device
idf.py menuconfig # Optionally configure project/build properties
idf.py build
```

This creates a build/ directory containing the compiled files (e.g. dpp-enrollee.elf), which can be flashed to the device using a command like

```bash
idf.py -p (PORT) flash
```

The port will probably be `/dev/ttyUSB0` but you can check by running `ls /dev/tty*` while the device is plugged in then taken out and seeing which one appears/disappears. If you run the flash line and you get an error saying the port doesn't exist, add yourself to the `dialout` group using `sudo usermod -a -G dialout (USER)` and then restart the computer.

The compiler toolchain for the ESP32-C3 devices is `risc32-esp-elf-*` (tab complete to see the tools). To disassemble the resulting binary, run

```bash
cd build/
riscv32-esp-elf-objdump -S --disassemble=app_main blink.elf
```

The result should be the contents of the `app_main` function, which should contains RISC-V instructions (if the ESP32-C3 target was set correctly).

The `-S` flag includes source code in the output, and the `--disassemble=<sym>` disassembles the contents of a symbol. To see the symbol table, use the `-t` flag (add pipe the results to grep to restrict the symbols to those of interest -- e.g. function names).

## Creating a new project

Create a new project as follows:

```bash
idf.py create-project NAME
```

This command will make a folder called `NAME` and populate it with the necessary files. Run `idf.py set-target esp32c3` to set the target device. Then build and flash using:

```bash
idf.py build
idf.py -p /dev/ttyUSB0 flash
idf.py monitor # To open logging console. Use ctrl-] to exit
```
