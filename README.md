# STM32F446RE USART Device Drivers

This repository contains USART (Universal Synchronous Asynchronous Receiver Transmitter) device drivers for STM32F446RE microcontrollers with ARM Cortex-M4 processors.

## Features

- USART configuration for UART2, UART4, and UART5.
- USART communication functions for sending and receiving data.
- Sample code for UART configuration on STM32 microcontrollers.

## Usage

1. Include the appropriate USART driver files in your STM32 project.
2. Configure USART according to your specific needs using the provided functions.
3. Utilize the `_USART_WRITE` function to send data over USART.
4. Use `_USART_READ` and `_USART_READ_STR` functions to receive data.
5. Modify the baud rate, word length, and other settings as required.

## License

This project is provided under the license terms specified in the source code files. Please see the individual source files for licensing details.
