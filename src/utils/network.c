#if CONFIG_NETWORK
#  include <arpa/inet.h>
#  include <net/if.h>
#  include <netinet/ether.h>
#  include <netinet/ip.h>
#  include <netinet/tcp.h>
#  include <netinet/udp.h>
#  include <netpacket/packet.h>
#  include <stdlib.h>
#  include <string.h>
#  include <sys/ioctl.h>
#  include <sys/socket.h>
#  include <sys/types.h>

#  include "debug.h"
#  include "utils.h"

char *eth_iface;
static pcap_handler pcap;

static int iface_socket;
static uint32_t iface_ip_addr;
static uint32_t eth_ip_addr;
static struct sockaddr_ll eth_sll;

/* https://doc.dpdk.org/api-2.2/rte__ether_8h_source.html */
struct ether_addr_t {
  uint8_t addr_bytes[ETHER_ADDR_LEN];
} __attribute__((__packed__));

static uint8_t iface_mac_addr[ETHER_ADDR_LEN];
static uint8_t eth_mac_addr[ETHER_ADDR_LEN];

/* /usr/include/netinet/ether.h */
// struct ether_header;
// struct ether_arp;
/* /usr/include/netinet/ip.h */
// struct iphdr
/* /usr/include/netinet/tcp.h */
// struct tcphdr
/* /usr/include/netinet/udphdr.h */
// struct udphdr

struct nat_conn_t {
  uint16_t inner_port;
  uint16_t outer_port;
  uint32_t protocol; // TCP, UDP
  int socket;
  int conn;
};

static struct nat_conn_t nats[16];
static const int nr_nats = sizeof(nats) / sizeof(*nats);

struct nat_conn_t *nat_get_conn_with_inner_port(int proto, int port) {
  for (int i = 0; i < nr_nats; i++) { ; }
  return NULL;
}

struct nat_conn_t *nat_find_conn_with_outer_port(int proto, int port) {
  return NULL;
}

static void ip_packet_fix_checksum(uint8_t *data, const int len) {
  uint32_t checksum = 0;
  *(uint16_t *)&data[24] = 0;
  for (int i = 14; i < 34; i += 2) checksum += *(uint16_t *)&data[i];
  checksum = (checksum >> 16) + (checksum & 0xFFFF);
  // printf("check sum is %04x\n", checksum);
  *(uint16_t *)&data[24] = ~checksum;
}

const char *ip_ntoa(uint32_t ip) {
  static char s[128];
  uint8_t *p = (uint8_t *)&ip;
  sprintf(s, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
  return s;
}

void net_bind_mac_addr(uint8_t mac_addr[ETHER_ADDR_LEN]) {
  memcpy(&eth_mac_addr, mac_addr, ETHER_ADDR_LEN);
}

void init_nat() {
  pcap = pcap_open("build/packets.pcap");

  /* init the socket */
  iface_socket = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  Assert(iface_socket > 0, "init raw socket failed, please run me with sudo");

  struct ifreq eth_req;
  Assert(eth_iface, "please specify net card with --iface");
  strncpy(eth_req.ifr_name, eth_iface, IFNAMSIZ);
  if (ioctl(iface_socket, SIOCGIFINDEX, &eth_req) == -1) {
    perror("ioctl");
    exit(0);
  }

  eth_sll.sll_ifindex = eth_req.ifr_ifindex;
  eth_sll.sll_family = AF_PACKET;
  eth_sll.sll_halen = ETHER_ADDR_LEN;

  /* get mac address */
  if (ioctl(iface_socket, SIOCGIFHWADDR, &eth_req) == -1) {
    perror("ioctl");
    abort();
  } else {
    memcpy(iface_mac_addr, eth_req.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);
  }

  /* get ip address */
  if (ioctl(iface_socket, SIOCGIFADDR, &eth_req) == -1) {
    perror("ioctl");
    abort();
  } else {
    struct sockaddr_in *iface_ip = (void *)&eth_req.ifr_addr;
    iface_ip_addr = iface_ip->sin_addr.s_addr;
  }
}

static void recver_modify_packet(uint8_t *data, const int len) {
  /* copy the eth addr */
  memcpy(data, &eth_mac_addr, ETHER_ADDR_LEN);
  /* 0x1a .. 0x1d is ip.src.ip, 0x1e .. 0x21 is ip.dst.ip */
  /* 0x16 .. 0x1b is arp.src.mac, 0x1c .. 0x1f is arp.src.ip */
  /* 0x20 .. 0x25 is arp.dst.mac, 0x26 .. 0x29 is arp.dst.ip */

  int protocol = ntohs(*(uint16_t *)&data[12]);
  switch (protocol) {
  case ETH_P_IP: {
    memcpy(&data[0x1e], &eth_ip_addr, 4);
    ip_packet_fix_checksum(data, len);
  } break;
  case ETH_P_ARP:
    memcpy(&data[0x20], &eth_mac_addr, ETHER_ADDR_LEN);
    memcpy(&data[0x26], &eth_ip_addr, 4);
    break;
  }
}

void net_ip_send_data(uint8_t *data, const int len) {

  struct iphdr *iphdr = (void *)data + sizeof(struct ether_header);
  memcpy(&data[0x1a], &iface_ip_addr, 4);
  switch (iphdr->protocol) {
  case IPPROTO_TCP: {
  } break;
  case IPPROTO_UDP: {
    struct udphdr *udphdr = (void *)iphdr + sizeof(*iphdr);
    for (int i = 0; i < nr_nats; i++) {
      if (udphdr->uh_sport == nats[i].inner_port) {
        udphdr->uh_sport = nats[i].outer_port;
        goto end;
      }
    }
  } break;
  }

end:
  ip_packet_fix_checksum(data, len);
}

void net_send_data(const uint8_t *_data, const int len) {
  static uint8_t data[2048];
  assert(len < sizeof(data));
  memcpy(data, _data, len);

  pcap_write_and_flush(pcap, data, len);

  struct ether_header *ehdr = (void *)data;
  memcpy(ehdr->ether_shost, iface_mac_addr, ETHER_ADDR_LEN);

  int protocol = ntohs(ehdr->ether_type);
  switch (protocol) {
  case ETH_P_IP: {
    net_ip_send_data(data, len);
  } break;
  case ETH_P_ARP: {
    struct ether_arp *ahdr = (void *)data + sizeof(struct ether_header);
    memcpy(&eth_sll.sll_addr, ehdr->ether_dhost, ETHER_ADDR_LEN);
    memcpy(ahdr->arp_sha, &iface_mac_addr, ETHER_ADDR_LEN);
    memcpy(&eth_ip_addr, ahdr->arp_sha, sizeof(eth_ip_addr));
    memcpy(ahdr->arp_spa, &iface_ip_addr, sizeof(iface_ip_addr));
    eth_sll.sll_protocol = protocol;
    pcap_write_and_flush(pcap, data, len);
    /* send the data */
    sendto(iface_socket, &data[0], len, 0, (struct sockaddr *)&eth_sll,
        sizeof(eth_sll));
  } break;
  default: printf("unsupported protocol %d\n", protocol); break;
  }
}

int net_recv_data(uint8_t *to, const int maxlen) {
  // printf("try receive data\n");
  int nbytes = recvfrom(iface_socket, to, maxlen, 0, NULL, NULL);

  pcap_write_and_flush(pcap, to, nbytes);
  recver_modify_packet(to, nbytes);
  pcap_write_and_flush(pcap, to, nbytes);
  return nbytes;
}

#endif
