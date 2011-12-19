from time import sleep
import logging

from RespireBusPirate import BusPirate
from RespireBusPirate import BusPiratePins
from NRF import RespireNRF, NrfStates

def int2bin(n, count=8):
  """returns the binary of integer n, using count number of digits"""
  return "".join([str((n >> y) & 1) for y in range(count-1, -1, -1)])

"""
Implemnts the NRF radio over the BusPirate
"""
class BusPirateNRF(RespireNRF):
  
  def __init__(self, port):
    RespireNRF.__init__(self)
    self._nrf = BusPirate(port)
    self._currentState = NrfStates.POWER_DOWN
    
    self.__logger = logging.getLogger('BusPirateNRF')
  
  def setCE(self, state):
    self._nrf.setPin(BusPiratePins.AUX, state)
  
  def getIRQ(self):
    # The BusPirate cannot read the IRQ pin !!
    # so we read the status and config reg and return are calc of it
    status = self.getReg(self.STATUS)
    config = self.getReg(self.CONFIG)
    
    rx_dr  = (status & self.RX_DR ) != 0
    tx_ds  = (status & self.TX_DS ) != 0
    max_rt = (status & self.MAX_RT) != 0
    
    mask_rx_dr  = (config & self.MASK_RX_DR ) != 0
    mask_tx_ds  = (config & self.MASK_TX_DS ) != 0
    mask_max_rt = (config & self.MASK_MAX_RT) != 0
    
    irq = (mask_rx_dr and rx_dr) or (mask_tx_ds and tx_ds) or (mask_max_rt and max_rt)
        
    self.__logger.info('S  : ' + int2bin(status))
    self.__logger.info('C  : ' + int2bin(config))
    self.__logger.info('IRQ: ' + str(irq))
    
    return irq

if __name__ == '__main__':
  raise RuntimeError('Unable to run module as main')

