from Interfaces import *

class RespireNRF(Radio):
  
  # SPI commands
  R_REGISTER         = 0x00
  W_REGISTER         = 0x20
  R_RX_PAYLOAD       = 0x61
  W_TX_PAYLOAD       = 0xA0
  FLUSH_TX           = 0xE1
  FLUSH_RX           = 0xE2
  REUSE_TX_PL        = 0xE3
  R_RX_PL_WID        = 0x60
  W_ACK_PAYLOAD      = 0xA8
  W_TX_PAYLOAD_NOACK = 0xB0
  NOP                = 0xFF
  
  # Reg addresses
  CONFIG             = 0x00
  RX_PW_P0           = 0x11
  RX_PW_P1           = 0x12
  RX_PW_P5           = 0x16
  FEATURE            = 0x1D
  
  # Reg sub addresses
  PWR_UP             = 0x02
  PRIM_RX            = 0x01
  
  def __init__(self):
    pass
  
  def getStatus(self):
    data = self._bp.writeAndRead(chr(self.NOP), 1)
    return data[0]
  
  def getReg(self, address):
    if address > pow(2, 5):
      raise Exception('Invalid Address')
    data = self._bp.writeAndRead(chr(self.R_REGISTER | address), 2)
    return ord(data[1])
  
  def setReg(self, address, value):
    if address > pow(2, 5):
      raise Exception('Invalid Address')
    data = self._bp.writeAndRead(chr(self.W_REGISTER | address) + chr(value), 2)
