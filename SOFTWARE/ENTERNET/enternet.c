#include "includes.h"
#include "w5500.h"
#include "enternet.h"


/***********************************************
*
*				文件说明：此文件封装网络操作相关函数
*				详细说明：此文件封装的函数功能包括获取网络状态
*									打开关闭网络接口
*									发送接收网络数据包
*									必要网络参数的初始化
*									一些测试函数
*				更新时间：2018.12.8
*
***********************************************/



//网络参数
 u8 Sub_Mask[4]={0};
 u8 IP_Addr[4]={0};
 u8 Gateway_IP[4]={0};
 u8 DNS_SERVER[4]={114,114,114,114};//DNS服务器IP地址
 u8 NTP_SERVER[4]={202, 112, 10, 60};
//温控数据上传服务器地址
u8 SERVER_IP[4]={0};
u16 SERVER_PORT=6000;




void set_ntp_serverip (u8 *ip)
{
	mymemcpy (NTP_SERVER,ip,4);
}


void set_dns_serverip (u8 *ip)
{
	mymemcpy (DNS_SERVER,ip,4);
}









//通过udp网络发送数据，需提供目的地ip地址和端口
u8 udp_send(SOCKET s,u8 *ip,u16 com,u8 *databuff,u16 size)
{
	OS_ENTER_ONLYME();

	if((Read_W5500_SOCK_1Byte(s,Sn_MR)&0x0f) != SOCK_UDP)//如果Socket打开失败
	{		
		Write_W5500_SOCK_4Byte(s, Sn_DIPR, ip);//设置目的主机IP  		
		Write_W5500_SOCK_2Byte(s, Sn_DPORTR, com);//设置目的主机端口号				
	}
	
	Write_SOCK_Data_Buffer(s, databuff,size);		
	OS_EXIT_ONLYME();
	return 0;
}


u8 udp_init(SOCKET s,u16 mycom)
{
	u8 ret=FALSE;

	u8 temp=Read_W5500_SOCK_1Byte(s,Sn_SR);
	if(temp!=SOCK_CLOSED)
	{
		return FALSE;
	}
	
	//设置端口0的端口号
	Write_W5500_SOCK_2Byte(s, Sn_PORT,mycom);

	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_UDP);//设置Socket为UDP模式*/
	Write_W5500_SOCK_1Byte(s,Sn_CR,SN_OPEN);//打开Socket*/
	Delay(5);//延时5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_UDP)//如果Socket打开失败
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,SN_CLOSE);//打开不成功,关闭Socket
		ret= FALSE;//返回FALSE(0x00)
	}
	else
		ret= TRUE;
	return ret;
}


//通过tcp连接
u8 tcp_connect(SOCKET s,u16 mycom,u8 *ip,u16 com)
{ 
	
	if (!(ip[0]||ip[1]||ip[2]||ip[3]))
	{
		return FALSE;//ip地址不合法，失败
	}
	u8 temp=Read_W5500_SOCK_1Byte(s,Sn_SR);
	if(temp!=SOCK_CLOSED)
	{
		return FALSE;
	}
	
	
	
	//设置端口0的端口号
	Write_W5500_SOCK_2Byte(s, Sn_PORT,mycom);
	//设置端口0目的(远程)端口号
	Write_W5500_SOCK_2Byte(s, Sn_DPORTR, com);
	//设置端口0目的(远程)IP地址
	Write_W5500_SOCK_4Byte(s, Sn_DIPR, ip);			
	//设置50秒发送一次心跳包
	Write_W5500_SOCK_1Byte(s,Sn_KPALVTR, 10);
	
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_TCP);//设置socket为TCP模式
	Write_W5500_SOCK_1Byte(s,Sn_CR,SN_OPEN);//打开Socket
	u16 i=20;
	do
	{
		Delay(5);//延时5ms
		if(Read_W5500_SOCK_1Byte(s,Sn_SR)==SOCK_INIT)//如果socket打开成功
		{  
			break;
		}
		
	}while(--i);
	if (i==0)
	{
		return FALSE;
	}
	Write_W5500_SOCK_1Byte(s,Sn_CR,SN_CONNECT);//设置Socket为Connect模式
	i=20000;//TCP超时时间为31.8秒，这里延时100秒
	do
	{
		Delay(5);//延时5ms
		u8 temp=0;
		temp=Read_W5500_SOCK_1Byte(s,Sn_SR);
		if(temp==SOCK_ESTABLISHED)//如果socket连接成功
		{  
			break;
		}
		if (checkNetstateN (UNREACH))//目标地址不可达 
		{
			return FALSE;
		}
		if (checkSocketStateN(s,IR_TIMEOUT))//超时
		{
			return FALSE;
		}
		if (checkSocketStateN(s,IR_DISCON))//连接被断开
		{
			return FALSE;
		}
		if (temp==SOCK_CLOSED)		//连接关闭了
		{
			return FALSE;
		}
	}while(--i);
	
	if (i==0)
	{
		return FALSE;
	}
	return TRUE;//返回TRUE,设置成功
	
}


