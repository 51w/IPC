#include <sys/types.h>  
#include <sys/socket.h>  
#include <asm/types.h>  
#include <linux/netlink.h>  
#include <linux/rtnetlink.h>  
#include <unistd.h>
#include <stdlib.h>  
#include <stdio.h>  
#include <sys/ioctl.h>  
#include <linux/if.h>  
#include <string.h>  
  
#define BUFLEN 20480  
  
int main(int argc, char *argv[])  
{  
    int fd, retval;  
    char buf[BUFLEN] = {0};  
    int len = BUFLEN;  
    struct sockaddr_nl addr;  
    struct nlmsghdr *nh;  
    struct ifinfomsg *ifinfo;  
    struct rtattr *attr;  
  
    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);  
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &len, sizeof(len));  
    memset(&addr, 0, sizeof(addr));  
    addr.nl_family = AF_NETLINK;  
    addr.nl_groups = RTNLGRP_LINK;  
    bind(fd, (struct sockaddr*)&addr, sizeof(addr));  
    while ((retval = read(fd, buf, BUFLEN)) > 0)  
    {  
        for (nh = (struct nlmsghdr *)buf; NLMSG_OK(nh, retval); nh = NLMSG_NEXT(nh, retval))  
        {  
            if (nh->nlmsg_type == NLMSG_DONE)  
                break;  
            else if (nh->nlmsg_type == NLMSG_ERROR)  
                return -1;  
            else if (nh->nlmsg_type != RTM_NEWLINK)  
                continue;  
            ifinfo = NLMSG_DATA(nh);  
            printf("%u: %s", ifinfo->ifi_index,  
                    (ifinfo->ifi_flags & IFF_LOWER_UP) ? "up" : "down" );  
            attr = (struct rtattr*)(((char*)nh) + NLMSG_SPACE(sizeof(*ifinfo)));  
            len = nh->nlmsg_len - NLMSG_SPACE(sizeof(*ifinfo));  
            for (; RTA_OK(attr, len); attr = RTA_NEXT(attr, len))  
            {  
                if (attr->rta_type == IFLA_IFNAME)  
                {  
                    printf(" %s", (char*)RTA_DATA(attr));  
                    break;  
                }  
            }  
            printf("\n");  
        }  
    }  
  
    return 0;  
}