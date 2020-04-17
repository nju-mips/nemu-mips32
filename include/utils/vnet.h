#ifndef UTILS_VNET_H
#define UTILS_VNET_H

#include <net/if.h>
#include <netinet/ether.h>
#include <stdbool.h>
#include <stdint.h>

/* tap */
int tap_create(char dev[IFNAMSIZ]);
int tap_set_hwaddr(int sockfd, const char *dev,
    const uint8_t ether_addr[ETHER_ADDR_LEN]);
int tap_set_netmask(
    int sockfd, const char *dev, const uint32_t netmask);
int tap_set_ipaddr(
    int sockfd, const char *dev, const uint32_t ipaddr);
int tap_set_mtu(int sockfd, const char *dev, int mtu);
int tap_set_up(int sockfd, const char *dev);
int tap_set_down(int sockfd, const char *dev);
int tap_set_attribute(const char *dev,
    const uint32_t ipaddr,
    const uint8_t ether_addr[ETHER_ADDR_LEN], int mtu);

/* pcap */
#define PCAP_HEADER_MAGIC 0xa1b2cd34
#define PCAP_HEADER_MAJOR 0x02
#define PCAP_HEADER_MINOR 0x04
#define PCAP_HEADER_LINKK_TYPE_ETH 0x1

typedef struct {
  uint32_t magic;     /* 0x34cdb2a1 */
  uint16_t major;     /* 0x0200 */
  uint16_t minor;     /* 0x0400 */
  uint32_t zone;      /* 00000 */
  uint32_t sig_figs;  /* 00000 */
  uint32_t snap_len;  /* 65535 */
  uint32_t link_type; /* 1 for ethernet */
} pcap_header_t;

typedef struct {
  uint32_t timestamp_hi; /* seconds */
  uint32_t timestamp_lo; /* microseconds */
  uint32_t caplen;
  uint32_t len;
} pcap_packet_header_t;

typedef FILE *pcap_handler;

void hexdump(const uint8_t *data, int len);

pcap_handler pcap_open(const char *filename);
int pcap_write(
    pcap_handler h, const void *data, const int len);
void pcap_flush(pcap_handler h);
int pcap_write_and_flush(
    pcap_handler h, const void *data, const int len);
void pcap_close(pcap_handler h);

/* nat */
void init_network();
bool net_poll_packet();
void net_bind_mac_addr(
    const uint8_t mac_addr[ETHER_ADDR_LEN]);
void net_send_data(const uint8_t *data, const int len);
int net_recv_data(uint8_t *to, const int maxlen);

#endif
