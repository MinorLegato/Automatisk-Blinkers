
#include <stdio.h>

#include "ats/ats_tool.h"

#include "ats/ats_net.h"
#include "controller.c"

int main(void)
{
    NetInit();

    Controller  controller  = {0};
    Server      server      = {0};

    NetServerInit(&server, 8888);

    while (1) {
        NetServerRecv(&server, &controller, sizeof controller);

        printf("thrust:   %d\n",   controller.thrust);
        printf("steering: %d\n",   controller.steering);
        printf("blink:    %d\n\n", controller.blink);
    }

    NetDeinit();
}

