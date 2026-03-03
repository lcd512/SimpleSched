				AREA    |.text|, CODE, READONLY
				PRESERVE8
                THUMB
PendSV_Handler	PROC
				EXPORT	PendSV_Handler
				IMPORT	SScheduling
				;	判断任务使用指针寄存器
				;TST LR, #0x4				
				;ITE EQ							
				;MRSEQ R0,	MSP				
				;MRSNE R0,	PSP	
				;	取出默认MSP指针
				MRS R0,	MSP	
				;	判断任务是否使用FPU
				TST LR, #0x10		
				BEQ Save_FPUInfo		
				BNE Save_NFPUInfo		
Save_FPUInfo	
				SUB	R0,R0,#168
				VMRS R1,FPSCR			;	获取FPU状态值
				STMIA R0!,{R4-R11,LR}	;	压入LR FPU状态寄存器 和通用寄存器R4-R11
				VSTMIA R0!,{S0-S31}		;	压入浮点值寄存器
				STMIA R0!,{R1}			;	压入FPU状态寄存器值
				SUB	R0,R0,#168			
				B Save_FPUInfoEnd
Save_NFPUInfo
				SUB	R0,R0,#36
				STMIA R0!,{R4-R11,LR} 	;	压入LR FPU状态寄存器 和通用寄存器R4-R11
				SUB	R0,R0,#36
Save_FPUInfoEnd
	
				;	任务调度切换，输入保存上任务SP指针	并返回下一个任务栈Sp指针地址*/
				BL SScheduling
				LDMIA   R0!, {R4-R11,LR}		;//	获取任务lr值和通用寄存器值
				
				;判断任务是否使用FPU
				TST LR, #0x10				
				BEQ Read_FPUInfo		
				BNE Read_NFPUInfo		
Read_FPUInfo
				VLDMIA  R0!, {S0-S31}	;	获取浮点值寄存器值
				LDMIA R0!,{R1}			;	获取FPU状态寄存器值
				VMSR FPSCR,R1
Read_NFPUInfo
				;	判断任务使用指针寄存器
				;TST LR, #0x4	
				;ITE EQ				
				;MSREQ MSP, R0	
				;MSRNE PSP, R0
				;	载入新任务指针到MSP
				MSR MSP, R0	
				BX LR			
				ENDP	
PendSVHandSet	PROC
				EXPORT	PendSVHandSet
				PUSH {R0-R3,LR}
				LDR R0,=0xE000ED04
				STR R1,[R0]
				MOV R2,#0x1
				LSL R3,R2,#28
				ORR R1,R1,R3
				STR R1,[R0]
				POP{R0-R3,PC}
				ENDP
				ALIGN
                END
					