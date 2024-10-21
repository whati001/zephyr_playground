/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/wifi_mgmt.h>

/**
 * Please check the KConfig and Devicetree file.
 * 
 * We have enable the network stack and the ESP32_AT module via KConfig
 * The CONFIG_WIFI_ESP_AT expects a devicetree node of compatibility "espressif,esp-at"
 * Please see the "esp-wifi" node under uart1 peripheral:
 *  - RX:  P1.01
 *  - TX:  P1.02
 *  - CTS: P1.03
 *  - RTS: P1.04
 */


// Callback handler for Wi-Fi events
static struct net_mgmt_event_callback wifi_mgmt_cb;

// Function to handle Wi-Fi management events
static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb,
                                    uint32_t mgmt_event, struct net_if *iface) {
  if (mgmt_event == NET_EVENT_WIFI_SCAN_RESULT) {
    struct wifi_scan_result *scan_result = (struct wifi_scan_result *)cb->info;
    printf("Found SSID: %s, Channel: %d, RSSI: %d\n", scan_result->ssid,
           scan_result->channel, scan_result->rssi);
  }

  if (mgmt_event == NET_EVENT_WIFI_SCAN_DONE) {
    printf("Wi-Fi scan completed\n");
  }
}

static int test_wifi_scan(struct net_if *iface) {
  int ret;

  // Register callback for Wi-Fi scan result and completion events
  net_mgmt_init_event_callback(&wifi_mgmt_cb, wifi_mgmt_event_handler,
                               NET_EVENT_WIFI_SCAN_RESULT |
                                   NET_EVENT_WIFI_SCAN_DONE);
  net_mgmt_add_event_callback(&wifi_mgmt_cb);
  printf("Registered event handlers for net work manager\n");

  printf("Starting Wi-Fi scan\n");

  ret = net_mgmt(NET_REQUEST_WIFI_SCAN, iface, NULL, 0);
  if (ret < 0) {
    printf("Wi-Fi scan failed: %d\n", ret);
  }

  return ret;
}

int main(void) {
  int ret;

  // get network interface (we have only registered the esp32-at device)
  // so this function returns it
  struct net_if *iface = net_if_get_default();
  if (!iface) {
    printf("No network interface found\n");
  }

  // perform wifi scan with esp32
  ret = test_wifi_scan(iface);
  printf("Scan startet with ret: %d\n", ret);

  // TODO: use proper wait implementation -> NET_EVENT_WIFI_SCAN_DONE received
  k_msleep(10000);

  return 0;
}
