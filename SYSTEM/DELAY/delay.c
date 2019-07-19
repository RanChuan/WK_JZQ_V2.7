#include "includes.h"
#include "delay.h"






static u8  fac_us=0;//us延时倍乘数
static u16 fac_ms=0;//ms延时倍乘数

#define  MICRO_MS  2//5毫秒中断一次


//初始化延迟函数
//SYSTICK的时钟固定为HCLK时钟的1/8
//SYSCLK:系统时钟
void delay_init()	 
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);	//选择外部时钟  HCLK/8
	fac_us=SystemCoreClock/8000000;	//为系统时钟的1/8  
	fac_ms=(u16)fac_us*1000;//非ucos下,代表每个ms需要的systick时钟数   
	
	SysTick->LOAD=(u32)MICRO_MS*fac_ms;//时间加载(SysTick->LOAD为24bit)

	NVIC_SetPriority(SysTick_IRQn,0xf);//设置中断优先级为最低
	SysTick->CTRL|=3;//使能计数器，开启中断
	
}								    
	

//大于MICRO_MS的延时在这里面会形成死循环，大于MICRO_MS的延时请使用delay_ms
//
void delay_us(u32 nus)
{
	u32 old,now;
	old=SysTick->VAL;
	while (1)
	{
		now=SysTick->VAL;
		if (old>now)
		{
			if (((old-now)/fac_us)>=nus)//递减计数器，故旧值减新值
				break;
		}
		else 
		{
			if (((old+(MICRO_MS*fac_ms)-now)/fac_us)>=nus)
				break;
		}
	}
}

u32 delay_ms(u16 nms)
{	 
 #if OS_CRITICAL_METHOD == 3          /* Allocate storage for CPU status register */
		 OS_CPU_SR  cpu_sr;
 #endif
	if (OSRunning)
	{
		OS_ENTER_CRITICAL(); 
		TCB_Table[OSPrioHighRdy].MYDelay_ms=nms/MICRO_MS+1;//根据延时毫秒数设置节拍
		OS_EXIT_CRITICAL();
		return TaskGetMsg();
	}
	else
	{
		while (nms--)
		{
			delay_us(1000);
		}
		return 0;
	}
} 


//强制睡眠，结束前不能被唤醒
u32 sleep_ms(u16 nms)
{	 
 #if OS_CRITICAL_METHOD == 3          /* Allocate storage for CPU status register */
		 OS_CPU_SR  cpu_sr;
 #endif
	
	if (OSRunning)
	{
		u32 msg=0;
		OS_ENTER_CRITICAL(); 
		TCB_Table[OSPrioHighRdy].MYDelay_ms=nms/MICRO_MS+1;//根据延时毫秒数设置节拍
		OS_EXIT_CRITICAL();
		do{
			msg|=TaskGetMsg();
		}while((msg&DELAY_END)==0);
		return msg;
	}
	else
	{
		while (nms--)
		{
			delay_us(1000);
		}
		return 0;
	}
} 






//系统运行时间，单位秒
static u32 SYS_RUNTIME=0;

void RunTime_IRQHandler (void)
{
	static u16 micro_ms_t=0;//最小时间片个数
	micro_ms_t+=MICRO_MS;
	if (micro_ms_t>=1000)
	{
		micro_ms_t-=1000;
		SYS_RUNTIME++;
	}
}

//获取系统运行时间
u32 getSysRunTime(void)
{
	return SYS_RUNTIME;
}



//	防止出现长时间占用CPU的情况，下下策
void cpuBreakIRQ (void)
{
	static u16 cpu=0;
	if (OS_GET_ONLYME())
	{
		cpu++;
		if (cpu>=500)
		{
			cpu=0;
			OS_EXIT_ONLYME();
		}
	}
	else
	{
		cpu=0;
	}	
}






void SysTick_Handler (void)
{
	u8 i=0;
	for(i=0;i<32;i++)
	{
		if (TCB_Table[i].MYDelay_ms>0) 
		{
			TCB_Table[i].MYDelay_ms--;
			if (TCB_Table[i].MYDelay_ms==0)
			{ 
				TaskIntSendMsg(i,DELAY_END);//在计数到0时发送一次任务唤醒
			}
		}
	}
	RunTime_IRQHandler();
	//CheckTaskUsege();在这里校验堆栈会占用大量中断时间
}




























