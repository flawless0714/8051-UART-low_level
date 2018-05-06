#include <c8051f330.h>
#include <stdio.h>

#define SYSCLK 24500000
#define BAUDRATE 115200
#define TM1VAL(baudrate)  (256 - (3062500 / baudrate )) /* assuming SYSCLK is divided by 4 */

sbit LED = P1^3;
unsigned int uartDataIndex, uartDataQueuing, uartDataBufIndex = 0;
unsigned char uartData[] = "AT\r\n";
unsigned char buffer[20] = {0};
void portInit(void);
void uartInit(void);
void SYSCLK_Init(void);
bool uartPutDataQueue(unsigned char);
void uartSendData(void);

void main()
{
    uartDataIndex = 0;
    uartDataBufIndex = 0;
    uartDataQueuing = 0;
    portInit(); 
    uartInit();
    LED = 0;   
    while(1)
    {

    }
}

void portInit()
{
   	XBR0 = 0x01;
   	XBR1 = 0x42;
    P0MDIN	|= 0x20; /* uart RX */
    P1MDOUT |= 0x08;
}

void uartInit()
{
   	SCON0 = 0x10;
    TH1 = TM1VAL(BAUDRATE);
    CKCON &= ~0x0B;
	CKCON |= 0x01;
    TL1 = TH1;
    TMOD &= ~0xF0;
	TMOD |=  0x20;
    TR1 = 1;
    ES0 = 1; // uart0 interrupt enabled
    //TI0 = 1; UART TX INT flag (testing if ignored)
}

void SYSCLK_Init (void)
{
	OSCICN = 0x83;                  // Configure internal oscillator for its maximum frequency
	RSTSRC  = 0x04;                 // Enable missing clock detector
}

void uart0_ISR(void) interrupt 4
{
    if (TI0)
    {
        TI0 = 0;
        if (uartDataIndex < sizeof(uartData))
        {
            SBUF0 = uartData[uartDataIndex++];
            if (uartDataIndex == sizeof(uartData)) uartDataIndex = 0;
            uartDataQueuing--;
        }
    }
    if (RI0)
    {
        RI0 = 0;

    }
}

bool uartPutDataQueue(unsigned char buf)
{
    if (uartDataBufIndex >= sizeof(buffer)) return false; /* buffer full */
    buffer[uartDataBufIndex] = buf;
    uartDataBufIndex++;
    uartDataQueuing++;
}

void uartSendData(void)
{
    if (uartDataQueuing > 0)
    {
        SBUF0 = buffer[uartDataIndex++];
        if (uartDataIndex == sizeof(uartData)) uartDataIndex = 0;
        uartDataQueuing--;
    }
}