//通过tcp发送数据
u8 tcp_send(SOCKET s,u8 *databuff,u16 size)
{
	if (net_get_comstate(s)==SOCK_ESTABLISHED)
	{
		OS_ENTER_ONLYME();
		Write_SOCK_Data_Buffer(s, databuff,size);
		OS_EXIT_ONLYME();
		return 0;
	}
	else
	{
		return 1;
	}
}

//断开tcp连接
u8 tcp_close(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_CR,SN_DISCON);
	u16 i=2000;
	do
	{
		Delay(5);//延时5ms
		if(Read_W5500_SOCK_1Byte(s,Sn_SR)==SOCK_CLOSED)//如果socket关闭成功
		{  
			break;
		}
	}while(--i);
	if (i==0) return FALSE;
	return TRUE;//返回TRUE,设置成功

}

u8 udp_close(SOCKET s)
{
	
	Write_W5500_SOCK_1Byte(s,Sn_CR,SN_CLOSE);//设置Socket为Connect模式
	u8 i=20;
	do
	{
		Delay(1);//延时5ms
		if(Read_W5500_SOCK_1Byte(s,Sn_SR)==SOCK_CLOSED)//如果socket打开成功
		{  
			break;
		}
	}while(--i);
	
	if (i==0)
	{
		return FALSE;
	}
	return TRUE;//返回TRUE,设置成功

}

//判断端口状态并关闭
u8 socket_close (SOCKET s)
{
	u8 temp=Read_W5500_SOCK_1Byte(s,Sn_SR);
	if (temp==SOCK_CLOSED)
	{
		return TRUE;
	}
	else if ((temp==SOCK_INIT)||(temp==SOCK_LISTEN)||(temp==SOCK_UDP)||(temp==SOCK_MACRAW))
	{
		return udp_close(s);
	}
	else if ((temp==SOCK_SYNSEND)||(temp==SOCK_ESTABLISHED)||(temp==SOCK_SYNRECV)||(temp==SOCK_CLOSE_WAIT))
	{
		return tcp_close(s);
	}
	else
	{
//		do
//		{
//			delay_ms (5);
//			temp=Read_W5500_SOCK_1Byte(s,Sn_SR);
//			if (temp==SOCK_CLOSED) return TRUE;
//			if (temp==SOCK_CLOSE_WAIT) Write_W5500_SOCK_1Byte(s,Sn_CR,SN_DISCON);
//		}while (temp==SOCK_CLOSE_WAIT);
		return FALSE;
	}
}
 


//读取无法抵达的IP地址，端口号
u8 net_get_unsenddir (u8 *ip,u16 *port)
{
	u8 port_s[2]={0};
	Read_W5500_nByte (UIPR,ip,4);
	Read_W5500_nByte (UPORT,port_s,2);
	*port=(port_s[0]<<8)|port_s[1];
	return TRUE;
}






		//接收数据
u8 socket_recv(SOCKET s,u8 *databuff)
{
	OS_ENTER_ONLYME();
	Read_SOCK_Data_Buffer(s,databuff);
	OS_EXIT_ONLYME();
	return 0;
}




			//等待发送完成 
