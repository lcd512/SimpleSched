#ifndef __Cortex_M4_Port_H__
#define __Cortex_M4_Port_H__

#ifdef __cplusplus
extern "C" {
#endif

//	中断接口
/*	中断函数接口	*/
#define SSched_PenSVCall()					PendSVHandSet()											//	系统PendSVHandler中断唤起

/*	函数声明		*/
void PendSVHandSet(void);

#ifdef __cplusplus
}
#endif

#endif

