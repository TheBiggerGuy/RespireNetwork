#include <stddef.h>
#include "efm32.h"
#include "NRF24.h"
#include "NRF24L01.h"
#include "efm32_usart.h"
#include "efm32_emu.h"
#include "rtc.h"
#include "config.h"
#include "nrf24_config.h"

void NRF_Delay(void)
{
  volatile int i;
  for (i = 0; i < 100000; i++);
}
uint8_t NRF_Status(void)
{  
  NRF_CSN_lo;
  while(!(NRF_USART->STATUS & USART_STATUS_TXBL));
  NRF_USART->TXDATA = 0xFF;
  while (!(NRF_USART->STATUS & USART_STATUS_TXC)) ;
  NRF_CSN_hi;
  return USART_Rx(NRF_USART);
}

uint8_t NRF_ReadByte(uint8_t cmd)
{
  volatile uint8_t blah;
  NRF_CSN_lo;
  while((NRF_USART->STATUS & USART_STATUS_RXDATAV)) {
    blah = NRF_USART->RXDATA;
  }
  while(!(NRF_USART->STATUS & USART_STATUS_TXBL));
  NRF_USART->TXDATA = cmd;
  while(!(NRF_USART->STATUS & USART_STATUS_TXC));
  USART_Rx(NRF_USART);
  NRF_USART->TXDATA = 0x00;
  while (!(NRF_USART->STATUS & USART_STATUS_TXC)) ;
  NRF_CSN_hi;
  return USART_Rx(NRF_USART);
}

uint8_t NRF_ReadRegister(uint8_t reg)
{  
  return NRF_ReadByte(reg | NRF_R_REGISTER);
}

void NRF_WriteRegister(uint8_t reg, uint8_t data)
{
  NRF_SendCommand((reg | NRF_W_REGISTER), data);
}

void NRF_WriteRegisterMulti(uint8_t reg, uint8_t bytes, uint8_t *data)
{
  NRF_SendPayload((NRF_W_REGISTER | reg), bytes, data);
}

void NRF_SendPayload(uint8_t reg, uint8_t bytes, uint8_t *data)
{
  NRF_CSN_lo;
  while(!(NRF_USART->STATUS & USART_STATUS_TXBL));
  NRF_USART->TXDATA = reg;
  for (int i = 0; i < bytes; i++) {
    while(!(NRF_USART->STATUS & USART_STATUS_TXBL));
	  NRF_USART->TXDATA = data[i];
  }
  while(!(NRF_USART->STATUS & USART_STATUS_TXC)) ;
  NRF_CSN_hi;
}

void NRF_SendCommand(uint8_t cmd, uint8_t data)
{
  NRF_CSN_lo;
  while(!(NRF_USART->STATUS & USART_STATUS_TXBL));
  NRF_USART->TXDATA = cmd;
  while(!(NRF_USART->STATUS & USART_STATUS_TXC));
  USART_Rx(NRF_USART);
  NRF_USART->TXDATA = data;
  while (!(NRF_USART->STATUS & USART_STATUS_TXC)) ;
  NRF_CSN_hi;
}  
  
void NRF_SetupTX(void)
{
  uint8_t addr_array[5];
  NRF_CE_lo;
  NRF_WriteRegister(NRF_CONFIG, 0x0C);
  NRF_WriteRegister(NRF_EN_AA, 0x01); // AutoAcknowledge
  NRF_WriteRegister(NRF_EN_RXADDR, 0x01); // Receive Pipe 0 enabled
  NRF_WriteRegister(NRF_SETUP_RETR, 15); // Retransmits
  NRF_WriteRegister(NRF_SETUP_AW, 0x03); // Address Width  (5 bytes)
  NRF_WriteRegister(NRF_RF_SETUP, 0x0F); // RF Setup
  NRF_WriteRegister(NRF_RF_CH, 85); // RF Channel
  NRF_WriteRegister(NRF_RX_PW_P0, 32); // RX Payload Width
  addr_array[0] = 0xE7;
  addr_array[1] = 0xE7;
  addr_array[2] = 0xE7;
  addr_array[3] = 0xE7;
  addr_array[4] = 0x17;
  NRF_WriteRegisterMulti(NRF_TX_ADDR, 5, addr_array);
  NRF_WriteRegisterMulti(NRF_RX_ADDR_P0, 5, addr_array);
  NRF_WriteRegister(NRF_STATUS, 0x7E); // Clear Interrupts
  //NRF_WriteRegister(NRF_CONFIG, 0x0E); // Power Up, Transmitter
  NRF_SendCommand(NRF_FLUSH_TX, 0xFF);
  NRF_WriteRegister(NRF_CONFIG, 0x0E);
} 

void NRF_EnableRX(void)
{
  NRF_WriteRegister(NRF_STATUS, 0x7E);
  NRF_WriteRegister(NRF_CONFIG, 0x0F);
  NRF_SendCommand(0xE2, 0xFF);
  RTC_Delay(1);
  NRF_CE_hi;
  NRF_SendCommand(0xE2, 0xFF);
}

void CE_Lo_TriggerFunc(void)
{
  NRF_CE_lo;
}

void NRF_TransmitPacket(uint8_t bytes, uint8_t *data)
{
  //NRF_WriteRegister(NRF_STATUS, 0x7E); // Clear Interrupts
  //NRF_WriteRegister(NRF_CONFIG, 0x0E); // Power Up, Transmitter
  NRF_WriteRegister(NRF_CONFIG, 0x0E); // Power Up, Transmitter

  //RTC_DelayTicks(1);
  NRF_SendCommand(0xE1, 0xFF);
  NRF_SendPayload(NRF_W_TX_PAYLOAD, bytes, data);
  NRF_CE_hi;
  //RTC_TriggerTicks(1, &CE_Lo_TriggerFunc);
  //EMU_EnterEM2(true);
  //NRF_CE_lo;
}


void NRF_PowerDown(void)
{
  NRF_WriteRegister(NRF_CONFIG, NRF_ReadRegister(NRF_CONFIG) & (~0x02));
}

void NRF_PowerUp(void)
{
  NRF_WriteRegister(NRF_CONFIG, NRF_ReadRegister(NRF_CONFIG) | 0x02);
}

void NRF_Carrier(void)
{
  uint8_t packet[32];
  for (int i = 0; i < 32; i++) {
    packet[i] = 0xFF;
  }
  NRF_WriteRegister(NRF_CONFIG, 0x02);
  NRF_WriteRegister(NRF_EN_AA, 0x00);
  NRF_WriteRegister(NRF_SETUP_RETR, 0x00);
  NRF_WriteRegister(NRF_RF_SETUP, 0x1F); // Enable PLL lock signal (test)
  NRF_WriteRegister(NRF_SETUP_AW, 0x03);
  NRF_WriteRegisterMulti(NRF_TX_ADDR, 5, packet);
  NRF_SendPayload(NRF_W_TX_PAYLOAD, 32, packet);
  NRF_CE_hi;
  NRF_Delay(); //RTC_DelayTicks(1);
  NRF_CE_lo;
  NRF_Delay(); //RTC_Delay(1);
  NRF_CE_hi;
  NRF_CSN_lo;
  while(!(NRF_USART->STATUS & USART_STATUS_TXBL));
  NRF_USART->TXDATA = NRF_REUSE_TX_PL;
  while(!(NRF_USART->STATUS & USART_STATUS_TXC));
  NRF_CSN_hi;
}