u8 net_wailt_sended(u8 socket)
{ 
	u16 i=0;
	if (socket==0)
	{
		while(!(checkSocketState(socket,IR_SEND_OK)))
		{
			delay_ms(100);
			i++;
			if (checkSocketState(socket,IR_TIMEOUT))
			{
				return FALSE;
			}
			if (i>20) return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}






//获取端口网络连接状态
u8 net_get_comstate(u8 socket)
{
	return Read_W5500_SOCK_1Byte(socket,Sn_SR);
}


		//设置网关ip
void net_set_getway(u8 *getwayip)
{
	Write_W5500_nByte(GAR, getwayip, 4);
}

		//设置子网掩码
void net_set_sub_mask(u8 *mask)
{
	Write_W5500_nByte(SUBR,mask,4);	
}

		//设置本机IP地址
void net_set_myip(u8 *myip)
{
	Write_W5500_nByte(SIPR,myip,4);	
}

//设置目的地IP地址及端口
void net_set_disip(u8 socket,u8 *disip,u16 discom,u16 mycom)
{
	
	Write_W5500_SOCK_2Byte(socket, Sn_MSSR, 1460);//最大分片字节数=1460(0x5b4)

	//设置端口0的端口号
	Write_W5500_SOCK_2Byte(socket, Sn_PORT, mycom);
	//设置端口0目的(远程)端口号
	Write_W5500_SOCK_2Byte(socket, Sn_DPORTR, discom);
	//设置端口0目的(远程)IP地址
	Write_W5500_SOCK_4Byte(socket, Sn_DIPR, disip);			
	
}



void net_init(net_parameters *t)
{
	net_set_getway(t->getwayip);
	net_set_sub_mask(t->submask);
	net_set_myip(t->myip);
	
}

void net_sysboot_init(void)
{	
	net_parameters t;
	t.getwayip=Gateway_IP;
	t.myip=IP_Addr;
	t.submask=Sub_Mask;
	net_init(&t);
}

			//返回1，已连接，0，未连接
u8 net_get_phycstate(void)
{
	return Read_W5500_1Byte(PHYCFGR)&LINK;
}



			//检查网关是否存在
u8 net_check_gateway(void)
{
	unsigned char ip_adde[4];
	extern u8 IP_Addr[4];
	ip_adde[0]=IP_Addr[0]+1;
	ip_adde[1]=IP_Addr[1]+1;
	ip_adde[2]=IP_Addr[2]+1;
	ip_adde[3]=IP_Addr[3]+1;

	//检查网关及获取网关的物理地址
	Write_W5500_SOCK_4Byte(0,Sn_DIPR,ip_adde);//向目的地址寄存器写入与本机IP不同的IP值
	Write_W5500_SOCK_1Byte(0,Sn_MR,MR_TCP);//设置socket为TCP模式
	Write_W5500_SOCK_1Byte(0,Sn_CR,SN_OPEN);//打开Socket	 
	Delay(5);//延时5ms 	
	
	if(Read_W5500_SOCK_1Byte(0,Sn_SR) != SOCK_INIT)//如果socket打开失败
	{
		Write_W5500_SOCK_1Byte(0,Sn_CR,SN_CLOSE);//打开不成功,关闭Socket
		return FALSE;//返回FALSE(0x00)
	}

	Write_W5500_SOCK_1Byte(0,Sn_CR,SN_CONNECT);//设置Socket为Connect模式						
	u8 i=255;
	do
	{
		u8 j=0;
		j=Read_W5500_SOCK_1Byte(0,Sn_IR);//读取Socket0中断标志寄存器
		if(j!=0)
		Write_W5500_SOCK_1Byte(0,Sn_IR,j);
		Delay(5);//延时5ms 
		if((j&IR_TIMEOUT) == IR_TIMEOUT)
		{
			return FALSE;	
		}
		else if(Read_W5500_SOCK_1Byte(0,Sn_DHAR) != 0xff)
		{
			Write_W5500_SOCK_1Byte(0,Sn_CR,SN_CLOSE);//关闭Socket
			return TRUE;							
		}
	}while(i--);
	return FALSE;
}




//0没有初始化好，1初始化好了
u8 net_check_parameters(void)
{
	
	if (!(Sub_Mask[0]||Sub_Mask[1]||Sub_Mask[2]||Sub_Mask[3]) )
	{
		return 0;
	}
	if (!(IP_Addr[0]&&IP_Addr[1]&&IP_Addr[2]&&IP_Addr[3]))
	{
		return 0;
	}
	if (!(Gateway_IP[0]&&Gateway_IP[1]&&Gateway_IP[2]&&Gateway_IP[3])) 
	{
		return 0;
	}
	
	return 1;
}









		//用于测试的，在这个函数内改变各项网络参数
		//调用这个函数之后，再调用net_sysboot_init()
void net_test(void)
{
	int_addr_struct *ipconfig=Get_MyIP();
	
	mymemcpy(IP_Addr,ipconfig->nativeIp,4);
	
	mymemcpy(SERVER_IP,ipconfig->serverIp,4);
	
	SERVER_PORT=ipconfig->serverPort;
	
	mymemcpy(Gateway_IP,ipconfig->getwayIp,4);
	
	mymemcpy(Sub_Mask,ipconfig->subMask,4);

}












