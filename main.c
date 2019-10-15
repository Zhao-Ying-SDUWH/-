#include <STC8A.H>
#include <intrins.h>
#include <string.h>


sbit STAT_pinn = P0^0;
int angle[3]={0};
unsigned char i,sum;
unsigned char xd[7] ;//�������飬��7��Ԫ��
bit send2_ok = 0;
bit receive2_ok = 0;
bit receive_ADC_ok = 0;
//����1��ʼ��
void UartInit(void)		//9600bps@24.000MHz
{
	SCON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x01;		//����1ѡ��ʱ��2Ϊ�����ʷ�����
	AUXR |= 0x04;		//��ʱ��2ʱ��ΪFosc,��1T
	T2L = 0x8F;		//�趨��ʱ��ֵ
	T2H = 0xFD;		//�趨��ʱ��ֵ
	AUXR |= 0x10;		//������ʱ��2
}
//����2��ʼ��
void Uart2Init(void)		//9600bps@24.000MHz
{
	S2CON = 0x50;		//8λ����,�ɱ䲨����
	AUXR |= 0x04;		//��ʱ��2ʱ��ΪFosc,��1T
	T2L = 0x8F;		//�趨��ʱ��ֵ
	T2H = 0xFD;		//�趨��ʱ��ֵ
	AUXR |= 0x10;		//������ʱ��2
}
//ADC��ʼ��
void ADC_Init()
{
  P1M0 &= ~(1<<2) ;  //����P1.2ΪADC��
	P1M1 |=  (1<<2);  
	ADCCFG = 0x0f;  //����ADCʱ��Ϊϵͳʱ��/2/16/16
	ADC_CONTR = 0x80;  //ʹ��ADCģ��
	EADC = 1;//ʹ��ADC�ж�
	EA = 1;
	ADC_CONTR |= 0x40;  //����ADת��
}

//����1����
bit send_ok = 0;
void UartSend(char aChar)
{
	
	SBUF = aChar;
	while(TI==0);
	TI=0;
}


//����2����
void UART2_Send_Char(char aChar)
{
	S2BUF = aChar; 	
	send2_ok = 1;
	while(send2_ok)        //�ȴ����ͻ���Ϊ��
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
	UART2_Send_Char(0x45);//���Ͷ���λ��ָ��
	UART2_Send_Char(0xEA);
	memset(angle,0,3);
	
	EA =1;           //�����ж� 
  
	
	while( STAT_pinn==0 );
	
 	while(1)
	{
		
		IE2 |= ES2;//�򿪴���2�ж�
		EADC = 0;//�ر���ģת���ж�
		while(!receive2_ok);
		
		if(receive2_ok) //���ڽ������
		{
		  
			IE2 = 0x00;//�رմ���2�ж�
      
			UART2_Send_Char(0xA5);
		  UART2_Send_Char(0x75);//���Ͷ���λ��ָ��
		  UART2_Send_Char(0x1A);
			
			for(sum=0,i=0;i<(angle_data[3]+4);i++)//��λ��angle_data[3]=2
			{
				sum+=angle_data[i];
			}
			
			if(sum==angle_data[10])                //У����ж�
			{
				angle[0]=(angle_data[4]<<8)|angle_data[5];
				angle[1]=(angle_data[6]<<8)|angle_data[7];
				angle[2]=(angle_data[8]<<8)|angle_data[9];//ʵ��ֵ��100��
				
		    xd[0] = angle_data[4];//����Ҫ����������д������
		    xd[1] = angle_data[5];
		    xd[2] = angle_data[6];
		    xd[3] = angle_data[7];
		    xd[4] = angle_data[8];
		    xd[5] = angle_data[9];
				

			}			
			receive2_ok=0;                               //����������ϱ�־	
      
    UART2_Send_Char(0xA5);
		UART2_Send_Char(0x45);//���Ͷ���λ��ָ��
		UART2_Send_Char(0xEA);			
		}
    ADC_Init();
		ADC_Init();
	if(receive_ADC_ok==1)
		{
		
				xd[7] = 0x00;//��־λ
				UartSend(xd[0]);
		    UartSend(xd[1]);
		    UartSend(xd[2]);
		    UartSend(xd[3]);
		    UartSend(xd[4]);
		    UartSend(xd[5]);
		    UartSend(xd[6]);
			receive_ADC_ok=0; //��ֹ�ظ�����
			}
		}
	}
//����2�ж�
void UART2_Isr() interrupt 8
{
	
	if(S2CON & 0x02)          //S2TI = 1
	{
		S2CON &= ~0x02;
		send2_ok = 0;
	}
		
	if(S2CON & 0x01)          //S2RI = 1
	{
		angle_data[a]=S2BUF;
		a += 1;
		S2CON &= ~0x01;
		if (a==11)
    {
				a = 0;
				receive2_ok = 1;
    }
	}
}
//��ģת���ж�
void ADC_Isr() interrupt 5 
{
	ADC_CONTR &= ~0x20;  //����ɱ�־
	xd[6] = ADC_RES;  //��ȡADC���
	receive_ADC_ok=1; //�������
}


