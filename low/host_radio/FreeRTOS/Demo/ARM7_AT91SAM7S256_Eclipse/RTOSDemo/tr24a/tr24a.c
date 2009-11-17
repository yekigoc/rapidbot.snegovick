#include "tr24a.h"
#include "FreeRTOS.h"
#include "pio/pio.h"
#include "spi/spi.h"

static const Pin pins[] = 
{
  TRRESET_PIO,
  MISO_PIO,
  MOSI_PIO,
  NPCS_PIO,
  SPCK_PIO,
  FIFOFLG_PIO,
  PKTFLG_PIO
};

//------------------------------------------------------------------------------
/// Wait time in ms
//------------------------------------------------------------------------------
void UTIL_Loop(unsigned int loop)
{
    while(loop--);	
}


void UTIL_WaitTimeInMs(unsigned int mck, unsigned int time_ms)
{
  register unsigned int i = 0;
  i = (mck / 1000) * time_ms;
  i = i / 3;
  UTIL_Loop(i);
}

//------------------------------------------------------------------------------
/// Wait time in us
//------------------------------------------------------------------------------
void UTIL_WaitTimeInUs(unsigned int mck, unsigned int time_us)
{
  volatile unsigned int i = 0;
  i = (mck / 1000000) * time_us;
  i = i / 3;
  UTIL_Loop(i);
}

void tr24_init()
{
  PIO_Configure(pins, PIO_LISTSIZE(pins));
    
  //  AIC_ConfigureIT(AT91C_ID_SPI,  AT91C_AIC_PRIOR_LOWEST, ISR_Spi0);
  //  SPI_Configure(&trspi, TR24_NPCS, AT91C_SPI_MSTR | AT91C_SPI_MODFDIS  | AT91C_SPI_PCS);
  //  AIC_EnableIT(AT91C_ID_SPI);
  
  /*  PIO_Clear(RESET_PIN);
      UTIL_WaitTimeInMs(BOARD_MCK, 2000);*/

  //  SPI_Enable(&trspi);

  //  AT91C_BASE_PMC->PMC_PCDR = 1 << AT91C_ID_SPI;
  //  AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_SPI;
  
  //  SPI_ConfigureNPCS(&trspi, TR24_NPCS, SPI_SCBR(TR24_BAUDRATE,
  //  BOARD_MCK));
  AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_SPI;

  AT91C_BASE_SPI->SPI_CR      = 0x02;               //SPI Disable
  AT91C_BASE_SPI->SPI_CR      = 0x81;               //SPI Enable, Software reset
  AT91C_BASE_SPI->SPI_CR      = 0x01;               //SPI Enable,
						    //Software reset
  //l_pSpi->SPI_MR      = 0xE0099;           //Master mode, fixed select, disable decoder, FDIV=1 (NxMCK), PCS=1110, loopback
  //l_pSpi->SPI_MR      = 0xE0019;            //Master mode, fixed select, disable decoder, FDIV=1 (NxMCK), PCS=1110,
  //l_pSpi->SPI_MR      = 0xE0011;            //Master mode, fixed select, disable decoder, FDIV=0 (MCK), PCS=1110
  AT91C_BASE_SPI->SPI_MR      = 0xE0019;//0xC0E0011;// 0xE0019;//0xE0001;//0x90E0011;//0xE0019;
					  ////Master mode, fixed
					  //select, disable decoder,
					  //FDIV=1 (MCK), PCS=1110
  //  AT91C_BASE_SPI->SPI_CR      = 0x02;               //SPI Disable
  //  AT91C_BASE_SPI->SPI_CR      = 0x01;               //SPI Enable
  AT91C_BASE_SPI->SPI_CSR[0]=0x1010402;
  //  AT91C_BASE_SPI->SPI_CSR[0]=0x1010210;
  //  AT91C_BASE_SPI->SPI_CSR[0]  = 0x4A02;             //8bit, CPOL=0, ClockPhase=1, SCLK = 200kHz
  //l_pSpi->SPI_CSR[0]  = 0x4A13;             //9bit, CPOL=1, ClockPhase=1, SCLK = 200kHz
  //l_pSpi->SPI_CSR[0]  = 0x4A12;             //9bit, CPOL=0, ClockPhase=1, SCLK = 200kHz
  //l_pSpi->SPI_CSR[0]  = 0x4A11;             //9bit, CPOL=1, ClockPhase=0, SCLK = 200kHz
  //l_pSpi->SPI_CSR[0]  = 0x01011F11;           //9bit, CPOL=1, ClockPhase=0, SCLK = 48Mhz/32*31 = 48kHz
  // work l_pSpi->SPI_CSR[0]  = 0x01010F11;           //9bit, CPOL=1, ClockPhase=0, SCLK = 48Mhz/32*15 = 96kHz
  //AT91C_BASE_SPI->SPI_CSR[0]  = 0x1013100;//0x109FF00;0x3002;//0xCC3000;//0x109FF00;//0x01010F11;//0x01010C11;           //9bit, CPOL=1,
						      //ClockPhase=0,
						      //SCLK =
						      //48Mhz/32*12 =
						      //125kHz
  //  AT91C_BASE_SPI->SPI_CR      = 0x01;               //SPI Enable
}

