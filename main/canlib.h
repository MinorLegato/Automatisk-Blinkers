
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

struct Can {
	int 		    socket;
	sockaddr_can	addr;
	can_frame	    frame;
	ifreq 		    ifr;
};

static void canInit(Can *can) {
	memset(can, 0, sizeof (Can));

	can->socket 		    = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	can->addr.can_family 	= AF_CAN;

	strcpy(can->ifr.ifr_name, "can0");
	ioctl(can->socket, SIOCGIFINDEX, &can->ifr);

	can->addr.can_ifindex	= can->ifr.ifr_ifindex;

	setsockopt(can->socket, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);

	bind(can->socket, (struct sockaddr *)&can->addr, sizeof(can->addr));
}

static void canSend(Can *can, unsigned id, void *data, size_t data_size) {
	can->frame.can_id	= id;
	can->frame.can_dlc 	= data_size > 8? 8 : data_size;

	memcpy(can->frame.data, data, data_size);

	write(can->socket, &can->frame, sizeof(can->frame));
}

