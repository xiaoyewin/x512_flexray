#include "derivative.h"      /* derivative-specific definitions */
#include "LCD.h"      




/*************************************************************/
/*                         延时函数1                         */
/*************************************************************/
void delay20us(unsigned int n) 
{
    unsigned int i;
    for(i=0;i<n;i++) 
    {
        TFLG1_C0F = 1;              //清除标志位
        TC0 = TCNT + 5;             //设置输出比较时间为20us
        while(TFLG1_C0F == 0);      //等待，直到发生输出比较事件
    }
}

/*************************************************************/
/*                         延时函数2                         */
/*************************************************************/
void delay1ms(unsigned int n) 
{
    unsigned int i;
    for(i=0;i<n;i++) 
    {
        TFLG1_C0F = 1;              //清除标志位
        TC0 = TCNT + 250;             //设置输出比较时间为1ms
        while(TFLG1_C0F == 0);      //等待，直到发生输出比较事件
    }
}










