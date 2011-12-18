from Interfaces import *

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

class BusPirateBase(object):
  def __init__(self, port):
    self._currentPins = 0
    self._spi = None
    
    self.connect(port)
  
  """ Connect to the BP """
  def connect(self, port):
    # Open serial port
    try:
      self._port = serial.Serial(port, 115200, timeout=1)
    except SerialException as ex:
      raise IOError('Unable to open serial port')
    
    # Enter binmode
    # and raw SPI mode
    self.enterBB()
    self.enterSPI()
    
    # Configuring SPI peripherals
    self.setPin(BusPiratePins.POWER, True)
    self.setPin(BusPiratePins.CS, True)
    
    # Configure SPI speed
    self.setSpiSpeed(BusPirateSPISpeed._1MHZ)
    
    # Configure SPI configuration
    self.setSpiConfig(BusPirateSPI.OUT_TYPE)
    
    # Waiting 1s 
    sleep(1)
  
  """ Enter BitBang mode """
  def enterBB(self):
    self._port.flushInput()
    self._port.flushOutput()
    self._port.write('\x00'*20)
    self._port.flush()
    
    if self._port.read(5) != 'BBIO1':
      raise IOError('Unable to start BitBang mode')

  def enterSPI(self):
    self._port.flushInput()
    self._port.flushOutput()
    self._port.write('\x01')
    self._port.flush()
    
    if self._port.read(4) != 'SPI1':
      raise IOError('Unable to enter SPI mode')
  
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
  def setSpiConfig(self, settings):
    self._port.flushInput()
    self._port.flushOutput()
    self._port.write(chr(0x80 | settings))
    self._port.flush()
    
    status = self._port.read(1)
    if ord(status) != 0x1:
      raise IOError('Could not set SPI config')

  """ Sets the configuration of the SPI port """
  def setSpiSpeed(self, settings):
    self._port.flushInput()
    self._port.flushOutput()
    self._port.write(chr(0x60 | settings))
    self._port.flush()
    
    status = self._port.read(1)
    if ord(status) != 0x1:
      raise IOError('Could not set SPI Speed')
  
  """ Writes data and then reads a lenght of data with correct CS """
  def writeAndRead(self, data, readLen, withCS=True):
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
    print 'DEBUG: ' + str(map(hex, msg)) + str(map(hex, map(ord, data)))
    self._port.flushInput()
    self._port.flushOutput()
    self._port.write(msg)
    self._port.flush()
    sleep(0.1)
    if self._port.inWaiting() != 0:
      raise IOError('Invalid read/write lenghts')
    self._port.write(data)
    data = self._port.read(readLen+1)
    #print len(data)
    #print 'DEBUG: ' + str(map(hex, map(ord, data)))
    #print type(data)
    if ord(data[0]) != 0x01:
      raise IOError('Unknown IO error')
    return data[1:]

class BusPirateRadio(Radio):
  
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
  
  def __init__(self, port):
    Radio.__init__(self)    
    self._bp = BusPirateBase(port)
    
    # PWR_UP = 1
    config = self.getReg(self.CONFIG)
    self.setReg(self.CONFIG, self.PWR_UP | config)
    sleep(1)
    
    # PRIM_RX = 1
    config = self.getReg(self.CONFIG)
    self.setReg(self.CONFIG, self.PRIM_RX | config)
    # CE = 1
    self._bp.setPin(BusPiratePins.AUX, True)
  
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
  
  def readMsg(self): # TODO
    data = self._bp.writeAndRead(chr(self.W_REGISTER | address) + chr(value), 2)
  
