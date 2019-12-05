
#include "canlib.h"
#include "net.h"

#include <stdint.h>

#define ARRAY_COUNT(array) (sizeof (array) / sizeof (array[0]))

struct Controller
{
	int8_t 	thrust;
	int8_t 	steering;
	int8_t 	blink;
};

//unsigned char data[] = { 0, 1, 2, 3, 4, 5, 6 };
//canSend(&can, 0x7DF, data, ARRAY_COUNT(data));

int main(void)
{
	Can 		can         = {0};
	Server 		server      = {0};
	Controller 	controller  = {0};

    puts("can");
	canInit(&can);
    puts("net");
	netServerInit(&server, 8888);

	while (1) {
        puts("net recv");
		netServerRecv(&server, &controller, sizeof (Controller));

		printf("t %d\n", controller.thrust);
		printf("s %d\n", controller.steering);
		printf("b %d\n", controller.blink);

        puts("can send");
		canSend(&can, 0x7DF, &controller, sizeof (Controller));
	}

	return 0;
}

