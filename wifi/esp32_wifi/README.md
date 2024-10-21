# Nordic 52840dk <> ESP32 AT Sample
This sample illustrates how to interact with an ESP32 AT module via UART.
It performs:
- WiFi Scan
- HTTP Request


## Connection
Please find below the connection setup. The pin names are respective to the nrf dk board (nrf TX = esp32 RX, etc...).

| Pin | Nordic | ESP32 |
| --- | ------ | ----- |
| RX  | P1.01  | 17    |
| TX  | P1.02  | 16    |
| CTS | P1.03  | 14    |
| RTS | P1.04  | 15    |

## ESP32 Module
We have used a ESP32-WROOM32 module running the firmware version 3.4.0.0.
Please checkout the official [ESP Webpage](https://docs.espressif.com/projects/esp-at/en/latest/esp32/Get_Started/index.html) how to program and connected the ESP with the micro controller.