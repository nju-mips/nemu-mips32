#include <arpa/inet.h>
#include <sys/time.h>

#include "utils.h"

void hexdump(const uint8_t *data, int len) {
  for (int i = 0; i < len; i += 16) {
    printf("%02x: ", i);
    for (int j = i; j < len && j < i + 16; j++) printf("%02x ", data[j]);
    printf("\n");
  }
}

pcap_handler pcap_open(const char *filename) {
  FILE *fp = fopen(filename, "wa+");
  pcap_header_t header = {0};
  header.magic = PCAP_HEADER_MAGIC;
  header.major = PCAP_HEADER_MAJOR;
  header.minor = PCAP_HEADER_MINOR;
  header.snap_len = 65535;
  header.link_type = PCAP_HEADER_LINKK_TYPE_ETH;
  fwrite(&header, sizeof(header), 1, fp);
  return fp;
}

int pcap_write_and_flush(pcap_handler h, const void *data, const int len) {
  int ret = pcap_write(h, data, len);
  pcap_flush(h);
  return ret;
}

int pcap_write(pcap_handler h, const void *data, const int len) {
  static const int zeros[2] = {0, 0};

  struct timeval t;
  gettimeofday(&t, NULL);

  pcap_packet_header_t header = {0};
  header.timestamp_hi = t.tv_sec;
  header.timestamp_lo = t.tv_usec;
  header.caplen = len;
  header.len = len;

  fwrite(&header, sizeof(header), 1, h);
  fwrite(zeros, sizeof(zeros), 1, h);
  fwrite(data, len, 1, h);
  return 0;
}

void pcap_flush(pcap_handler h) { fflush(h); }

void pcap_close(pcap_handler h) { fclose(h); }
