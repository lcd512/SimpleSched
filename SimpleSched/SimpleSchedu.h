#ifndef __SimpleSched_H__
#define __SimpleSched_H__

typedef struct SSTask_T
{
	struct SSTask_T* last;					//	指针
	struct SSTask_T* next;					//	指针
	unsigned int   run_time;				//	任务栈运行时间
	unsigned int	 run_count;			//	运行计数
	unsigned int* 	stack;						//	任务栈起始地址			
	unsigned int   stack_size;			//	任务栈大小
	unsigned int*  sp;							//	任务栈sp指针器
	void (*fun)(void);							//	任务函数
}SSTask_T;

void SSched_SysTickHandler(void);

void SSched_Init(void);
void SSched_TaskInit(struct SSTask_T* task,unsigned int run_time,unsigned int* stack,unsigned int stack_size,void (*fun)(void));
void SSched_Run(void);
void SSched_SetRun(unsigned char run);

unsigned char SSched_AddTask(struct SSTask_T* task);
unsigned char SSched_DelTask(struct SSTask_T* task);
void SSched_Delay(unsigned long long delay_ms);


#endif

