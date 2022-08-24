#ifndef UNIVERSAL_FUNC_H
#define UNIVERSAL_FUNC_H

#include "main.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <netpacket/packet.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>


char* retrieve_mac_addr();
char* get_ip_address();

#endif // UNIVERSAL_FUNC_H