void tr24_writebyte(unsigned int byte)
{
  portENTER_CRITICAL();
  unsigned int data = byte;
  //  data = (data & ~0x0100);
  while((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_TXEMPTY) == 0);
  AT91C_BASE_SPI->SPI_TDR = data | SPI_PCS(1);
  while ((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_TDRE) == 0);
  portEXIT_CRITICAL();
}

void tr24_writereg(unsigned short reg, unsigned short h, unsigned short l)
{
  UTIL_WaitTimeInUs(BOARD_MCK, 5);
  portENTER_CRITICAL();
  Pin ss = NPCS_PIO;

#ifdef SS
  PIO_Clear(&ss);
#endif
  
  unsigned short data = reg;
  while((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_TXEMPTY) == 0);
  AT91C_BASE_SPI->SPI_TDR = data | SPI_PCS(1);
  while ((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_TDRE) == 0)

  data = h;
  while((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_TXEMPTY) == 0);
  AT91C_BASE_SPI->SPI_TDR = data | SPI_PCS(1);
  while ((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_TDRE) == 0)

  data = l;
  while((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_TXEMPTY) == 0);
  AT91C_BASE_SPI->SPI_TDR = data | SPI_PCS(1);
  while ((AT91C_BASE_SPI->SPI_SR & AT91C_SPI_TDRE) == 0);
#ifdef SS   
  PIO_Set(&ss);
#endif
  //  tr24_writereg(0x31, 0xFF, 0x8F);

  portEXIT_CRITICAL();
  UTIL_WaitTimeInUs(BOARD_MCK, 5);
}

void tr24_initframer()
{
  UTIL_WaitTimeInMs(BOARD_MCK, 1);
  tr24_writereg(0x30, 0x98, 0x00);
  //  tr24_writereg(0x31, 0xFF, 0x8F);
  tr24_writereg(0x31, 0xFF, 0x0F);
  tr24_writereg(0x32, 0x80, 0x28);
  tr24_writereg(0x33, 0x80, 0x56);
  tr24_writereg(0x34, 0x4E, 0xF6);
  tr24_writereg(0x35, 0xF6, 0xF5);
  tr24_writereg(0x36, 0x18, 0x5C);
  tr24_writereg(0x37, 0xD6, 0x51);
  //  tr24_writereg(0x38, 0x44, 0x44);
  tr24_writereg(0x38, 0x44, 0x04);
  tr24_writereg(0x39, 0xE0, 0x00);
  tr24_writereg(0x3A, 0x00, 0x00);
  trspistat.trinited = 1001;
}

