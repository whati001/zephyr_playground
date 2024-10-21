# Nordic 52840dk <> ESP32 AT Sample
This sample illustrates how to interact with an ESP32 AT module via UART and perform a simple HTTP GET request.
The sample was initially written with Zephyr at commit "372b3a9b8da2967e43676943392daf9091affdb1". 

## Connection
Please find below the connection setup. The pin names are respective to the nrf dk board (nrf TX = esp32 RX, etc...).

| Pin | Nordic | ESP32 |
| --- | ------ | ----- |
| RX  | P1.01  | 17    |
| TX  | P1.02  | 16    |
| CTS | P1.03  | 14    |
| RTS | P1.04  | 15    |

## Configuration
Please set the WiFi AP and server parameters properly in `main.c`.

```h
#define SERVER_ADDR4 "192.168.0.123" // server ip
#define HTTP_PORT 8000 // server port
#define WIFI_AP_SSID "__SSID__" // wifi ap ssid
#define WIFI_AP_PSK "__PSK__" // wifi ap psk (pre shared key)
```

In addition, please ensure that a http server is running on the server.
For testing, you can start a simple http server via python:
```bash
python -m http.server # starts a http server on port 8000
```

## Build & Flash
Building and flashing is done via west:
```bash
# http client
west build -b nrf52840dk/nrf52840 __path__/zephyr_playground/wifi/esp32_wifi_http
west flash
```

## ESP32 Module
We have used a ESP32-WROOM32 module running the firmware version 3.4.0.0.
Please checkout the official [ESP Webpage](https://docs.espressif.com/projects/esp-at/en/latest/esp32/Get_Started/index.html) how to program and connected the ESP with the micro controller.

