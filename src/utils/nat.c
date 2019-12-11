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
#  include "utils.h"

static char *eth_iface;
static pcap_handler pcap;

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

static void sender_modify_packet(u8 *data, const int len) {
  /* copy the destination addr */
  memcpy(&eth_sll.sll_addr, &data[0], ENET_ADDR_LENGTH);
  /* override the source addr */
  memcpy(&data[ENET_ADDR_LENGTH], iface_mac_addr, ENET_ADDR_LENGTH);

  int protocol = ntohs(*(u16 *)&data[12]);
  switch (protocol) {
  case ETH_P_IP: {
    break;
    memcpy(&data[0x1a], &iface_ip_addr, 4);
    ip_packet_modify_checksum(data, len);
  } break;
  case ETH_P_ARP:
    memcpy(&data[0x16], &iface_mac_addr, ENET_ADDR_LENGTH);
    memcpy(&eth_ip_addr, &data[0x1c], 4);
    printf("set eth_ip_addr to %s\n", ip_ntoa(eth_ip_addr));
    memcpy(&data[0x1c], &iface_ip_addr, 4);
    break;
  }
  // eth_packet_modify_crc(data, len);
}

static void recver_modify_packet(u8 *data, const int len) {
  /* copy the eth addr */
  memcpy(data, eth_mac_addr, ENET_ADDR_LENGTH);
  /* 0x1a .. 0x1d is ip.src.ip, 0x1e .. 0x21 is ip.dst.ip */
  /* 0x16 .. 0x1b is arp.src.mac, 0x1c .. 0x1f is arp.src.ip */
  /* 0x20 .. 0x25 is arp.dst.mac, 0x26 .. 0x29 is arp.dst.ip */

  int protocol = ntohs(*(u16 *)&data[12]);
  switch (protocol) {
  case ETH_P_IP: {
    break;
    memcpy(&data[0x1e], &eth_ip_addr, 4);
    ip_packet_modify_checksum(data, len);
  } break;
  case ETH_P_ARP:
    memcpy(&data[0x20], &eth_mac_addr, ENET_ADDR_LENGTH);
    memcpy(&data[0x26], &eth_ip_addr, 4);
    break;
  }
  // eth_packet_modify_crc(data, len);
}

void nat_send_data(const u8 *data, const int len) {
  static u8 eth_frame[2048];
  assert(len < sizeof(eth_frame));
  memcpy(eth_frame, data, len);

  pcap_write(pcap, data, len);
  pcap_flush(pcap);

  sender_modify_packet(eth_frame, len);

  eth_sll.sll_protocol = *(u16 *)&eth_frame[12];

  /* send the eth_frame */
  sendto(mac_socket, &eth_frame[0], len, 0, (struct sockaddr *)&eth_sll,
      sizeof(eth_sll));
}

int nat_recv_data(u8 *to, const int maxlen) {
  // printf("try receive data\n");
  int nbytes = recvfrom(mac_socket, to, maxlen, 0, NULL, NULL);

  recver_modify_packet(to, nbytes);
  pcap_write(pcap, to, nbytes);
  pcap_flush(pcap);
  return nbytes;
}

#endif
