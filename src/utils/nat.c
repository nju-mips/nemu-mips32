#if CONFIG_NETWORK
#  include <arpa/inet.h>
#  include <net/if.h>
#  include <netinet/ether.h>
#  include <netpacket/packet.h>
#  include <stdlib.h>
#  include <string.h>
#  include <sys/ioctl.h>
#  include <sys/socket.h>
#  include <sys/types.h>

#  include "debug.h"

char *eth_iface;

static int iface_socket;
static uint32_t iface_ip_addr;
static uint32_t iface_mac_addr[ETHER_ADDR_LEN];
static struct sockaddr_ll eth_sll;

/* https://doc.dpdk.org/api-2.2/rte__ip_8h_source.html */
struct ipv4_hdr {
  uint8_t version_ihl;
  uint8_t type_of_service;
  uint16_t total_length;
  uint16_t packet_id;
  uint16_t fragment_offset;
  uint8_t time_to_live;
  uint8_t next_proto_id;
  uint16_t hdr_checksum;
  uint32_t src_addr;
  uint32_t dst_addr;
} __attribute__((__packed__));

/* https://doc.dpdk.org/api-2.2/rte__tcp_8h_source.html */
struct tcp_hdr {
  uint16_t src_port;
  uint16_t dst_port;
  uint32_t sent_seq;
  uint32_t recv_ack;
  uint8_t data_off;
  uint8_t tcp_flags;
  uint16_t rx_win;
  uint16_t cksum;
  uint16_t tcp_urp;
} __attribute__((__packed__));

/* https://doc.dpdk.org/api-2.2/rte__udp_8h_source.html */
struct udp_hdr {
  uint16_t src_port;
  uint16_t dst_port;
  uint16_t dgram_len;
  uint16_t dgram_cksum;
} __attribute__((__packed__));

// static uint32_t tcp_conns[256];

void init_nat() {
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

void nat_send_data(uint8_t *data, const int len) {
  int protocol = ntohs(*(uint16_t *)&data[12]);
  switch (protocol) {
  case ETH_P_IP:
    /* TCP, UDP, ICMP */
    break;
  case ETH_P_ARP: break;
  }
}
#endif
