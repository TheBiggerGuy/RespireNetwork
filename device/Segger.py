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

from threading import Thread
import subprocess
import os

class GdbServer(Thread):
  
  SWO_REPLY = '+$OK#9a'
  
  TRANSPORT_JTAG = 'JTAG'
  TRANSPORT_SWD  = 'SWD'
  
  PROTOCOL_UART = 'UART'
  PROTOCOL_MAN  = 'Manchester'
  
  GDB_SERVER = ('127.0.0.1', 2331)
  
  def __init__(self, seggerRoot='/opt/SEGGER', printter=None):
    """
    Launch and maintain a SEGGER GDB server.
    """
    Thread.__init__(self)
    
    if printter == None:
      self._printter = lambda x: x
    else:
      self._printter = printter
    
    self._pHandle = subprocess.Popen(
        ['JLinkGDBServer'],
        bufsize=1, shell=False,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=None,
        cwd=seggerRoot, env={'LD_LIBRARY_PATH': seggerRoot}
      )
    self._out = self._pHandle.stderr

    self.start()
  
  def run(self):
    self._printter("\nSEGGER GDB Server started\n")
    try:
      self._pHandle.poll()
      while self._pHandle != None and self._pHandle.returncode == None:
        #self._printter(self._pHandle.communicate()[1]) #stdout.read(100)
        self._printter(self._out.readline())
        self._pHandle.poll()
    except OSError:
      self._printter("\nServer failed!!\n")
    self._printter("\nSEGGER GDB Server ended\n")
  
  def stop(self):
    if self._pHandle != None and self._pHandle.returncode == None:
      self._pHandle.terminate()
      self._pHandle.wait()
      self._pHandle = None
  
  def get_host(self):
    return self.GDB_SERVER
  
  @staticmethod
  def _formGdbMsg(msg):
    msg = reduce(lambda x, y: x+y, map(lambda x: x in '#$}' and '}'+chr(ord(x)^0x20) or x, msg))
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

if __name__ == '__main__':
  print 'Error: Can not be run as "__main__"'

