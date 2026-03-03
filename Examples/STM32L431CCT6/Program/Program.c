#include "Program.h"
#include "main.h"
SSTask_T Task0 = {0};
SSTask_T Task1 = {0};
unsigned int Task0_Stack[256] = {0};
unsigned int Task1_Stack[256] = {0};

void Task0_Fun(void)
{
	volatile static unsigned int a = 0;
	while(1)
	{
		a++;
		SSched_Delay(1000);
	}
}



void Task1_Fun(void)
{
	volatile static unsigned int b = 0;
	while(1)
	{
		b++;
		SSched_Delay(2000);
	}
}



/*
*	功能	:		初始化
*	参数	:		无
*	返回值:		无
*/
void Init(void)
{
	SSched_Init();
	
	SSched_TaskInit(&Task0,1,Task0_Stack,256,Task0_Fun);
	SSched_TaskInit(&Task1,1,Task1_Stack,256,Task1_Fun);
	SSched_AddTask(&Task0);
	SSched_AddTask(&Task1);
	
	SSched_Run();
	while(1)
	{

	}
}




