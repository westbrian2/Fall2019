#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"


uint32_t ui32PeriodOn;
uint32_t ui32PeriodOff;




void Timer1ADelay(int ttime){ //delays by .5seconds
    int i;

    SYSCTL_RCGCTIMER_R |=2;
    TIMER1_CTL_R=0;
    TIMER1_CFG_R=0x04;
    TIMER1_TAMR_R=0x02;
    TIMER1_TAILR_R=64000-1;
    TIMER1_TAPR_R=625-1;
    TIMER1_ICR_R=0x1;
    TIMER1_CTL_R|=0x1;
    for(i=0;i<ttime;i++){
        while((TIMER1_RIS_R &0x01)==0); //wait for timeout
        TIMER1_ICR_R = 0x1; //clear flag
    }
}
void ButtonInterrupt(void){
    int status = 0; //used to check which pin's interrupt is triggered (best practice)

    status =GPIOIntStatus(GPIO_PORTF_BASE,true);
    GPIOIntClear(GPIO_PORTF_BASE,GPIO_INT_PIN_0); //Clears that pin's interrupt
    if(status & GPIO_INT_PIN_0){
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 4);
        Timer1ADelay(5); //delay for 1 sec
    }


    SysCtlDelay(1000000); //debouncing?
}


int main(void)
{


    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    HWREG(GPIO_PORTF_BASE+GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(GPIO_PORTF_BASE+GPIO_O_CR) |= GPIO_PIN_0;

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    //set up button
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,GPIO_PIN_0); //sets button pin as input
    GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU); //configures button settings

    GPIOIntTypeSet(GPIO_PORTF_BASE,GPIO_PIN_0,GPIO_RISING_EDGE); //Determines what signal triggers interrupt
    //GPIOIntRegister(GPIO_PORTF_BASE,ButtonInterrupt);//Associates pin group with interrupt(?)
    GPIOIntEnable(GPIO_PORTF_BASE,GPIO_INT_PIN_0); //enables pin interrupt
    IntEnable(INT_GPIOF);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    ui32PeriodOn = (SysCtlClockGet() / 10)*.43; //variable to determine on time
    ui32PeriodOff = (SysCtlClockGet() / 10)*.57; //variable to determine off time
    TimerLoadSet(TIMER0_BASE, TIMER_A, ui32PeriodOff -1); //loading timer to enter interrupt

    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntMasterEnable();

    TimerEnable(TIMER0_BASE, TIMER_A);

    while(1)
    {
    }
}




void Timer0IntHandler(void)
{
    // Clear the timer interrupt
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Read the current state of the GPIO pin and
    // write back the opposite state
    if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2))
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
        TimerLoadSet(TIMER0_BASE, TIMER_A, ui32PeriodOff -1);
    }
    else
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 4);
        TimerLoadSet(TIMER0_BASE, TIMER_A, ui32PeriodOn -1);
    }
}
