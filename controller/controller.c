#include <stdint.h>
#include "../lib/crc32.h"

typedef struct Controller
{
    int8_t  thrust;
    int8_t  steering;
    int8_t  blink;
} Controller;

typedef struct ControllerPackage
{
    uint32_t        crc32;
    //
    Controller      controller;
} ControllerPackage;

static ControllerPackage
ControllerPackageCreate(Controller controller)
{
    ControllerPackage cp = {0};

    cp.crc32        = CRC32Code(&controller, sizeof (controller));
    cp.controller   = controller;

    return cp;
}

