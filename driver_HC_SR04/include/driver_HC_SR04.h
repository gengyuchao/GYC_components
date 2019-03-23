/*******************************************************************************

    Driver for HC-SR04 

  引脚连接：Trig--D6   Echo--D5
*******************************************************************************/
#ifndef __DRIVER_HC_SR04__
#define __DRIVER_HC_SR04__

void HC_SR04_Init();
void HC_SR04_Send_Wave();
void task_Get_Distance(void* time_xms);
extern float Distance;

#endif

