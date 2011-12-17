#!/usr/bin/env python
# encoding: utf-8
"""
Created by Sean Nelson on 2009-10-14.
Copyright 2009 Sean Nelson <audiohacked@gmail.com>

This file is part of pyBusPirate.

pyBusPirate is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

pyBusPirate is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with pyBusPirate.  If not, see <http://www.gnu.org/licenses/>.
"""

from .BitBang import *

from struct import pack
from time import sleep

class SPISpeed:
  _30KHZ = 0b000
  _125KHZ = 0b001
  _250KHZ = 0b010
  _1MHZ = 0b011
  _2MHZ = 0b100
  _2_6MHZ = 0b101
  _4MHZ = 0b110
  _8MHZ = 0b111

class SPICfg:
  OUT_TYPE = 0x8
  IDLE = 0x4
  CLK_EDGE = 0x2
  SAMPLE = 0x1

class SPI_OUT_TYPE:
  HIZ = 0
  _3V3 = 1

class SPI(BBIO):
  bulk_read = None

  def __init__(self, port, speed):
    BBIO.__init__(self, port, speed)
  
  def setCS(self, state):
    if state:
      self.port.write('\03')
    else:
      self.port.write('\02')
    self.timeout(0.1)
    return self.response(1, True)
  
  def setCfg(self, spi_cfg):
    self.port.write(chr(0x80 | spi_cfg))
    self.timeout(0.1)
    return self.response()
  
  def getCfg(self):
    self.port.write('\x90')
    self.timeout(0.1)
    return self.response(1, True)
  
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
    self.port.flushInput()
    self.port.flushOutput()
    self.port.write(msg)
    self.port.flush()
    sleep(0.1)
    if self.port.inWaiting() != 0:
      raise IOError('Invalid read/write lenghts')
    self.port.write(data)
    data = self.port.read(readLen+1)
    #print len(data)
    print 'DEBUG: ' + str(map(hex, map(ord, data)))
    #print type(data)
    if ord(data[0]) != 0x01:
      raise IOError('Unknown IO error')
    return data[1:]
