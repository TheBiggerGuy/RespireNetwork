from time import sleep

from RespireBusPirate import BusPirate
from RespireBusPirate import BusPiratePins
from NRF import RespireNRF

"""
Implemnts the NRF radio over the BusPirate
"""
class BusPirateNRF(RespireNRF):
  
  def __init__(self, port):
    RespireNRF.__init__(self)    
    self._nrf = BusPirate(port)
    
    # PWR_UP = 1
    config = self.getReg(self.CONFIG)
    self.setReg(self.CONFIG, self.PWR_UP | config)
    sleep(1)
    
    # PRIM_RX = 1
    config = self.getReg(self.CONFIG)
    self.setReg(self.CONFIG, self.PRIM_RX | config)
    # CE = 1
    self.setCE(True)
  
  def setCE(self, state):
    self._nrf.setPin(BusPiratePins.AUX, state)
  
  def getIRQ(self):
    return self._nrf.getADC() > 1.0

if __name__ == '__main__':
  raise RuntimeError('Unable to run module as main')

