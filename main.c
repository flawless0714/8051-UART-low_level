#include <c8051f330.h>
#include <stdio.h>
#include <String.h>

#define SYSCLK 24500000
#define BAUDRATE 115200
#define TM1VAL(baudrate)  ((unsigned int)(256 - (3062500 / baudrate ))) % 1 == 0 ?  ((unsigned int)(256 - (3062500 / baudrate ))) : ((unsigned int)((256 - (3062500 / baudrate ))) - 1)/* baudrate calculator, assuming SYSCLK is divided by 4 */

sbit LED = P1^3;
unsigned long debug = 0;
unsigned int uartDataIndex = 0, uartDataQueuing = 0, uartDataBufIndex = 0, uartRecvDataBufIndex = 0, uartRecvCount = 0;
unsigned char uartData[] = "AT\r\n";
unsigned char sendBuffer[20] = {0}, recvBuffer[20] = {0};
void portInit(void);
void uartInit(void);
void sysclk_Init(void);
int uartPutDataQueue(unsigned char);
void uartSendData(void);

void main(void)
{
    sysclk_Init();
    portInit(); 
    uartInit();
		EA = 1;
    LED = 1;   
		uartPutDataQueue('a');
		uartSendData();
		debug++;
    while(1)
    {
        if (uartRecvCount == 3 ) 
            if (strcmp("a\r\n", recvBuffer) == 0) LED = 0; //AT\r\nOK\r\n
        
    }
}

void portInit(void)
{
    PCA0MD &= ~0x40;  /* disable watchdog */
   	XBR0 = 0x01;
   	XBR1 = 0x42;
    P0MDIN	|= 0x20; /* uart RX */
    P1MDOUT |= 0x08;
}

void uartInit(void)
{
   	SCON0 = 0x10;
    TH1 = TM1VAL(BAUDRATE);
    CKCON &= ~0x0B;
	CKCON |= 0x01;
    TL1 = TH1;
    TMOD &= ~0xF0;
	TMOD |=  0x20;
    TR1 = 1;
		PS0 = 1; // uart0 interrupt priority set to high
   //ES0 = 1; // uart0 interrupt enabled
    //TI0 = 1; //UART TX INT flag (testing if ignored)
}

void sysclk_Init (void)
{
	OSCICN = 0x83;                  // Configure internal oscillator for its maximum frequency
	RSTSRC  = 0x04;                 // Enable missing clock detector
}

void UART0_ISR(void) interrupt 4
{
            debug++;
    if (TI0)
    {
        TI0 = 0;
        if (uartDataQueuing > 0)
        {
            SBUF0 = sendBuffer[uartDataIndex++];
            if (uartDataIndex == sizeof(uartData)) uartDataIndex = 0;
            uartDataQueuing--;
        }

    }
    if (RI0)
    {
        RI0 = 0;
            recvBuffer[uartRecvDataBufIndex] = SBUF0;
            if (uartRecvDataBufIndex == sizeof(recvBuffer)) uartRecvDataBufIndex = 0; 
            uartRecvCount++;
    }
}

int uartPutDataQueue(unsigned char buf)
{
    if (uartDataBufIndex >= sizeof(sendBuffer)) return 1; /* sendBuffer full */
    sendBuffer[uartDataBufIndex] = buf;
    uartDataBufIndex++;
    uartDataQueuing++;
    return 0;
}

void uartSendData(void)
{
    if (uartDataQueuing > 0)
    {
				
			  uartDataQueuing--;
        SBUF0 = sendBuffer[uartDataIndex++];
        if (uartDataIndex == sizeof(uartData)) uartDataIndex = 0;
       TI0 = 1;
    }
}
