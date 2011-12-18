from time import sleep

from RespireBusPirate import BusPirate
from RespireBusPirate import BusPiratePins
from NRF import RespireNRF

class BusPirateNRF(RespireNRF):
  
  def __init__(self, port):
    RespireNRF.__init__(self)    
    self._bp = BusPirate(port)
    
    # PWR_UP = 1
    config = self.getReg(self.CONFIG)
    self.setReg(self.CONFIG, self.PWR_UP | config)
    sleep(1)
    
    # PRIM_RX = 1
    config = self.getReg(self.CONFIG)
    self.setReg(self.CONFIG, self.PRIM_RX | config)
    # CE = 1
    self._bp.setPin(BusPiratePins.AUX, True)
  
