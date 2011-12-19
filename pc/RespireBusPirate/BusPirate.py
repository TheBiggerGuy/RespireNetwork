import serial
from serial.serialutil import SerialException

from time import sleep

class BusPiratePins:
  POWER   = 0x8
  PULLUPS = 0x4
  AUX     = 0x2
  CS      = 0x1

class BusPirateSPI:
  OUT_TYPE = 0x8
  IDLE     = 0x4
  CLK_EDGE = 0x2
  SAMPLE   = 0x1

class BusPirateSPISpeed:
  _30KHZ  = 0x00
  _125KHZ = 0x01
  _250KHZ = 0x02
  _1MHZ   = 0x03
  _2MHZ   = 0x04
  _2_6MHZ = 0x05
  _4MHZ   = 0x06
  _8MHZ   = 0x07

class BusPirate(object):
  
  STATE_TEXT = 1
  STATE_BB   = 2
  STATE_SPI  = 3
  
  def __init__(self, port):
    self._currentPins = 0
    self._spi = None
    self._state = self.STATE_TEXT
    self._spiSettings = 0
    self._spiSpeed = BusPirateSPISpeed._30KHZ
    
    self.connect(port)
  
  """ Connect to the BP """
  def connect(self, port):
    # Open serial port
    try:
      self._port = serial.Serial(port, 115200, timeout=1)
      self._port.write('\x00\x0F')
      self._port.flush()
      sleep(1)
      self._port.flushInput()
    except SerialException as ex:
      raise IOError('Unable to open serial port')
    
    # Enter raw SPI mode
    self.enterSPI()
    
    # Configuring SPI peripherals
    self.setPin(BusPiratePins.POWER, True)
    self.setPin(BusPiratePins.CS, True)
    
    # Configure SPI configuration and speed
    self.setSpiConfig(BusPirateSPI.OUT_TYPE, BusPirateSPISpeed._1MHZ)
    
    # Waiting 1s 
    sleep(1)
  
  """ Enter BitBang mode """
  def enterBB(self):
    if self._state == self.STATE_BB:
      return
    
    self._port.flushInput()
    self._port.flushOutput()
    if self._state == self.STATE_SPI:
      self._port.write('\x00')
    else: # TEXT
      self._port.write('\x00'*20)
    self._port.flush()
    
    if self._port.read(5) != 'BBIO1':
      raise IOError('Unable to start BitBang mode')
    sleep(1)
    self._port.flushInput()
    
    self._state = self.STATE_BB

  def enterSPI(self):
    if self._state == self.STATE_SPI:
      return
    
    if self._state == self.STATE_TEXT:
      self.enterBB()
    
    self._port.flushInput()
    self._port.flushOutput()
    self._port.write('\x01')
    self._port.flush()
    
    if self._port.read(4) != 'SPI1':
      raise IOError('Unable to enter SPI mode')
    
    self.setSpiConfig(self._spiSettings, self._spiSpeed)
    
    self._state = self.STATE_SPI
  
  def enterText(self):
    if self._state == self.STATE_TEXT:
      return
    self._port.write('\x0F')
    self._port.flush()
    
    # As the UART is rest you get rubish on the line
    sleep(1)
    self._port.flushInput()
    
    self._state == self.STATE_TEXT
  
  """ Sets a BusPirate Pins on or off """
  def setPin(self, pin, state):
    if state:
      self._currentPins = self._currentPins | pin
    else:
      self._currentPins = self._currentPins & (~pin)
    self._port.flushInput()
    self._port.flushOutput()
    self._port.write(chr(0x40 | self._currentPins))
    self._port.flush()
    
    status = self._port.read(1)
    if ord(status) != 0x1:
      raise IOError('Could not set pins')
  
  """ Sets the configuration of the SPI port """
  def setSpiConfig(self, settings, speed):
    self._port.flushInput()
    self._port.flushOutput()
    self._port.write(chr(0x80 | settings))
    self._port.write(chr(0x60 | speed))
    self._port.flush()
    
    status = self._port.read(1)
    if ord(status) != 0x1:
      raise IOError('Could not set SPI config')
    
    self._spiSettings = settings
    self._spiSpeed = speed
  
  """ Writes data and then reads a lenght of data with correct CS """
  def writeAndRead(self, data, readLen, withCS=True, withInitialByte=False):
    writeLen = len(data)
    msg = bytearray()
    if withCS:
      msg.append(0x04)
    else:
      msg.append(0x05)
    msg.append((writeLen >> 8) & 0xff)
    msg.append(       writeLen & 0xff)
    msg.append((readLen >> 8)  & 0xff)
    msg.append(       readLen  & 0xff)
    self._port.flushInput()
    self._port.flushOutput()
    self._port.write(msg)
    self._port.flush()
    sleep(0.1)
    if self._port.inWaiting() != 0:
      raise IOError('Invalid read/write lenghts')
    self._port.write(data)
    data = self._port.read(readLen+1)
    if ord(data[0]) != 0x01:
      raise IOError('Unknown IO error')
    if withInitialByte:
      return data
    else:
      return data[1:]
  
  def __del__(self):
    if self._port != None:
      self._port.write('\x0f')
      self._port.flush()
      self._port.close()
      self._port = None

if __name__ == '__main__':
  raise RuntimeError('Unable to run module as main')

