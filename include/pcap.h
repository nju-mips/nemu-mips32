#ifndef PCAP_H
#define PCAP_H

#include <stdio.h>
#include <stdint.h>

#define PCAP_HEADER_MAGIC 0xa1b2cd34
#define PCAP_HEADER_MAJOR 0x02
#define PCAP_HEADER_MINOR 0x04
#define PCAP_HEADER_LINKK_TYPE_ETH 0x1

typedef struct {
  uint32_t magic; /* 0x34cdb2a1 */
  uint16_t major; /* 0x0200 */
  uint16_t minor; /* 0x0400 */
  uint32_t zone;  /* 00000 */
  uint32_t sig_figs; /* 00000 */
  uint32_t snap_len; /* 65535 */
  uint32_t link_type; /* 1 for ethernet */
} pcap_header_t;

typedef struct {
  uint32_t timestamp_hi; /* seconds */
  uint32_t timestamp_lo; /* microseconds */
  uint32_t caplen;
  uint32_t len;
} pcap_packet_header_t;

typedef FILE *pcap_handler;

pcap_handler pcap_open(const char *filename);
int pcap_write(pcap_handler h, const void *data, const int len);
void pcap_flush(pcap_handler h);
void pcap_close(pcap_handler h);

#endif
