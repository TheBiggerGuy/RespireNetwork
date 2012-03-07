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
"""

import threading
import socket

class SwoServer(threading.Thread):
  
  OVERFLOW = 0x70
  
  SYNC_NONE = 0x01
  SYNC_80   = 0x02
  SYNC_OK   = 0x03
  
  def __init__(self, server, printter=None):
    self._run = True
    
    if printter == None:
      self._printter = lambda x: x
    else:
      self._printter = printter
    
    self._io = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    self._io.connect(server)
    self._io.setblocking(0)
    
    threading.Thread.__init__(self)
    self.start()
  
  def run(self):
    inSync = self.SYNC_OK
    buf = ''
    
    self._printter('SWO: started\n')
    f  = open('log.swo.log', 'w')
    while self._run:
      try:
        buf += self._io.recv(4096)
        if len(buf) < 1:
          continue
        if inSync == self.SYNC_NONE:
          while len(buf) > 0:
            if buf[0] == 0x80:
              inSync = self.SYNC_80
              continue
            buf = buf[1:]
        elif inSync == self.SYNC_80:
          if len(buf) < 5:
            continue
          if buf[0] == 0x00 and \
              buf[1] == 0x00 and \
              buf[2] == 0x00 and \
              buf[3] == 0x00:
            inSync = self.SYNC_OK
            self._printter('got sync\n')
          else:
            inSync = self.SYNC_NONE
        elif inSync == self.SYNC_OK:
          if buf[0] == self.OVERFLOW:
            # we have overfloe
            buf = buf[1:]
            self._printter('>Overflow\n')
          elif ord(buf[0]) & 0x07 == 0x00:
            # we have a timestamp
            if len(buf) < 4:
              continue
            msg = ''
            while True:
              b = buf[0]
              buf = buf[1:]
              msg += b
              if ord(b) & 0x80 == 0:
                break
            self._printter('>Timestamp (' + str(map(ord, msg)) + ')\n')
            if len(msg) == 1:
              msg = ord(msg[0])
              if msg == 0:
                self._printter('Reserved timestamp control\n')
              elif msg == 7:
                self._printter('Overflow ITM\n')
              else:
                self._printter('Timestamp emitted synchronous to ITM data\n')
                
          elif ord(buf[0]) & 0x03 == 0x00:
            # we have 'Reserved'
            if len(buf) < 4:
              continue
            msg = ''
            while True:
              b = buf[0]
              buf = buf[1:]
              msg += b
              if ord(b) & 0x80 == 0:
                break
            self._printter('>Reserved (' + str(map(ord, msg)) + ')\n')
          else:
            # we have SWIT
            if len(buf) < 4:
              continue
            plen = ord(buf[0]) & 0x03
            buf = buf[1:]
            msg = buf[:plen]
            buf = buf[plen:]
            self._printter('>SWIT\n')
            self._printter('len: ' + str(plen) + ' (' + str(map(ord, msg)) + ')\n')
            f.write(msg)
      
      except Exception as e:
        if e.args[0] != 11: # TODO
          print '-'*60
          traceback.print_exc(file=sys.stdout)
          print '-'*60
          raise e
  
  def stop(self):
    self._run = False
  
  def __del__(self):
    if self._io != None:
      self._io.close()

if __name__ == '__main__':
  print 'ERROR: Can only run as module'

