#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/if_tun.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netpacket/packet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils/vnet.h"

int tap_create(char dev[IFNAMSIZ]) {
  struct ifreq ifr = {0};
  int fd, err;

  if ((fd = open("/dev/net/tun", O_RDWR)) < 0) return -1;

  ifr.ifr_flags =
      IFF_TAP | IFF_NO_PI | IFF_VNET_HDR | IFF_MULTI_QUEUE;

  if (*dev) { strncpy(ifr.ifr_name, dev, IFNAMSIZ); }

  if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
    close(fd);
    return err;
  }
  strncpy(dev, ifr.ifr_name, IFNAMSIZ);
  return fd;
}

int tap_set_hwaddr(int sockfd, const char *dev,
    const uint8_t ether_addr[ETHER_ADDR_LEN]) {
  /* ifconfig tap0 hw ether 72:99:c8:4e:78:5c */
  struct ifreq ifr = {0};
  strcpy(ifr.ifr_name, dev);

  ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
  memcpy(
      ifr.ifr_hwaddr.sa_data, ether_addr, ETHER_ADDR_LEN);
  return ioctl(sockfd, SIOCSIFHWADDR, (void *)&ifr);
}

int tap_set_netmask(
    int sockfd, const char *dev, const uint32_t netmask) {
  struct ifreq ifr = {0};
  strcpy(ifr.ifr_name, dev);

  struct sockaddr_in *addr = (void *)&ifr.ifr_addr;
  addr->sin_family = AF_INET;
  addr->sin_port = 0;
  addr->sin_addr.s_addr = netmask;

  /* ifconfig tap0 10.0.1.5 # set ip addr */
  return ioctl(sockfd, SIOCSIFNETMASK, (void *)&ifr);
}

int tap_set_ipaddr(
    int sockfd, const char *dev, const uint32_t ipaddr) {
  struct ifreq ifr = {0};
  strcpy(ifr.ifr_name, dev);

  struct sockaddr_in *addr = (void *)&ifr.ifr_addr;
  addr->sin_family = AF_INET;
  addr->sin_port = 0;
  addr->sin_addr.s_addr = ipaddr;

  /* ifconfig tap0 10.0.1.5 # set ip addr */
  return ioctl(sockfd, SIOCSIFADDR, (void *)&ifr);
}

int tap_set_mtu(int sockfd, const char *dev, int mtu) {
  struct ifreq ifr = {0};
  strcpy(ifr.ifr_name, dev);

  /* ifconfig mtu 10.0.1.5 # set mtu */
  ifr.ifr_mtu = mtu;
  return ioctl(sockfd, SIOCSIFMTU, (void *)&ifr);
}

int tap_set_up(int sockfd, const char *dev) {
  struct ifreq ifr = {0};
  strcpy(ifr.ifr_name, dev);

  /* get interface status */
  if (ioctl(sockfd, SIOCGIFFLAGS, (void *)&ifr) < 0)
    return -1;

  /* ifup tap0 # start up */
  ifr.ifr_flags |= IFF_UP;
  if (ioctl(sockfd, SIOCSIFFLAGS, (void *)&ifr) < 0)
    return -1;
  return 0;
}

int tap_set_down(int sockfd, const char *dev) {
  struct ifreq ifr = {0};
  strcpy(ifr.ifr_name, dev);

  /* get interface status */
  if (ioctl(sockfd, SIOCGIFFLAGS, (void *)&ifr) < 0)
    return -1;

  /* ifup tap0 # start up */
  ifr.ifr_flags &= ~IFF_UP;
  if (ioctl(sockfd, SIOCSIFFLAGS, (void *)&ifr) < 0)
    return -1;
  return 0;
}

int tap_set_attribute(const char *dev,
    const uint32_t ipaddr,
    const uint8_t ether_addr[ETHER_ADDR_LEN], int mtu) {
  int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
  if (tap_set_down(sockfd, dev) < 0) return -1;
  if (tap_set_hwaddr(sockfd, dev, ether_addr) < 0)
    return -1;
  if (tap_set_ipaddr(sockfd, dev, ipaddr) < 0) return -1;
  if (tap_set_mtu(sockfd, dev, mtu) < 0) return -1;
  // if (tap_set_netmask(sockfd, dev, netmask) < 0) return
  // -1;
  if (tap_set_up(sockfd, dev) < 0) return -1;
  close(sockfd);
  return 0;
}
