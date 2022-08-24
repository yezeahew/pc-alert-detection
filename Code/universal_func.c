#include "universal_func.h"
#include "main.h"

// Function to get ip address
//reference : https://www.geeksforgeeks.org/c-program-display-hostname-ip-address/
char* get_ip_address()
{
    int n;
    struct ifreq ifr;
    char array[] = "eth0";
    char *IP_addr;
 
    n = socket(AF_INET, SOCK_DGRAM, 0);
    //Type of address to retrieve - IPv4 IP address
    ifr.ifr_addr.sa_family = AF_INET;
    //Copy the interface name in the ifreq structure
    strncpy(ifr.ifr_name , array , IFNAMSIZ - 1);
    ioctl(n, SIOCGIFADDR, &ifr);
    close(n);
    
    IP_addr =  inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr);
    return IP_addr;
}

// get the mac address of the node
// reference: https://stackoverflow.com/questions/1779715/how-to-get-mac-address-of-your-machine-using-a-c-program# (mpromonet's solution)
char* retrieve_mac_addr() {
    struct ifaddrs *ifaddr=NULL;
    struct ifaddrs *ifa = NULL;
    int i = 0, j = 0;
    char mac_arr[18];
    char* mac_address;

    if (getifaddrs(&ifaddr) == -1)
    {
         perror("getifaddrs");
    }
    else
    {
         for ( ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
         {
             if ( (ifa->ifa_addr) && (ifa->ifa_addr->sa_family == AF_PACKET) &&  (strcmp(ifa -> ifa_name, "eth0") == 0))
             {
                  struct sockaddr_ll *s = (struct sockaddr_ll*)ifa->ifa_addr;
                  for (i=0; i <s->sll_halen; i++)
                  {
                    sprintf(&mac_arr[j], "%02x%c", (s->sll_addr[i]), (i+1!=s->sll_halen)?':':'\n');
                    j += 3;
                  }
                
                mac_address = mac_arr;
             }
         }
         freeifaddrs(ifaddr);
    }
    return mac_address;
}

