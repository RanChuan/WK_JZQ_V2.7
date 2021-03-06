#include "includes.h"
#include "beep.h"
#include "cscript.h"
#include "swd.h"
#include "file.h"

//读取SD卡中存储的配置信息，0，成功，1，失败
u8 read_config (void)
{
	FRESULT ret;
	UINT real_length=0;
	if (fats_state()==0) return 1;
	ret=FATS->f_open(file,_T("0:/wk_config.bcfg"),FA_OPEN_ALWAYS|FA_READ|FA_WRITE);
	if (ret==FR_OK)
	{
		FATS->f_read(file,EN_CONFIG,CONFIG_DATA_NUM*2,&real_length);
		FATS->f_close(file);
		if (real_length!=CONFIG_DATA_NUM*2) return 3;
		else return 0;
	}
	else
	{
		return 2;
	}
}


//读取SD卡中存储的配置信息，0，成功，1，失败
u8 write_config (void)
{
	FRESULT ret;
	UINT real_length=0;
	if (fats_state()==0) return 1;
	ret=FATS->f_open(file,_T("0:/wk_config.json"),FA_OPEN_ALWAYS|FA_READ|FA_WRITE);
	if (ret==FR_OK)
	{
		FATS->f_write(file,EN_CONFIG,CONFIG_DATA_NUM*2,&real_length);
		FATS->f_close(file);
		if (real_length!=CONFIG_DATA_NUM*2) return 3;
	}
	else
	{
		return 2;
	}
	return 0;
}

//函数作用：读取json文件，删除文件中的无用字符，空格，回车等
//参数：file_name,要打开的文件名，buf是缓存区地址，buff_size是缓存区大小
//返回：0，成功，1，失败
u8 read_json(TCHAR *file_name,u8 *buf,u16 buff_size)
{
	FRESULT ret;
	FILINFO fileinfo;
	UINT readsize=0;
	if (fats_state()==0) return 1;//不支持文件系统，返回
	TCHAR *file_type=file_name+strlen(file_name)-5;
	if (strcmp((const char*)file_type,(const char*)".json")!=0)//判断json格式，不是json格式就返回
	{
		return 1;
	}
	ret=FATS->f_open(file,file_name,FA_READ);
	if (ret!=FR_OK)
	{
		return 1;//打开失败，返回
	}
	ret=FATS->f_stat(file_name,&fileinfo);
	if (ret!=FR_OK)
	{
		return 1;//读取失败，返回
	}
	if (fileinfo.fsize>FILE_MAX_SIZE)//文件太大，受不了
	{
		FATS->f_close(file);
		return 1;
	}
	u8 *file_buf=0;
	file_buf=mymalloc(fileinfo.fsize);
	if (file_buf==0)//内存分配失败，返回
	{
		FATS->f_close(file);
		return 1;	
	}
	ret=FATS->f_read(file,file_buf,fileinfo.fsize,&readsize);
	if (ret!=FR_OK)
	{
		myfree(file_buf);
		return 1;
	}
	u8 *file_ptr=file_buf;//文件操作指针
	while (readsize--)
	{
		if (*file_ptr==' ') file_ptr++;
		else if (*file_ptr=='\t') file_ptr++;
		else if (*file_ptr=='\r') file_ptr++;
		else if (*file_ptr=='\n') file_ptr++;
		else
		{
			*buf++=*file_ptr++;
			buff_size--;
			if (buff_size==0) break;//接收json字符串的缓存不足
		}
	}
	myfree(file_buf);//释放内存
	if (readsize!=0)//转换没有完成
	{
		return 1;
	}
	return 0;//转换完成
	
}




//运行系统初始化脚本
u8 run_sysinit (void)
{
	FRESULT ret;
	UINT real_length=0;
	char *script_buff=mymalloc(1024);
	if (fats_state()==0) 
	{
		myfree(script_buff);
		return 1;
	}
	ret=FATS->f_open(file,_T("0:/wk_init.txt"),FA_OPEN_ALWAYS|FA_READ|FA_WRITE);
	if (ret==FR_OK)
	{
		FATS->f_read(file,script_buff,1024,&real_length);
		FATS->f_close(file);
		if (real_length==0) 
		{
			myfree(script_buff);
			return 3;
		}
		else
		{
			runCScript (script_buff);
			myfree(script_buff);
		}
	}
	else
	{
		myfree(script_buff);
		return 2;
	}
}




//播放文件里的beep音乐
u8 run_beep (const char *name_str)
{
	FRESULT ret;
	UINT real_length=0;
	char *beep_buff=mymalloc(2048);
	if (beep_buff==0) return 5;
	mymemset (beep_buff,0,2048);
	if (fats_state()==0) 
	{
		myfree(beep_buff);
		return 1;
	}
	ret=FATS->f_open(file,_T(name_str),FA_OPEN_ALWAYS|FA_READ|FA_WRITE);
	if (ret==FR_OK)
	{
		FATS->f_read(file,beep_buff,2048,&real_length);
		FATS->f_close(file);
		if (real_length==0) 
		{
			myfree(beep_buff);
			return 3;
		}
		else
		{ 
			beep_playBy (beep_buff);//播放指定旋律的音乐
			myfree(beep_buff);
		}
	}
	else
	{
		myfree(beep_buff);
		return 2;
	}
}














