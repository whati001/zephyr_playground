/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/net/http/client.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/wifi_mgmt.h>

// Define WiFi AP and Server
#define SERVER_ADDR4 "192.168.0.123"
#define HTTP_PORT 8000
#define WIFI_AP_SSID "__SSID__"
#define WIFI_AP_PSK "__PSK__"

#define MAX_RECV_BUF_LEN 512

static uint8_t recv_buf_ipv4[MAX_RECV_BUF_LEN];

// Callback handler for Wi-Fi events
static struct net_mgmt_event_callback wifi_mgmt_cb;
static K_SEM_DEFINE(wifi_connected, 0, 1);

/**
 * @brief Callback for network manager
 */
static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb,
                                    uint32_t mgmt_event, struct net_if *iface) {

  // handle wifi connect cmd response
  if (mgmt_event == NET_EVENT_WIFI_CONNECT_RESULT) {

    const struct wifi_status *status = (const struct wifi_status *)cb->info;

    if (status->status) {
      printf("Connection request failed (%d)\n", status->status);
    } else {
      printf("Connected\n");
    }

    k_sem_give(&wifi_connected);
  }
}

/**
 * @brief Response patch callback of HTTP client
 */
static void response_cb(struct http_response *rsp,
                        enum http_final_call final_data, void *user_data) {
  if (final_data == HTTP_DATA_MORE) {
    printf("Partial data received (%zd bytes)\n", rsp->data_len);
  } else if (final_data == HTTP_DATA_FINAL) {
    printf("All the data received (%zd bytes)\n", rsp->data_len);
  }

  printf("Response to %s\n", (const char *)user_data);
  printf("Response status %s\n", rsp->http_status);
}

/**
 * @brief Establish remote server socket connection (IPv4)
 */
static int connect_socket(sa_family_t family, const char *server, int port,
                          int *sock, struct sockaddr *addr,
                          socklen_t addr_len) {
  int ret;
  memset(addr, 0, addr_len);

  net_sin(addr)->sin_family = AF_INET;
  net_sin(addr)->sin_port = htons(port);
  inet_pton(family, server, &net_sin(addr)->sin_addr);

  *sock = socket(family, SOCK_STREAM, IPPROTO_TCP);

  if (*sock < 0) {
    printf("Failed to create IPv4 HTTP socket (%d)\n", -errno);
    return -1;
  }

  ret = connect(*sock, addr, addr_len);
  if (ret < 0) {
    printf("Cannot connect to IPv4 remote (%d)\n", -errno);
    close(*sock);
    *sock = -1;
    ret = -errno;
  }

  return ret;
}

/**
 * @brief Establish a WPA2 WiFi connection
 */
static int wifi_connect_to_ap(void) {
  printf("Start trying to establish a WiFi connection\n");

  // taken from wifi shell
  // (ZEPHYR_BASE/subsys/net/l2/wifi/wifi_shell.c@cmd_wifi_connect)
  struct net_if *iface = net_if_get_wifi_sta();
  struct wifi_connect_req_params cnx_params = {0};
  int ret;

  net_mgmt_init_event_callback(&wifi_mgmt_cb, wifi_mgmt_event_handler,
                               NET_EVENT_WIFI_CONNECT_RESULT);

  net_mgmt_add_event_callback(&wifi_mgmt_cb);

  cnx_params.ssid = WIFI_AP_SSID;
  cnx_params.ssid_length = strlen(WIFI_AP_SSID);
  cnx_params.psk = WIFI_AP_PSK;
  cnx_params.psk_length = strlen(WIFI_AP_PSK);
  cnx_params.security = WIFI_SECURITY_TYPE_PSK;

  ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &cnx_params,
                 sizeof(struct wifi_connect_req_params));
  if (ret) {
    printk("Connection request failed with error: %d\n", ret);
    return -ENOEXEC;
  }

  printf("Requested to establish a WiFi connection\n");

  printf("Wait until connect completed\n");
  k_sem_take(&wifi_connected, K_FOREVER);
  // INFO: we do not check the actual result of the connect

  return 1;
}

int main(void) {
  struct sockaddr_in addr4;
  int sock4 = -1;
  int32_t timeout = 3 * MSEC_PER_SEC;
  int ret = 0;
  int port = HTTP_PORT;

  // establish a AP connection
  ret = wifi_connect_to_ap();

  // open a socket
  printf("Start to open new IPv4 socket\n");
  (void)connect_socket(AF_INET, SERVER_ADDR4, port, &sock4,
                       (struct sockaddr *)&addr4, sizeof(addr4));

  if (sock4 < 0) {
    printf("Cannot create HTTP connection.\n");
    return -ECONNABORTED;
  }

  printf("Received socket fd: %d\n", sock4);

  // send some test http request
  struct http_request req;

  memset(&req, 0, sizeof(req));

  req.method = HTTP_GET;
  req.url = "/";
  req.host = SERVER_ADDR4;
  req.protocol = "HTTP/1.1";
  req.response = response_cb;
  req.recv_buf = recv_buf_ipv4;
  req.recv_buf_len = sizeof(recv_buf_ipv4);

  ret = http_client_req(sock4, &req, timeout, "IPv4 GET");

  close(sock4);

  exit(ret);
  return ret;
}
