from RespireNetwork import Radio
from RespireNetwork import RadioMessage

class NrfStates(object):
  POWER_DOWN = 0x00
  STANDBY1   = 0x01
  STANDBY2   = 0x02 # never used
  RX_MODE    = 0x03
  TX_MODE    = 0x04
  UNDEFINED  = 0xFF

"""
Interface for the nRF24L01+ radio
"""
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
  STATUS             = 0x07
  RX_PW_P0           = 0x11
  RX_PW_P1           = 0x12
  RX_PW_P5           = 0x16
  FEATURE            = 0x1D
  
  # Reg sub addresses
  # CONFIG
  # res              = pow(2, 7)
  MASK_RX_DR         = pow(2, 6)
  MASK_TX_DS         = pow(2, 5)
  MASK_MAX_RT        = pow(2, 4)
  EN_CRC             = pow(2, 3)
  CRCO               = pow(2, 2)
  PWR_UP             = pow(2, 1)
  PRIM_RX            = pow(2, 0)
  # STATUS
  # res              = pow(2, 7)
  RX_DR              = pow(2, 6)
  TX_DS              = pow(2, 5)
  MAX_RT             = pow(2, 4)
  RX_P_NO            = pow(2, 1) + pow(2, 2) + pow(2, 3)
  TX_FULL            = pow(2, 0)
  
  def __init__(self):
    self._nrf = None
    self._currentState = NrfStates.UNDEFINED
  
  def getReg(self, address):
    if address > pow(2, 5):
      raise Exception('Invalid Address')
    data = self._nrf.writeAndRead(chr(self.R_REGISTER | address), 1)
    return ord(data[0])
  
  def setReg(self, address, value):
    if address > pow(2, 5):
      raise Exception('Invalid Address')
    data = self._nrf.writeAndRead(chr(self.W_REGISTER | address) + chr(value), 0)
  
  def setState(self, state):
    if self._currentState == NrfStates.UNDEFINED:
      raise Exception('Invalid state change')
    
    if state == NrfStates.POWER_DOWN:
      # you can go from any state to power down via PWR_UP=0
      reg = self.getReg(self.CONFIG)
      self.setReg(self.CONFIG, reg & (~self.PWR_UP))
      self._currentState = NrfStates.POWER_DOWN
    
    elif state == NrfStates.STANDBY1:
      # from power_up we go via PWR_UP=1
      if self._currentState == NrfStates.POWER_DOWN:
        reg = self.getReg(self.CONFIG)
        self.setReg(self.CONFIG, reg | self.PWR_UP)
        self._currentState = NrfStates.STANDBY1
      
      # from rx or tx we can got via CE=0
      elif self._currentState == NrfStates.RX_MODE or self._currentState == NrfStates.TX_MODE:
        self.setCE(False)
        self._currentState = NrfStates.STANDBY1
      
    elif state == NrfStates.STANDBY2:
        raise Exception('Invalid state change')
    
    elif state == NrfStates.RX_MODE:
      # from standby1 we go via CE=1 and PRIM_RX=1
      if self._currentState == NrfStates.STANDBY1:
        reg = self.getReg(self.CONFIG)
        self.setReg(self.CONFIG, reg | self.PRIM_RX)
        self._currentState = NrfStates.STANDBY1
        self.setCE(True)
      
      # from anywhere else we cannot go straight to RX_MODE
      else:
        raise Exception('Invalid state change')
    
    elif state == NrfStates.TX_MODE:
      # from standby1 we go via CE=1 and PRIM_RX=0
      if self._currentState == NrfStates.STANDBY1:
        reg = self.getReg(self.CONFIG)
        self.setReg(self.CONFIG, reg & (~self.PRIM_RX))
        self._currentState = NrfStates.STANDBY1
        self.setCE(True)
      
      # from anywhere else we cannot go straight to RX_MODE
      else:
        raise Exception('Invalid state change')
