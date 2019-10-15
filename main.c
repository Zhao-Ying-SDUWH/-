#include <STC8A.H>
#include <intrins.h>
#include <string.h>


sbit STAT_pinn = P0^0;
int angle[3]={0};
unsigned char i,sum;
unsigned char xd[7] ;//定义数组，有7个元素
bit send2_ok = 0;
bit receive2_ok = 0;
bit receive_ADC_ok = 0;
//串口1初始化
void UartInit(void)		//9600bps@24.000MHz
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x01;		//串口1选择定时器2为波特率发生器
	AUXR |= 0x04;		//定时器2时钟为Fosc,即1T
	T2L = 0x8F;		//设定定时初值
	T2H = 0xFD;		//设定定时初值
	AUXR |= 0x10;		//启动定时器2
}
//串口2初始化
void Uart2Init(void)		//9600bps@24.000MHz
{
	S2CON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x04;		//定时器2时钟为Fosc,即1T
	T2L = 0x8F;		//设定定时初值
	T2H = 0xFD;		//设定定时初值
	AUXR |= 0x10;		//启动定时器2
}
//ADC初始化
void ADC_Init()
{
  P1M0 &= ~(1<<2) ;  //设置P1.2为ADC口
	P1M1 |=  (1<<2);  
	ADCCFG = 0x0f;  //设置ADC时钟为系统时钟/2/16/16
	ADC_CONTR = 0x80;  //使能ADC模块
	EADC = 1;//使能ADC中断
	EA = 1;
	ADC_CONTR |= 0x40;  //启动AD转换
}

//void set_baudrate(float frequency,long int baudrate)
//{	
//	unsigned int itmp; 	
//	unsigned char tlow,thigh;

//	IRC24MCR = 0x80;//使能内部24M高精度IRC振荡器
//	
//	AUXR |= 0x01;	  //选择定时器2为串口1波特率发生器
//	AUXR |= 0x04;		//定时器2速度控制位，不分频1T
//	
//	SCON = 0x50;    //可变波特率8位数据方式
//	PCON = 0x00;    //串口1的各个模式波特率都不加倍,SMOD=0
//	S2CON = 0x50;   //允许串口接收数据，选择定时器2为串口2波特率发生器
//	//S3CON = 0x10;   //允许串口接收数据，选择定时器2为串口3波特率发生器
//	//S4CON = 0x10;   //允许串口接收数据，选择定时器2为串口4波特率发生器
//	
//	itmp = (int)(65536-(frequency*1000000)/(baudrate*4));
//	thigh = itmp/256; 	
//	tlow = itmp%256; 	

//	TH2 = thigh; 	
//	TL2 = tlow; 	

//	AUXR |= 0x10;		//启动定时器2
//}

//串口1发送
bit send_ok = 0;
void UartSend(char aChar)
{
	
	SBUF = aChar;
	while(TI==0);
	TI=0;
}


//串口2发送
void UART2_Send_Char(char aChar)
{
	S2BUF = aChar; 	
	send2_ok = 1;
	while(send2_ok)        //等待发送缓存为空
	{
		if(S2CON & 0x02)     //S2TI = 1
	  {
		  S2CON &= ~0x02;
 			send2_ok=0;
		}
	}
}
unsigned char a=0,angle_data[11]={0};
void main(void)
{
   UartInit();
	 Uart2Init();
	UART2_Send_Char(0xA5);
	UART2_Send_Char(0x45);//发送读方位角指令
	UART2_Send_Char(0xEA);
	memset(angle,0,3);
	
	EA =1;           //开总中断 
  
	
	while( STAT_pinn==0 );
	
 	while(1)
	{
		
		IE2 |= ES2;//打开串口2中断
		EADC = 0;//关闭数模转换中断
		while(!receive2_ok);
		
		if(receive2_ok) //串口接收完毕
		{
		  
			IE2 = 0x00;//关闭串口2中断
      
			UART2_Send_Char(0xA5);
		  UART2_Send_Char(0x75);//发送读方位角指令
		  UART2_Send_Char(0x1A);
			
			for(sum=0,i=0;i<(angle_data[3]+4);i++)//方位角angle_data[3]=2
			{
				sum+=angle_data[i];
			}
			
			if(sum==angle_data[10])                //校验和判断
			{
				angle[0]=(angle_data[4]<<8)|angle_data[5];
				angle[1]=(angle_data[6]<<8)|angle_data[7];
				angle[2]=(angle_data[8]<<8)|angle_data[9];//实际值的100倍
				
		    xd[0] = angle_data[4];//将需要的六轴数据写入数组
		    xd[1] = angle_data[5];
		    xd[2] = angle_data[6];
		    xd[3] = angle_data[7];
		    xd[4] = angle_data[8];
		    xd[5] = angle_data[9];
				

			}			
			receive2_ok=0;                               //处理数据完毕标志	
      
    UART2_Send_Char(0xA5);
		UART2_Send_Char(0x45);//发送读方位角指令
		UART2_Send_Char(0xEA);			
		}
    ADC_Init();
		ADC_Init();
	if(receive_ADC_ok==1)
		{
		
				xd[7] = 0x00;//标志位
				UartSend(xd[0]);
		    UartSend(xd[1]);
		    UartSend(xd[2]);
		    UartSend(xd[3]);
		    UartSend(xd[4]);
		    UartSend(xd[5]);
		    UartSend(xd[6]);
			receive_ADC_ok=0; //防止重复发送
			}
		}
	}

//void UART2_Isr() interrupt 8
//{
//	
//	if(S2CON & 0x02)          //S2TI = 1
//	{
//		S2CON &= ~0x02;
//		send2_ok = 0;
//	}
//		
//	if(S2CON & 0x01)          //S2RI = 1
//	{
//		angle_data[a]=S2BUF;
//		a += 1;
//		S2CON &= ~0x01;
//		if (a==11)
//    {
//				a = 0;
//				receive2_ok = 1;
//    }
//	}
//}
//数模转换中断
void ADC_Isr() interrupt 5 
{
	ADC_CONTR &= ~0x20;  //清完成标志
	xd[6] = ADC_RES;  //读取ADC结果
	receive_ADC_ok=1; //接收完成
}


