#include "pcap.h"
#include <arpa/inet.h>

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

int pcap_write(pcap_handler h, const void *data, const int len) {
  static const int zeros[2] = {0, 0};

  pcap_packet_header_t header = {0};
  header.timestamp_hi = 0;
  header.timestamp_lo = 0;
  header.caplen = len;
  header.len = len;

  fwrite(&header, sizeof(header), 1, h);
  fwrite(zeros, sizeof(zeros), 1, h);
  fwrite(data, len, 1, h);
  return 0;
}

void pcap_flush(pcap_handler h) { fflush(h); }

void pcap_close(pcap_handler h) { fclose(h); }
