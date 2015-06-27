#ifndef _BASE_GET_LOCAL_IP_H_
#define _BASE_GET_LOCAL_IP_H_

#include <sys/unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

#include <net/if.h>    // struct ifconf
#include <sys/ioctl.h> // SIOCGIFCONF

#include <vector>

namespace base {

static bool IsPrivateIP(uint32_t ip) {
  return ((ip >> 24) == 127)
      || ((ip >> 24) == 10)
      || ((ip >> 20) == ((172 << 4) | 1))
      || ((ip >> 16) == ((192 << 8) | 168));
}

std::string GetLocalIp() {
  int fd;
  std::vector<uint32_t> addrs;
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    fprintf(stderr, "nosocket\n");
    return "";
  }

  struct ifconf ifc;
  ifc.ifc_len = 64 * sizeof(struct ifreq);
  ifc.ifc_buf = new char[ifc.ifc_len];

  if (ioctl(fd, SIOCGIFCONF, &ifc) < 0) {
    fprintf(stderr, "ioctl error\n");
    return "";
  }
  // ASSERT(ifc.ifc_len < static_cast<int>(64 * sizeof(struct ifreq)));

  struct ifreq* ptr = reinterpret_cast<struct ifreq*>(ifc.ifc_buf);
  struct ifreq* end = reinterpret_cast<struct ifreq*>(ifc.ifc_buf + ifc.ifc_len);

  while (ptr < end) {
    if (strcmp(ptr->ifr_name, "lo")) { // Ignore the loopback device
      struct sockaddr_in* inaddr = reinterpret_cast<struct sockaddr_in*>(&ptr->ifr_ifru.ifru_addr);
      if (inaddr->sin_family == AF_INET) {
        uint32_t ip = ntohl(inaddr->sin_addr.s_addr);
        if (IsPrivateIP(ip)) {
          addrs.insert(addrs.begin(), ip);
        } else {
          addrs.push_back(ip);
        }
      }
    }
#ifdef _SIZEOF_ADDR_IFREQ
    ptr = reinterpret_cast<struct ifreq*>(
        reinterpret_cast<char*>(ptr) + _SIZEOF_ADDR_IFREQ(*ptr));
#else
    ptr++;
#endif
  }

  delete [] ifc.ifc_buf;
  close(fd);
  
  char ip_str[32];
  // uint32_t addr = addrs.back();
  uint32_t addr = addrs.front();
  snprintf(ip_str, 16, "%u.%u.%u.%u",
      ((addr >> 24) & 255),
      ((addr >> 16) & 255),
      ((addr >> 8) & 255),
      addr & 255);
  // printf("%s\n", ip_str);
  return ip_str;
}

}

#endif // _BASE_GET_LOCAL_IP_H_

