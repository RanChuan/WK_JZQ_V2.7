#ifndef __DELAY_H
#define __DELAY_H 			   


#ifdef __cplusplus
	extern "C" {
#endif


//滴答定时器初始化
//使用函数函数必须首先初始化滴答定时器
void delay_init(void);

//毫秒级延时
//在系统开始运行之前误差为0ms
//在系统开始运行之后误差为+nms,n为系统节拍
//在任务中调用该函数最慢是在延时时间结束之前返回
//如果在此之前有事件发生，则提前返回
u32 delay_ms(u16 nms);

//毫秒级延时
//在系统开始运行之前误差为0ms，调用这个函数效果和delay_ms()相同
//在系统开始运行之后误差为+nms,n为系统节拍
//在任务中调用该函数总是在延时时间结束之前返回
//如果在此之前有事件发生，则记录发生的事件，在延时结束时返回
u32 sleep_ms(u16 nms);

//微秒级延时
//这个函数可以在系统开始前或开始后调用，误差为0us
//延时时间长度不要超过系统节拍
//阻塞延时，总是在延时结束时返回
void delay_us(u32 nus);

//获取系统运行时间
//系统运行时间在滴答定时器开始运行之后开始计时
//在滴答定时器开始运行之前调用总是返回0
u32 getSysRunTime(void);



#ifdef __cplusplus
	}
#endif




#endif





























