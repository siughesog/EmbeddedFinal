#include<reg52.h> //包含頭文件，一般情況不需要改動，頭文件包含特殊功能寄存器的定義


#define DataPort P0 //定義數據端口 程序中遇到DataPort 則用P0 替換
#define KeyPort P1
sbit LATCH1=P2^2;//定義鎖存使能端口 段鎖存
sbit LATCH2=P2^3;//                 位鎖存

unsigned char code dofly_DuanMa[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,
		                  	         0x77,0x7c,0x39,0x5e,0x79,0x71};// 顯示段碼值0~F
unsigned char code dofly_WeiMa[]={0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};//分別對應相應的數碼管點亮,即位碼
unsigned char TempData[10]; //存儲顯示值的全局變量


int readCounter = 0;
void DelayUs2x(unsigned char t);//us級延時函數聲明 
void DelayMs(unsigned char t); //ms級延時
void Display(unsigned char FirstBit,unsigned char Num);//數碼管顯示函數
unsigned char KeyScan(void);//鍵盤掃瞄
unsigned char KeyPro(void);
void Init_Timer0(void);//定時器初始化

void InitUART  (void)
{

    SCON  = 0x50;		        // SCON: 模式 1, 8-bit UART, 使能接收  
    TMOD |= 0x20;               // TMOD: timer 1, mode 2, 8-bit 重裝
    TH1   = 0xFD;               // TH1:  重裝值 9600 波特率 晶振 11.0592MHz  
    TR1   = 1;                  // TR1:  timer 1 打開                         
    EA    = 1;                  //打開總中斷
    ES    = 1;                  //打開串口中斷
}  


void SendByte(unsigned char dat)
{
 SBUF = dat;
 while(!TI);
      TI = 0;
}

void UART_SER (void) interrupt 4
{
    unsigned char Temp;
   
   if(RI)                      
     {
	  RI=0;                     
	  Temp=SBUF;
	  if(readCounter < 3){
	  	TempData[readCounter] = dofly_DuanMa[Temp];//收到esp32的溫度資訊 ex:23.25度，收到的是2,3,2,5
		readCounter++;
	  }else{
	  	 TempData[readCounter] = dofly_DuanMa[Temp];
		 TempData[1] = TempData[1] | 0x80;//小數點
		 readCounter = 0;
	  }

	 }
   if(TI)                      
     TI=0;
} 





void main (void)
{
unsigned char num,i,j;                  
unsigned char temp[8];
bit Flag;
int setFlag = 0;
Init_Timer0();
InitUART();

while (1)        
  {
	num=KeyPro();
	if(num >= 0 && num < 17)
		SBUF = num;//傳送按鍵編號，對應到esp32裡面的縣市list
  }
  Display(0, 4);
}

void DelayUs2x(unsigned char t)
{   
 while(--t);
}

void DelayMs(unsigned char t)
{
     
 while(t--)
 {
     DelayUs2x(245);
	 DelayUs2x(245);
 }
}

void Display(unsigned char FirstBit,unsigned char Num)
{
      static unsigned char i=0;
	  

	   DataPort=0;   //清空數據，防止有交替重影
       LATCH1=1;     //段鎖存
       LATCH1=0;

       DataPort=dofly_WeiMa[i+FirstBit]; //取位碼 
       LATCH2=1;     //位鎖存
       LATCH2=0;

       DataPort=TempData[i]; //取顯示數據，段碼
       LATCH1=1;     //段鎖存
       LATCH1=0;
       
	   i++;
       if(i==Num)
	      i=0;


}

void Init_Timer0(void)
{
 TMOD |= 0x01;	  //使用模式1，16位定時器，使用"|"符號可以在使用多個定時器時不受影響		     
 //TH0=0x00;	      //給定初值
 //TL0=0x00;
 EA=1;            //總中斷打開
 ET0=1;           //定時器中斷打開
 TR0=1;           //定時器開關打開
}
/*------------------------------------------------
                 定時器中斷子程序
------------------------------------------------*/
void Timer0_isr(void) interrupt 1 
{
 TH0=(65536-2000)/256;		  //重新賦值 2ms
 TL0=(65536-2000)%256;
 
 Display(0,8);       // 調用數碼管掃瞄

}


unsigned char KeyScan(void)  //鍵盤掃瞄函數，使用行列反轉掃瞄法
{
 unsigned char cord_h,cord_l;//行列值中間變量
 KeyPort=0x0f;            //行線輸出全為0
 cord_h=KeyPort&0x0f;     //讀入列線值
 if(cord_h!=0x0f)    //先檢測有無按鍵按下
 {
  DelayMs(10);        //去抖
  if((KeyPort&0x0f)!=0x0f)
  {
    cord_h=KeyPort&0x0f;  //讀入列線值
    KeyPort=cord_h|0xf0;  //輸出當前列線值
    cord_l=KeyPort&0xf0;  //讀入行線值

    while((KeyPort&0xf0)!=0xf0);//等待鬆開並輸出

    return(cord_h+cord_l);//鍵盤最後組合碼值
   }
  }return(0xff);     //返回該值
}

unsigned char KeyPro(void)
{
 switch(KeyScan())
 {
  case 0x7e:return 0;break;//0 
  case 0x7d:return 1;break;//1
  case 0x7b:return 2;break;//2
  case 0x77:return 3;break;//3
  case 0xbe:return 4;break;//4
  case 0xbd:return 5;break;//5
  case 0xbb:return 6;break;//6
  case 0xb7:return 7;break;//7
  case 0xde:return 8;break;//8
  case 0xdd:return 9;break;//9
  case 0xdb:return 10;break;//a
  case 0xd7:return 11;break;//b
  case 0xee:return 12;break;//c
  case 0xed:return 13;break;//d
  case 0xeb:return 14;break;//e
  case 0xe7:return 15;break;//f
  default:return 0xff;break;
 }
}