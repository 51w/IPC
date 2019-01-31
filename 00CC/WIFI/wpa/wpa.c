#include <stdio.h>
#include <string.h>
#include "includes.h"
#include "wpa_ctrl.h"

static struct wpa_ctrl *ctrl_conn;

int main(int argc,char **argv)
{
    const char *global = "/var/run/wpa_supplicant/wlan0";
    ctrl_conn = wpa_ctrl_open(global);
    if (ctrl_conn == NULL)
    {
        fprintf(stderr, "Failed to connect to wpa_supplicant "
            "global interface: %s error: %s\n",global,strerror(errno));
        return -1;
    }
    else
    {
        printf("Success\n");
    }
    return 0;
}