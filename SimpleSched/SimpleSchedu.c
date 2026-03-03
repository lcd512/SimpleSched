#include "SimpleSchedu.h"
#include "SimpleSchedu_Config.h"

typedef struct TaskList_T
{
	unsigned int num;								//	数量
	struct SSTask_T* head;			//	头指针
	unsigned char lock;								//	操作锁
}TaskList_T;


typedef struct SimpleSched_T
{
	struct SSTask_T* Run;					//	运行任务
	struct TaskList_T Ready;			//	就绪任务
	volatile unsigned long long Tick;			//	节拍计时
	unsigned char State;					//	运行状态
}SimpleSched_T;

static SimpleSched_T SimpleSched = {0};
/*	初始任务	*/
struct SSTask_T SSMainTask = {0};


/*
*	添加一个任务
*	@param		list	|		任务列表
*	@param		task	|		任务节点
*	@return		0失败	|		1成功
*/
static unsigned char ListAddTask(struct TaskList_T* list,struct SSTask_T* task)
{
	unsigned char state = 0;
	//	上锁
	if(++list->lock != 1)
	{
		list->lock--;
		goto re;
	}
	//	判断是否第一个核心
	if(list->head == 0)
	{
		//	设置头尾指针
		task->last = task;
		task->next = task;
		list->head = task;
		//list->tail = node;
	}
	else
	{
		task->last = list->head->last;
		task->next = list->head;
		
		list->head->last->next = task;
		list->head->last = task;
		//	list->tail = node;
	}
	state = 1;
	list->num++;
	//	解锁
	list->lock--;
re:
	return state;
}


/*
*	删除一个任务
*	@param		list	|		任务列表
*	@param		task	|		任务节点
*	@return		0失败	|		1成功
*/
static unsigned char ListDelTask(struct TaskList_T* list,struct SSTask_T* task)
{
	unsigned char state = 0;
	//	上锁
	if(++list->lock != 1)
	{
		list->lock--;
		goto re;
	}
	//	判断是否有核心
	if(list->head == 0)
	{
		list->lock--;
		goto re;
	}
	
	//	判断是否只要一个核心
	if(list->head->next == list->head)
	{
		list->head = 0;				
		//	list->tail = OS_NULL;
	}
	else
	{
		task->next->last = task->last;
		task->last->next = task->next;
		if(list->head == task)
			list->head = task->next;
	}
	state = 1;
	list->num--;
	//	解锁
	list->lock--;
re:
	return state;
}

/*
*	功能		:		任务栈初始化
*	参数		:		tack		|		任务地址
*	返回值	:		无
*/
static void SSchedTaskStack_Init(struct SSTask_T* task)
{
	unsigned int *sp = task->sp-16;	//	算上LR
	//	设置SP值
	task->sp = sp;
	//	载入R4-R11寄存器值
	for(int i=0;i<8;i++)
	{
		sp[i] = 0;
	}
	sp[8] = 0xFFFFFFF9;//	特殊LR值 默认使用MSP特权模式
	//	载入R0 R1 R2 R3 R12 LR PC XPSR
	sp[9]  = 0;
	sp[10] = 0;
	sp[11] = 0;
	sp[12] = 0;
	sp[13] = 0;
	sp[14] = 0;
	sp[15] = (unsigned int)task->fun;
	//	PSR寄存器值 #0x01000000	THUMB模式
	sp[16] = 0x01000000;
}


/*
*设置任务参数
*	@param	tack				|		任务地址
*	@param	run_time		|		任务运行时间片长度
*	@param	stack				|		任务栈地址
*	@param	stack_size	|		任务栈大小
*	@param	fun					|		任务函数
*	返回值	:		无
*/
void SSched_TaskInit(struct SSTask_T* task,unsigned int run_time,unsigned int* stack,unsigned int stack_size,void (*fun)(void))
{
	//	初始化任务栈
	task->stack = &stack[stack_size-1];		//	注意栈生长方向
	task->stack_size = stack_size;
	task->sp		=	task->stack;
	//	运行时间
	task->run_time = run_time;
	task->run_count = 0;
	task->fun = fun;
	//	初始化堆栈
	SSchedTaskStack_Init(task);
}


/*
*	任务切换并返回下一个任务sp地址
*	@param		
*	@return		下一个任务sp地址	
*/
void* SScheduling(void* sp)
{
	SimpleSched.Run->sp = sp;
	SimpleSched.Run = SimpleSched.Run->next;
	return SimpleSched.Run->sp;
}

/*
*	调度定时器中断处理函数
*	@param
*	@return
*/
inline void SSched_SysTickHandler(void)
{
	if(SimpleSched.State == 1)
	{
		//	时钟节拍计数
		SimpleSched.Tick += SSched_TimeStep;
		//	呼叫osPendSVHandler切换任务
		if(SimpleSched.Run->run_count++ == SimpleSched.Run->run_time)
		{
			SimpleSched.Run->run_count = 0;
			SSched_PenSVCall();
		}
	}
}


/*
*	初始化
*	@param
*	@return
*/
void SSched_Init(void)
{
	SimpleSched.Run = 0;
	SimpleSched.State = 0;
	SimpleSched.Tick = 0;
	SimpleSched.Ready.head = 0;
	SimpleSched.Ready.lock = 0;
	SimpleSched.Ready.num = 0;
	//	设置初始任务
	SimpleSched.Run = &SSMainTask;
}

/*
*	运行
*	@param
*	@return
*/
void SSched_Run(void)
{
	if(SimpleSched.Ready.head != 0)
	{
		SSMainTask.next = SimpleSched.Ready.head;
		SimpleSched.State = 1;
	}
}


/*
*	添加任务
*	@param			task 		|		任务
*	@return			0	失败	|		1	成功
*/
unsigned char SSched_AddTask(struct SSTask_T* task)
{
	return ListAddTask(&SimpleSched.Ready,task);
}

/*
*	删除任务
*	@param			task 		|		任务
*	@return			0	失败	|		1	成功
*/
unsigned char SSched_DelTask(struct SSTask_T* task)
{
	return ListDelTask(&SimpleSched.Ready,task);
}

/*
*	任务延时函数-不准
*	@param		delay_ms				延时时间ms
*	@return
*/
void SSched_Delay(unsigned long long delay_ms)
{
  unsigned long long tickstart = SimpleSched.Tick;
  if (delay_ms < 0xFFFFFFFF)
  {
    delay_ms += SSched_TimeStep;
  }
  while(SimpleSched.Tick - tickstart  < delay_ms);
}


/*
*	设置任务调度
*	@param		0关闭 |1开启
*	@return
*/
void SSched_SetRun(unsigned char run)
{
	SimpleSched.State = run;
}

