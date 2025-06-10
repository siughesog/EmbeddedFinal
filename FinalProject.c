#include<reg52.h> //�]�t�Y���A�@�뱡�p���ݭn��ʡA�Y���]�t�S��\��H�s�����w�q


#define DataPort P0 //�w�q�ƾںݤf �{�Ǥ��J��DataPort �h��P0 ����
#define KeyPort P1
sbit LATCH1=P2^2;//�w�q��s�ϯ�ݤf �q��s
sbit LATCH2=P2^3;//                 ����s

unsigned char code dofly_DuanMa[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,
		                  	         0x77,0x7c,0x39,0x5e,0x79,0x71};// ��ܬq�X��0~F
unsigned char code dofly_WeiMa[]={0xfe,0xfd,0xfb,0xf7,0xef,0xdf,0xbf,0x7f};//���O�����������ƽX���I�G,�Y��X
unsigned char TempData[10]; //�s�x��ܭȪ������ܶq


int readCounter = 0;
void DelayUs2x(unsigned char t);//us�ũ��ɨ���n�� 
void DelayMs(unsigned char t); //ms�ũ���
void Display(unsigned char FirstBit,unsigned char Num);//�ƽX����ܨ��
unsigned char KeyScan(void);//��L����
unsigned char KeyPro(void);
void Init_Timer0(void);//�w�ɾ���l��

void InitUART  (void)
{

    SCON  = 0x50;		        // SCON: �Ҧ� 1, 8-bit UART, �ϯ౵��  
    TMOD |= 0x20;               // TMOD: timer 1, mode 2, 8-bit ����
    TH1   = 0xFD;               // TH1:  ���˭� 9600 �i�S�v ���� 11.0592MHz  
    TR1   = 1;                  // TR1:  timer 1 ���}                         
    EA    = 1;                  //���}�`���_
    ES    = 1;                  //���}��f���_
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
	  	TempData[readCounter] = dofly_DuanMa[Temp];//����esp32���ū׸�T ex:23.25�סA���쪺�O2,3,2,5
		readCounter++;
	  }else{
	  	 TempData[readCounter] = dofly_DuanMa[Temp];
		 TempData[1] = TempData[1] | 0x80;//�p���I
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
		SBUF = num;//�ǰe����s���A������esp32�̭�������list
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
	  

	   DataPort=0;   //�M�żƾڡA���������v
       LATCH1=1;     //�q��s
       LATCH1=0;

       DataPort=dofly_WeiMa[i+FirstBit]; //����X 
       LATCH2=1;     //����s
       LATCH2=0;

       DataPort=TempData[i]; //����ܼƾڡA�q�X
       LATCH1=1;     //�q��s
       LATCH1=0;
       
	   i++;
       if(i==Num)
	      i=0;


}

void Init_Timer0(void)
{
 TMOD |= 0x01;	  //�ϥμҦ�1�A16��w�ɾ��A�ϥ�"|"�Ÿ��i�H�b�ϥΦh�өw�ɾ��ɤ����v�T		     
 //TH0=0x00;	      //���w���
 //TL0=0x00;
 EA=1;            //�`���_���}
 ET0=1;           //�w�ɾ����_���}
 TR0=1;           //�w�ɾ��}�����}
}
/*------------------------------------------------
                 �w�ɾ����_�l�{��
------------------------------------------------*/
void Timer0_isr(void) interrupt 1 
{
 TH0=(65536-2000)/256;		  //���s��� 2ms
 TL0=(65536-2000)%256;
 
 Display(0,8);       // �եμƽX�ޱ���

}


unsigned char KeyScan(void)  //��L���˨�ơA�ϥΦ�C���౽�˪k
{
 unsigned char cord_h,cord_l;//��C�Ȥ����ܶq
 KeyPort=0x0f;            //��u��X����0
 cord_h=KeyPort&0x0f;     //Ū�J�C�u��
 if(cord_h!=0x0f)    //���˴����L������U
 {
  DelayMs(10);        //�h��
  if((KeyPort&0x0f)!=0x0f)
  {
    cord_h=KeyPort&0x0f;  //Ū�J�C�u��
    KeyPort=cord_h|0xf0;  //��X��e�C�u��
    cord_l=KeyPort&0xf0;  //Ū�J��u��

    while((KeyPort&0xf0)!=0xf0);//�����P�}�ÿ�X

    return(cord_h+cord_l);//��L�̫�զX�X��
   }
  }return(0xff);     //��^�ӭ�
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