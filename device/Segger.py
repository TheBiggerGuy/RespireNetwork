#!/usr/bin/env python3
"""
Copyright 2011 Guy Taylor <guy@thebiggerguy.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

NOTE:
  * Pre  7th March 2012: Copyright Guy Taylor
  * Post 7th March 2012: Copyright Guy Taylor with "Apache License, Ver 2.0"
"""

__version__ = "0.1"

from threading import Thread
import subprocess
import os
from time import sleep
import functools # TODO

class GdbServer(Thread):
  
  SWO_REPLY = '+$OK#9a'
  
  TRANSPORT_JTAG = 'JTAG'
  TRANSPORT_SWD  = 'SWD'
  
  PROTOCOL_UART = 'UART'
  PROTOCOL_MAN  = 'Manchester'
  
  GDB_SERVER = ('127.0.0.1', 2331)
  #GDB_SERVER = ('192.168.0.106', 2331)
  
  #SWO_SERVER = ('192.168.0.106', 2332)
  SWO_SERVER = ('127.0.0.1',    2332)
  
  def __init__(self, seggerRoot='/opt/SEGGER', printter=print):
    """
    Launch and maintain a SEGGER GDB server.
    """
    Thread.__init__(self)
    
    self._seggerRoot = seggerRoot
    self._printter = printter
    self._process = None
    #self.start()
  
  def run(self):
    process = subprocess.Popen(
        ['JLinkGDBServer', ' -vd -select usb=0 -if swd -endian little -speed 4000'],
        bufsize=1, shell=False,
        stdout=None, stderr=subprocess.PIPE, stdin=subprocess.PIPE,
        cwd=self._seggerRoot, env={'LD_LIBRARY_PATH': self._seggerRoot}
      )
    self._process = process
    self._printter("\nSEGGER GDB Server started\n")
    try:
      process.wait()
    except Exception:
      pass
    self._printter("\nSEGGER GDB Server ended\n")
  
  def stop(self):
    if self._process == None:
      return
    try:
      self._process.terminate()
    except Exception:
      pass
  
  def get_gdb_host(self):
    return self.GDB_SERVER
  
  def get_swv_host(self):
    return self.SWO_SERVER
  
  @staticmethod
  def _formGdbMsg(msg):
    msg = functools.reduce(lambda x, y: x+y, map(lambda x: x in '#$}' and '}'+chr(ord(x)^0x20) or x, msg))
    chksum =sum(map(ord, msg))%256
    return '${msg}#{chksum:02X}'.format(msg=msg, chksum=chksum)
  
  @classmethod
  def swoStart(cls, transport, protocol, speed):
    if transport != cls.TRANSPORT_SWD:
      raise ValueError('Invalid transport')
    if protocol == cls.PROTOCOL_UART:
      protocol = '0'
    elif protocol == cls.PROTOCOL_MAN:
      protocol = '1'
    else:
      raise ValueError('Invalid protocol')
    msg = 'qSeggerSWO:start:{protocol} {speed:x}+'.format(protocol=protocol, speed=speed)
    return cls._formGdbMsg(msg)
  
  @classmethod
  def swoStop(cls, transport, protocol):
    if transport != cls.TRANSPORT_SWD:
      raise ValueError('Invalid transport')
    return cls._formGdbMsg('qSeggerSWO:stop+')

import unittest

class GdbServerTest(unittest.TestCase):
  '''
  Created on 11 March 2012
  
  @author: Guy Taylor
  '''
  
  def test_init(self):
    '''
    Basic check for functionality.
    '''
    gdb = GdbServer(seggerRoot='/opt/SEGGER/JLink_Linux_V443c')
    sleep(3)
    gdb.stop()
    gdb.join()

if __name__ == "__main__":
    unittest.main()

