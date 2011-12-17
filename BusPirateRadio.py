from Interfaces import *

from serial.serialutil import SerialException
from pyBusPirateLite.SPI import *

import time

class BusPirateRadio(Radio):
  def __init__(self, port):
    self._spi = None
    self.connect(port)
  
  """ Connect to the BP """
  def connect(self, port):
    
    # Open serial port
    try:
      self._spi = SPI(port, 115200)
    except SerialException as ex:
      raise IOError('Unable to open serial port')
    
    # Enter binmode
    # and raw SPI mode
    self._spi.BBmode()
    self._spi.enter_SPI()
    
    # Configuring SPI peripherals
    if self._spi.cfg_pins(PinCfg.POWER | PinCfg.CS) != 1:
      raise IOError('Unable to start SPI')
    
    # Configure SPI speed
    if self._spi.set_speed(SPISpeed._1MHZ) != 1:
      raise IOError('Unable to start SPI')
    
    # Configure SPI configuration
    #if spi.cfg_spi(SPICfg.OUT_TYPE | ~SPICfg.IDLE | ~SPICfg.CLK_EDGE | ~SPICfg.SAMPLE ):
    self._spi.setCfg(SPICfg.OUT_TYPE)
    
    # Waiting 1s 
    time.sleep(1)
  
  def getStatus(self, old=False):
    if self._spi == None:
      raise IOError('Device not connected')
    
    if old:
      self._spi.setCS(False)
      data = self._spi.bulk_trans(1, [0xFF])
      self._spi.setCS(True)
    else:
      data = self._spi.writeAndRead('\xFF', 1)
    
    return data[0]
  
  def getConfig(self, old=False):
    if self._spi == None:
      raise IOError('Device not connected')
    
    if old:
      self._spi.setCS(False)
      data = self._spi.bulk_trans(1, [0x00])
      self._spi.setCS(True)
      return data
    else:
      data = self._spi.writeAndRead('\x00', 2)
      return data[0]
  
  def getReg(self, address, old=False):
    if address > 15:
      raise Exception('bad')
    
    if old:
      self._spi.setCS(False)
      data = self._spi.bulk_trans(1, [address])
      self._spi.setCS(True)
      return data
    else:
      data = self._spi.writeAndRead(chr(address) + '\00', 2)
      return data[1]
    
