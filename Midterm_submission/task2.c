//Noted functions were used from:
// https://www.digikey.com/eewiki/display/microcontroller/I2C+Communication+with+the+TI+Tiva+TM4C123GXL

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "inc/hw_i2c.h"
#include "driverlib/i2c.h"
#include "inc/hw_gpio.h"
#include "utils/uartstdio.h"
#include "utils/uartstdio.c"

static uint32_t mpu6050add = 0x68;

static uint8_t acc_xh_reg = 59;
static uint8_t acc_xl_reg = 60;
static uint8_t acc_yh_reg = 61;
static uint8_t acc_yl_reg = 62;
static uint8_t acc_zh_reg = 63;
static uint8_t acc_zl_reg = 64;

static uint8_t gyro_xh_reg = 67;
static uint8_t gyro_xl_reg = 68;
static uint8_t gyro_yh_reg = 69;
static uint8_t gyro_yl_reg = 70;
static uint8_t gyro_zh_reg = 71;
static uint8_t gyro_zl_reg = 72;


//setting up I2C (function was taken from source
void InitI2C0(void){
    //enable I2C module 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);

    //reset module
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0);

    //enable GPIO peripheral that contains I2C 0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    // Configure the pin muxing for I2C0 functions on port B2 and B3.
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);

    // Select the I2C function for these pins.
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

    // Enable and initialize the I2C0 master module.  Use the system clock for
    // the I2C0 module.  The last parameter sets the I2C data transfer rate.
    // If false the data rate is set to 100kbps and if true the data rate will
    // be set to 400kbps.
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);

    //clear I2C FIFOs
    HWREG(I2C0_BASE + I2C_O_FIFOCTL) = 80008000;
}

//read specified register on slave device
uint32_t I2CReceive(uint32_t slave_addr, uint8_t reg){
    //specify that we are writing (a register address) to the
    //slave device
    I2CMasterSlaveAddrSet(I2C0_BASE, slave_addr, false);

    //specify register to be read
    I2CMasterDataPut(I2C0_BASE, reg);

    //send control byte and register address byte to slave device
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);

    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2C0_BASE));

    //specify that we are going to read from slave device
    I2CMasterSlaveAddrSet(I2C0_BASE, slave_addr, true);

    //send control byte and read from the register we
    //specified
    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);

    //wait for MCU to finish transaction
    while(I2CMasterBusy(I2C0_BASE));

    //return data pulled from the specified register
    uint16_t x = I2CMasterDataGet(I2C0_BASE);
    return I2CMasterDataGet(I2C0_BASE);
}

void I2CSend(uint8_t slave_addr, uint8_t num_of_args, ...){
    // Tell the master module what address it will place on the bus when
    // communicating with the slave.
    I2CMasterSlaveAddrSet(I2C0_BASE, slave_addr, false);

    //stores list of variable number of arguments
    va_list vargs;

    //specifies the va_list to "open" and the last fixed argument
    //so vargs knows where to start looking
    va_start(vargs, num_of_args);

    //put data to be sent into FIFO
    I2CMasterDataPut(I2C0_BASE, va_arg(vargs, uint32_t));

    //if there is only one argument, we only need to use the
    //single send I2C function
    if(num_of_args == 1)
    {
        //Initiate send of data from the MCU
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);

        // Wait until MCU is done transferring.
        while(I2CMasterBusy(I2C0_BASE));

        //"close" variable argument list
        va_end(vargs);
    }

    //otherwise, we start transmission of multiple bytes on the
    //I2C bus
    else
    {
        //Initiate send of data from the MCU
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);

        // Wait until MCU is done transferring.
        while(I2CMasterBusy(I2C0_BASE));

        //send num_of_args-2 pieces of data, using the
        //BURST_SEND_CONT command of the I2C modules
        uint8_t i;
        for(i = 1; i < (num_of_args - 1); i++)
        {
            //put next piece of data into I2C FIFO
            I2CMasterDataPut(I2C0_BASE, va_arg(vargs, uint32_t));
            //send next data that was just placed into FIFO
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);

            // Wait until MCU is done transferring.
            while(I2CMasterBusy(I2C0_BASE));
        }

        //put last piece of data into I2C FIFO
        I2CMasterDataPut(I2C0_BASE, va_arg(vargs, uint32_t));
        //send next data that was just placed into FIFO
        I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
        // Wait until MCU is done transferring.
        while(I2CMasterBusy(I2C0_BASE));

        //"close" variable args list
        va_end(vargs);
    }
}
void InitConsole(void){

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    UARTStdioConfig(0, 115200, 16000000);
}

int main(void) {

    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    InitConsole();
    InitI2C0();

    uint16_t accel_x;
    uint16_t accel_y;
    uint16_t accel_z;

    uint16_t gyro_x;
    uint16_t gyro_y;
    uint16_t gyro_z;

    I2CSend(mpu6050add,0,0x6B);

    while (1)
    {
        accel_x = I2CReceive(mpu6050add, acc_xh_reg);
        accel_x += I2CReceive(mpu6050add, acc_xl_reg);
        accel_y = I2CReceive(mpu6050add, acc_yh_reg);
        accel_y +=I2CReceive(mpu6050add, acc_yl_reg);
        accel_z =I2CReceive(mpu6050add, acc_zh_reg);
        accel_z +=I2CReceive(mpu6050add, acc_zl_reg);

        gyro_x = I2CReceive(mpu6050add, gyro_xh_reg);
        gyro_x +=I2CReceive(mpu6050add, gyro_xl_reg);
        gyro_y =I2CReceive(mpu6050add, gyro_yh_reg);
        gyro_y +=I2CReceive(mpu6050add, gyro_yl_reg);
        gyro_z =I2CReceive(mpu6050add, gyro_zh_reg);
        gyro_z +=I2CReceive(mpu6050add, gyro_zl_reg);

        UARTprintf("AccelX:%d,", accel_x);
        UARTprintf("AccelY:%d,", accel_y);
        UARTprintf("AccelZ:%d,", accel_z);

        UARTprintf("GyroX:%d,", gyro_x);
        UARTprintf("GyroY:%d,", gyro_x);
        UARTprintf("GyroZ:%d\n", gyro_x);

    }

}

