#define TARGET_IS_BLIZZARD_RB1
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"

#ifndef M_PI
#define M_PI                    3.14159265358979323846
#endif

#define SERIES_LENGTH 1000

float gSeriesData[SERIES_LENGTH];

int32_t i32DataCount = 0;

int main(void)
{
    float fRadians;

    ROM_FPULazyStackingEnable();
    ROM_FPUEnable();

    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    fRadians = ((2 * M_PI) / SERIES_LENGTH); // determine value of radians

    while(i32DataCount < SERIES_LENGTH) // number of data points is under series length
    {
        //gSeriesData[i32DataCount] = sinf(fRadians * i32DataCount); assign value to array
        gSeriesData[i32DataCount]  = sinf(2*M_PI*50*(fRadians*i32DataCount))+(.5*cosf(2*M_PI*200*(fRadians*i32DataCount)));
        i32DataCount++;
    }

    while(1)
    {
    }
}
