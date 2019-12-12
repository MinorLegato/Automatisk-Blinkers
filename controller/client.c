
#include <stdio.h>
#include <stdint.h>

#include "ats/ats_tool.h"
#include "ats/ats_net.h"

#include "ats/ats_platform_glfw.h"
#include "ats/ats_render.h"

#include "controller.c"

static void ControllerUpdate(Controller *c, float t) {
    {
        c->thrust   = 0;
        c->steering = 0;
    }

    {
        const Gamepad *gp = &platform.gamepad[0];

        if (gp->active) {
            c->thrust   = INT8_MAX * gp->RT + INT8_MIN * gp->LT;

            if (fabsf(gp->LS.x) > 0.1f) 
                c->steering = INT8_MAX * gp->LS.x;

            if (gp->pressed.button.LB) c->blink--;
            if (gp->pressed.button.RB) c->blink++;
        }
    }

    {
        if (platform.keyboard.state[KEY_UP])   c->thrust =  127;
        if (platform.keyboard.state[KEY_DOWN]) c->thrust = -127;

        if (platform.keyboard.state[KEY_LEFT])  c->steering = -127;
        if (platform.keyboard.state[KEY_RIGHT]) c->steering = 127;

        if (platform.keyboard.pressed[KEY_Z]) c->blink--;
        if (platform.keyboard.pressed[KEY_X]) c->blink++;
    }

    c->blink = CLAMP(c->blink, -1, 1);
}

int main(void) {
    PlatformInit("CLIENT!", 800, 600, 8);

    //glfwSwapInterval(0);

    RenderInit();
    NetInit();

    Controller  controller  = {0};
    Client      client      = {0};

    const char *ip = "192.168.137.146";
    
    NetClientInit(&client, ip, 8888);

    while (!platform.close) {
        float t = platform.time.delta;

        if (platform.keyboard.pressed[KEY_ESCAPE])
            platform.close = true;

        ControllerUpdate(&controller, t);

        ControllerPackage cp = ControllerPackageCreate(controller);

        NetClientSend(&client, &cp, sizeof cp);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderSetCameraOrtho(platform.width, platform.height, -1.0f, 1.0f);

        RenderStringFormat(12, 1 * 16, 0, 12, 16, 1.0f, 0.5f, 0.0f, 1.0f, "thrust:    %d", controller.thrust);
        RenderStringFormat(12, 2 * 16, 0, 12, 16, 1.0f, 0.5f, 0.0f, 1.0f, "steering:  %d", controller.steering);
        RenderStringFormat(12, 3 * 16, 0, 12, 16, 1.0f, 0.5f, 0.0f, 1.0f, "blink:     %d", controller.blink);

        PlatformUpdate();
    }

    NetDeinit();
}

