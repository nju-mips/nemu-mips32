#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <linux/virtio_net.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netpacket/packet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

/* /usr/include/netinet/ether.h */
// struct ether_header;
// struct ether_arp;
/* /usr/include/netinet/ip.h */
// struct iphdr
/* /usr/include/netinet/tcp.h */
// struct tcphdr
/* /usr/include/netinet/udphdr.h */
// struct udphdr

#include "debug.h"
#include "dev/events.h"
#include "utils/fifo.h"
#include "utils/vnet.h"

static pcap_handler pcap;

static int iface_fd = -1;
const char *iface_gw = "192.168.12.1";
// const char *iface_ipaddr = "192.168.12.2";
// static uint8_t iface_hwaddr[ETHER_ADDR_LEN];
static char iface_dev[IFNAMSIZ];

static fifo_type(struct iovec, 1024) net_queue;

const char *ipv4_ntoa(uint32_t ip) {
  static char s[128];
  uint8_t *p = (uint8_t *)&ip;
  sprintf(s, "%d.%d.%d.%d", p[0], p[1], p[2], p[3]);
  return s;
}

int route(const char *fmt, ...) {
  char cmd[1024];
  char *cmd_p = cmd;
  cmd_p += sprintf(cmd, "route ");
  va_list ap;
  va_start(ap, fmt);
  vsprintf(cmd_p, fmt, ap);
  va_end(ap);

  printf("%s\n", cmd);
  return system(cmd);
}

int iptables(const char *fmt, ...) {
  char cmd[1024];
  char *cmd_p = cmd;
  cmd_p += sprintf(cmd, "iptables ");
  va_list ap;
  va_start(ap, fmt);
  vsprintf(cmd_p, fmt, ap);
  va_end(ap);

  printf("%s\n", cmd);
  return system(cmd);
}

void iptables_add_route(const char *ip) {
  /* sudo iptables-save -t nat */
#if 0
  iptables("-t nat -D POSTROUTING -s %s/24 -d 224.0.0.0/24 -j RETURN", ip);
  iptables(
      "-t nat -D POSTROUTING -s %s/24 -d 255.255.255.255/32 -j RETURN", ip);
  iptables(
      "-t nat -D POSTROUTING -s %s/24 ! -d %s/24 -p tcp -j MASQUERADE "
      "--to-ports 1024-65535",
      ip, ip);
  iptables(
      "-t nat -D POSTROUTING -s %s/24 ! -d %s/24 -p udp -j MASQUERADE "
      "--to-ports 1024-65535",
      ip, ip);
  iptables("-t nat -D POSTROUTING -s %s/24 ! -d %s/24 -j MASQUERADE", ip, ip);
#endif

#if 0
  iptables("-t nat -A POSTROUTING -s %s/24 -d 224.0.0.0/24 -j RETURN", ip);
  iptables(
      "-t nat -A POSTROUTING -s %s/24 -d 255.255.255.255/32 -j RETURN", ip);
  iptables(
      "-t nat -A POSTROUTING -s %s/24 ! -d %s/24 -p tcp -j MASQUERADE "
      "--to-ports 1024-65535",
      ip, ip);
  iptables(
      "-t nat -A POSTROUTING -s %s/24 ! -d %s/24 -p udp -j MASQUERADE "
      "--to-ports 1024-65535",
      ip, ip);
  iptables("-t nat -A POSTROUTING -s %s/24 ! -d %s/24 -j MASQUERADE", ip, ip);
#endif
}

void init_network() {
  pcap = pcap_open("build/packets.pcap");

  fifo_init(net_queue);

  iface_fd = tap_create(iface_dev);
  assert(iface_fd > 0);

  int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  tap_set_down(sockfd, iface_dev);
  tap_set_ipaddr(sockfd, iface_dev, inet_addr(iface_gw));
  tap_set_netmask(
      sockfd, iface_dev, inet_addr("255.255.255.0"));
  /* tap_set_mtu(sockfd, iface_dev, 500 * 4); */
  tap_set_up(sockfd, iface_dev);
  close(sockfd);
  route("add -host %s gw %s", iface_gw, iface_gw);

  int flags = fcntl(iface_fd, F_GETFL);
  fcntl(iface_fd, F_SETFL, flags | O_NONBLOCK);
}

void net_send_data(const uint8_t *data, const int len) {
  pcap_write_and_flush(pcap, data, len);

  struct virtio_net_hdr hdr = {};
  struct iovec iov_copy[2] = {
      {.iov_base = &hdr,
          .iov_len = sizeof(struct virtio_net_hdr)},
      {.iov_base = (void *)data, .iov_len = len}};

  do {
    int ret = writev(iface_fd, iov_copy, 2);
    (void)ret;
  } while (len == -1 && errno == EINTR);
}

bool net_poll_packet() {
  bool has_packet = false;
  /* poll all packets firstly */
  int nbytes = 0;
  do {
    static uint8_t raw_buf[1800];
    nbytes = read(iface_fd, raw_buf, sizeof(raw_buf));
    has_packet = nbytes > 0;

    /* remove */
    const int vnet_hdr_len = sizeof(struct virtio_net_hdr);
    if (nbytes <= vnet_hdr_len) break;
    nbytes -= vnet_hdr_len;

    const void *buf_p = &raw_buf[vnet_hdr_len];
    pcap_write_and_flush(pcap, buf_p, nbytes);

    /* copy packet */
    void *buf_copy = malloc(nbytes);
    memcpy(buf_copy, buf_p, nbytes);

    struct iovec packet = {
        .iov_base = buf_copy, .iov_len = nbytes};

    fifo_push(net_queue, packet);
  } while (nbytes > 0);

  /* notify device the packets */
  while (!fifo_is_empty(net_queue)) {
    struct iovec packet = fifo_top(net_queue);
#if 0
    if (notify_event(EVENT_PACKET_IN, packet.iov_base,
            packet.iov_len) < 0)
      break;
#endif
    free(packet.iov_base);
    fifo_pop(net_queue);
    has_packet = true;
  }
  return has_packet;
}

#if 0
int net_recv_data(uint8_t *to, const int maxlen) {
  static uint8_t buf[2048];

  pcap_write_and_flush(pcap, NULL, 0);

  int vnet_hdr_len = sizeof(struct virtio_net_hdr);
  int nbytes = read(iface_fd, buf, maxlen);
  if (nbytes < 0 || nbytes < vnet_hdr_len) return -1;

  nbytes -= vnet_hdr_len;
  memcpy(to, buf + vnet_hdr_len, nbytes);
  pcap_write_and_flush(pcap, to, nbytes);

  return nbytes;
}
#endif
