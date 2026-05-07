#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

#include "mi_sys.h"

#define GPIO_DEV "/sys/class/gpio"
#define SSTAR_GPIO_IN 0
#define SSTAR_GPIO_OUT 1

/* API call reference
    SStar_Gpio_Export(10);
    SStar_Gpio_SetDirection(10, SSTAR_GPIO_OUT);
    SStar_Gpio_SetValue(10, 0/1);
*/

// export gpio port
MI_S32 SStar_Gpio_Export(MI_S32 s32Gpio)
{
    MI_S32 s32Ret = -1;
    MI_S32 s32Fd = -1;

    char as8GpioDev[128];

    memset(as8GpioDev, 0, sizeof(as8GpioDev));
    sprintf(as8GpioDev, "%s/export", GPIO_DEV);

    s32Fd = open(as8GpioDev, O_WRONLY);

    if (s32Fd < 0)
        printf("failed to export gpio %d\n", s32Gpio);
    else
    {
        char as8GpioPort[10];
        memset(as8GpioPort, 0, sizeof(as8GpioPort));
        sprintf(as8GpioPort, "%d", s32Gpio);
        write(s32Fd, as8GpioPort, strlen(as8GpioPort));
        printf("export gpio port: %d\n", s32Gpio);
        close(s32Fd);
        s32Ret = MI_SUCCESS;
    }

    return s32Ret;
}

// set gpio direction: 0 in, others out
MI_S32 SStar_Gpio_SetDirection(MI_S32 s32Gpio, MI_S32 s32Direction)
{
    MI_S32 s32Ret  = -1;
    const char *ps8Direction = NULL;
    MI_S32 s32Fd = -1;

    char as8GpioDev[128];

    memset(as8GpioDev, 0, sizeof(as8GpioDev));
    sprintf(as8GpioDev, "%s/gpio%d/direction", GPIO_DEV, s32Gpio);

    s32Fd = open(as8GpioDev, O_RDWR);

    if (s32Fd < 0)
        printf("failed to set gpio%d direction\n", s32Gpio);
    else
    {
        if (SSTAR_GPIO_IN == s32Direction)
            ps8Direction = "in";
        else if (SSTAR_GPIO_OUT == s32Direction)
            ps8Direction = "out";
        else
        {
            printf("Unkown gpio direction %d\n", s32Direction);
        }
        write(s32Fd, ps8Direction, strlen(ps8Direction));
        printf("set gpio%d direction: %s\n", s32Gpio, ps8Direction);
        close(s32Fd);
        s32Ret = 0;
    }

    return s32Ret;
}

// get gpio direction
MI_S32 SStar_Gpio_GetDirection(MI_S32 s32Gpio, MI_S8 *ps8Direction, MI_S32 s32Len)
{
    MI_S32 s32Ret  = -1;
    MI_S32 s32Fd = -1;

    char as8GpioDev[128];

    memset(as8GpioDev, 0, sizeof(as8GpioDev));
    sprintf(as8GpioDev, "%s/gpio%d/direction", GPIO_DEV, s32Gpio);

    s32Fd = open(as8GpioDev, O_RDWR);

    if (s32Fd < 0)
        printf("failed to read gpio%d direction\n", s32Gpio);
    else
    {
        memset(ps8Direction, 0, s32Len);
        read(s32Fd, ps8Direction, s32Len);
        printf("get gpio%d direction: %s\n", s32Gpio, ps8Direction);
        close(s32Fd);
        s32Ret = MI_SUCCESS;
    }

    return s32Ret;
}

// set gpio value: 0 low, 1 high
MI_S32 SStar_Gpio_SetValue(MI_S32 s32Gpio, MI_S8 s8Value)
{
    MI_S32 s32Ret  = -1;
    MI_S32 s32Fd = -1;

    char as8GpioDev[128];

    memset(as8GpioDev, 0, sizeof(as8GpioDev));
    sprintf(as8GpioDev, "%s/gpio%d/value", GPIO_DEV, s32Gpio);

    s32Fd = open(as8GpioDev, O_RDWR);

    if (s32Fd < 0)
        printf("failed to set gpio%d value\n", s32Gpio);
    else
    {
        if (0 == s8Value)
        {
            write(s32Fd, (const void*)"0", 1);
        }
        else if (1 == s8Value)
        {
            write(s32Fd, (const void*)"1", 1);
        }
        else
        {
            printf("error:set gpio value fail (%d)\n", s8Value);
        }
        close(s32Fd);
        s32Ret = MI_SUCCESS;
    }

    return s32Ret;
}

// get gpio value
MI_S32 SStar_Gpio_GetValue(MI_S32 s32Gpio, MI_S8 *s8Level)
{
    MI_S32 s32Ret  = -1;
    MI_S32 s32Fd = -1;

    char as8GpioDev[128];
    char as8[2];

    memset(as8, 0, sizeof(as8));
    memset(as8GpioDev, 0, sizeof(as8GpioDev));
    sprintf(as8GpioDev, "%s/gpio%d/value", GPIO_DEV, s32Gpio);

    s32Fd = open(as8GpioDev, O_RDWR);

    if (s32Fd < 0)
        printf("failed to read gpio%d value\n", s32Gpio);
    else
    {
        read(s32Fd, as8, 1);
        *s8Level = atoi(as8);
        //printf("read gpio status: %s\n", s8Value);
        close(s32Fd);
        s32Ret = MI_SUCCESS;
    }

    return s32Ret;
}

// get gpio value
int new_SStar_Gpio_GetValue(MI_S32 s32Gpio)
{
    int value = -1;
    MI_S32 s32Fd = -1;

    char as8GpioDev[128];
    char as8[2];

    memset(as8, 0, sizeof(as8));
    memset(as8GpioDev, 0, sizeof(as8GpioDev));
    sprintf(as8GpioDev, "%s/gpio%d/value", GPIO_DEV, s32Gpio);

    s32Fd = open(as8GpioDev, O_RDWR);
    if (s32Fd < 0)
        printf("failed to read gpio%d value\n", s32Gpio);
    else
    {
        read(s32Fd, as8, 1);
        value = atoi(as8);
        close(s32Fd);
    }

    return value;
}



