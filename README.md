# SpinoMod Documentation

## Introduction

SpinoMod is a software tool designed to send commands to the SPINO communication board, developed by AMSAT-Francophone and Electrolab. The SPINO board is an open-source communication board for CubeSats, intended to facilitate satellite communication projects. You can find more information about the SPINO board and related projects [here](https://code.electrolab.fr/spino/).

SpinoMod interfaces with SPINO Controller, the control software for the SPINO board developed by AMSAT-F and Electrolab. The primary objective of SpinoMod is to serve as a bridge between user commands and the SPINO board, ensuring seamless communication and control.

## Features

- Send commands to the SPINO communication board.
- Interface with the SPINO Controller software.
- Configure SDR (Software Defined Radio) parameters for communication.
- Manage TCP/IP communication for data transmission.
- Handle FSK modulation initialization and configuration.

## Installation

### Prerequisites

To build SpinoMod, ensure you have the following dependencies installed:

- GCC or another C compiler
- CMake
- SoapySDR library
- Standard C libraries

### SoapySDR Installation

SoapySDR is a vendor-neutral library that provides a common interface to various SDR (Software Defined Radio) hardware. Follow these steps to install SoapySDR:

1. Clone the SoapySDR repository:

    ```sh
    git clone https://github.com/pothosware/SoapySDR.git
    cd SoapySDR
    ```

2. Build and install SoapySDR:

    ```sh
    git clone https://github.com/pothosware/SoapySDR.git
    cd SoapySDR

    mkdir build
    cd build
    cmake ..
    make -j`nproc`
    sudo make install -j`nproc`
    sudo ldconfig #needed on debian systems
    SoapySDRUtil --info
    ```

3. Install the driver for your specific SDR hardware (e.g., HackRF, USRP, etc.) :

    ```sh
    #osmo sdr support:
    sudo apt-get install osmo-sdr soapysdr-module-osmosdr

    #rtl sdr support:
    sudo apt-get install rtl-sdr soapysdr-module-rtlsdr

    #blade rf support:
    sudo apt-get install bladerf soapysdr-module-bladerf

    #hack rf support:
    sudo apt-get install hackrf soapysdr-module-hackrf

    #usrp support:
    sudo apt-get install uhd-host uhd-soapysdr soapysdr-module-uhd

    #miri SDR support:
    sudo apt-get install miri-sdr soapysdr-module-mirisdr

    #rf space support:
    sudo apt-get install soapysdr-module-rfspace

    #airspy support:
    sudo apt-get install airspy soapysdr-module-airspy
    ```

### SpinoMod Installation

Clone the SpinoMod repository and build the project:

```sh
git clone <https://github.com/MiguelP009/SpinoMod.git>
cd SpinoMod
make
```

## Usage

The SpinoMod executable accepts parameters for configuration during runtime. These parameters include the gain, TCP port, and frequency. If parameters are not provided, default values will be used.

### Command-Line Options

- `--gain <gain(dB)>`: Set the gain for the SDR device (default: 35.0).
- `--tcp_port <port>`: Set the TCP port for communication (default: 8888).
- `--spino_freq <frequency(Hz)>`: Set the frequency for the SPINO communication (default: 145.83e6).

### Example Command

```sh
./SpinoMod --gain 30 --tcp_port 9999 --spino_freq 144.80e6
```

### Default Usage

If no parameters are provided, the program uses the default values.

```sh
./SpinoMod
```

## Software Architecture

### Overview

SpinoMod is composed of several key components:

- **Modem Initialization**: Sets up the VCO, configures the SDR device, and initializes TCP sockets.
- **Data Reception**: Handles receiving data over TCP and processes it for transmission via the SDR.
- **FSK Modulation**: Initializes and configures FSK modulation parameters.


## Contributing

If you would like to contribute to SpinoMod, please follow these steps:

1. Fork the repository.
2. Create a new branch.
3. Make your changes.
4. Submit a pull request.


## Acknowledgements

For more information about the SPINO project, visit the [Electrolab SPINO page](https://code.electrolab.fr/spino/).