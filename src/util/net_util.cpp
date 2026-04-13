#include "ynet/util/net_util.h"
#include <ifaddrs.h>
#include <arpa/inet.h>

namespace ynet {
    std::string getExternalIP() {
        struct ifaddrs* ifaddr;
        if(getifaddrs(&ifaddr) == -1) {
            return "unknown";
        } 

        std::string result = "unknown";
        for(struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
            if(ifa->ifa_addr == nullptr) continue;

            if(ifa->ifa_addr->sa_family != AF_INET) continue;

            struct sockaddr_in* addr = (struct sockaddr_in*)ifa->ifa_addr;
            std::string ip = inet_ntoa(addr->sin_addr);
            if(ip == "127.0.0.1") continue;

            result = ip;
            break;
        }

        freeifaddrs(ifaddr);
        return result;
        
    }
}
