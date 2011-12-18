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
  RX_PW_P0           = 0x11
  RX_PW_P1           = 0x12
  RX_PW_P5           = 0x16
  FEATURE            = 0x1D
  
  # Reg sub addresses
  PWR_UP             = 0x02
  PRIM_RX            = 0x01
  
  def __init__(self):
    self._nrf = None
    self._currentState = NrfStates.UNDEFINED
  def getStatus(self):
    data = self._nrf.writeAndRead(chr(self.NOP), 1)
    return data[0]
  
  def getReg(self, address):
    if address > pow(2, 5):
      raise Exception('Invalid Address')
    data = self._nrf.writeAndRead(chr(self.R_REGISTER | address), 2)
    return ord(data[1])
  
  def setReg(self, address, value):
    if address > pow(2, 5):
      raise Exception('Invalid Address')
    data = self._nrf.writeAndRead(chr(self.W_REGISTER | address) + chr(value), 2)
  
  def setState(self, state):
    if self._currentState == NrfStates.UNDEFINED:
      raise Exception('Invalid state change')
    
    if state == NrfStates.POWER_DOWN:
      # you can go from any state to power down via PWR_UP=0
      reg = self.getReg(CONFIG)
      self.setReg(CONFIG, reg & (~PWR_UP))
      self._currentState = NrfStates.POWER_DOWN
    
    elif state == NrfStates.STANDBY1:
      # from power_up we go via PWR_UP=1
      if self._currentState == NrfStates.POWER_DOWN:
        reg = self.getReg(CONFIG)
        self.setReg(CONFIG, reg | PWR_UP)
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
        reg = self.getReg(CONFIG)
        self.setReg(CONFIG, reg | PRIM_RX)
        self._currentState = NrfStates.STANDBY1
        self.setCE(True)
      
      # from anywhere else we cannot go straight to RX_MODE
      else:
        raise Exception('Invalid state change')
    
    elif state == NrfStates.TX_MODE:
      # from standby1 we go via CE=1 and PRIM_RX=0
      if self._currentState == NrfStates.STANDBY1:
        reg = self.getReg(CONFIG)
        self.setReg(CONFIG, reg & (~PRIM_RX))
        self._currentState = NrfStates.STANDBY1
        self.setCE(True)
      
      # from anywhere else we cannot go straight to RX_MODE
      else:
        raise Exception('Invalid state change')