void tr24_initrfic()
{
tr24_writereg(0x00, 0x35, 0x4F);
  //  tr24_writereg(0x02, 0x1F, 0x01);
  tr24_writereg(0x02, 0x1F, 0x01);
  //  tr24_writereg(0x04, 0xBC, 0xF0);
  tr24_writereg(0x04, 0x38, 0xD8);
  tr24_writereg(0x05, 0x00, 0xA1);
  tr24_writereg(0x07, 0x12, 0x4C);
  tr24_writereg(0x08, 0x80, 0x08);
  tr24_writereg(0x09, 0x21, 0x01);
  tr24_writereg(0x0A, 0x00, 0x04);
  tr24_writereg(0x0B, 0x40, 0x41);
  //  tr24_writereg(0x00, 0x35, 0x4D);
  tr24_writereg(0x0C, 0x7E, 0x00);
  tr24_writereg(0x0D, 0x00, 0x00);
  tr24_writereg(0x0E, 0x16, 0x9B);
  tr24_writereg(0x0F, 0x80, 0x2F);
  tr24_writereg(0x10, 0xB0, 0xF8);
  tr24_writereg(0x12, 0xE0, 0x00);
  tr24_writereg(0x13, 0xA1, 0x14);
  tr24_writereg(0x14, 0x81, 0x91);
  tr24_writereg(0x15, 0x69, 0x62);
  tr24_writereg(0x16, 0x00, 0x02);
  tr24_writereg(0x17, 0x00, 0x02);
  tr24_writereg(0x18, 0xB1, 0x40);
  tr24_writereg(0x19, 0x78, 0x0F);
  tr24_writereg(0x1A, 0x3F, 0x04);
  tr24_writereg(0x1C, 0x58, 0x00);
  tr24_writereg(0x07, 0x00, 0x00);
  trspistat.trinited = 1002;
}

void tr24_readfifo()
{
  Pin pkt = PKTFLG_PIO;
  Pin fifo = FIFOFLG_PIO;
  Pin ss = NPCS_PIO;

  tr24_writereg(0x52, 0x00, 0x80);
  UTIL_WaitTimeInMs(BOARD_MCK,10);
  tr24_writereg(0x07,0x00,0x90); //INTO RX
  UTIL_WaitTimeInMs(BOARD_MCK,10);

  int e=1;

  trspistat.trinited = 4001;
  while (e)
    {
      if (PIO_Get(&fifo))
	{
	  trspistat.trinited += 10;
	  e=0;
	}
      if (PIO_Get(&pkt))
	{
	  trspistat.trinited += 100;
	  e=0;
	}
    }
  trspistat.trinited += 1;
}

void tr24_writefifo(char * msg, int len)
{
  Pin pkt = PKTFLG_PIO;
  Pin fifo = FIFOFLG_PIO;
  Pin ss = NPCS_PIO;
    
  //  tr24_writereg(0x07, 0x00, 0x80);  //Set DBUS_TX_EN (Enable
				    //transmission)
  //  memcpy(FIFObuf,msg,len ); // copy message to FIFObuf

  UTIL_WaitTimeInUs(BOARD_MCK, 10);

  tr24_writereg(0x52, 0x80, 0x00); //RESET TX FIFO

  UTIL_WaitTimeInMs(BOARD_MCK, 5);

#ifdef SS
  PIO_Clear(&ss);
#endif

  tr24_writebyte(0x50); //RESET TX
  UTIL_WaitTimeInUs(BOARD_MCK, 1);

  tr24_writebyte(0x04);
  UTIL_WaitTimeInUs(BOARD_MCK, 1);
  tr24_writebyte(0x99);
  UTIL_WaitTimeInUs(BOARD_MCK, 1);
  tr24_writebyte(0x99);
  UTIL_WaitTimeInUs(BOARD_MCK, 1);
  tr24_writebyte(0x99);
  UTIL_WaitTimeInUs(BOARD_MCK, 1);
  tr24_writebyte(0x99);


#ifdef SS
  PIO_Set(&ss);
#endif

  tr24_writereg(0x07, 0x01, 0x10);

  trspistat.trinited = 1003;

  unsigned int i=0;

  trspistat.trinited =   trspistat.trinited + PIO_Get(&fifo) + 10*PIO_Get(&pkt);

  while (!PIO_Get(&pkt) && !PIO_Get(&fifo))
    {
      if (i>6000000)
	break;
      i=i+1;
    }
 
  trspistat.trinited = 1100;  

  tr24_writereg(0x52, 0x80, 0x00); //RESET TX FIFO
  UTIL_WaitTimeInMs(BOARD_MCK, 5);
  tr24_writereg(0x07, 0x00, 0x00);
  UTIL_WaitTimeInMs(BOARD_MCK, 5);
}
