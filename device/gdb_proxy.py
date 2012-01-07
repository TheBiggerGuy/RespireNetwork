"""

http://davis.lbl.gov/Manuals/GDB/gdb_31.html
http://www.embecosm.com/appnotes/ean4/embecosm-howto-rsp-server-ean4-issue-2.html#sec_exchange_target_remote

http://infocenter.arm.com/help/topic/com.arm.doc.ddi0314h/DDI0314H_coresight_components_trm.pdf

"""

import socket
import SocketServer
import threading
from time import sleep
import traceback
import sys

magic = '$qSeggerSWO:start:0 f4240+#14'

class SwoThread(threading.Thread):
  
  OVERFLOW = 0x70
  
  SYNC_NONE = 0x01
  SYNC_80   = 0x02
  SYNC_OK   = 0x03
  
  def __init__(self):
    self._run = True
  
    self._io = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    self._io.connect(('192.168.1.14', 2332))
    self._io.setblocking(0)
    
    threading.Thread.__init__(self)
    self.start()
  
  def run(self):
    inSync = self.SYNC_OK
    buf = ''
    
    print ' '*40, 'SWO: started'
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
            print ' '*40, 'got sync'
          else:
            inSync = self.SYNC_NONE
        elif inSync == self.SYNC_OK:
          if buf[0] == self.OVERFLOW:
            # we have overfloe
            buf = buf[1:]
            print ' '*40, '>Overflow'
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
            print ' '*40, '>Timestamp (' + str(map(ord, msg)) + ')'
            if len(msg) == 1:
              msg = ord(msg[0])
              if msg == 0:
                print ' '*44, 'Reserved timestamp control'
              elif msg == 7:
                print ' '*44, 'Overflow ITM'
              else:
                print ' '*44, 'Timestamp emitted synchronous to ITM data'
                
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
            print ' '*40, '>Reserved (' + str(map(ord, msg)) + ')'
          else:
            # we have SWIT
            if len(buf) < 4:
              continue
            print ord(buf[0])
            plen = ord(buf[0]) & 0x03
            buf = buf[1:]
            msg = buf[:plen]
            buf = buf[plen:]
            print ' '*40, '>SWIT'
            print ' '*44, 'len: ' + str(plen) + ' (' + str(map(ord, msg)) + ')'
      
      except Exception as e:
        if e.args[0] != 11:
          print '-'*60
          traceback.print_exc(file=sys.stdout)
          print '-'*60
          raise e
  
  def stop(self):
    self._run = False

class MyTCPHandler(SocketServer.BaseRequestHandler):
  """
  The RequestHandler class for our server.

  It is instantiated once per connection to the server, and must
  override the handle() method to implement communication to the
  client.
  """
  
  currentHandle = None
  
  def __init__(self, request, client_address, server):
    self._input = None
    self._output = None
    self._swo = None
    self._run = True
    
    if request.family != socket.AF_INET or request.type != socket.SOCK_STREAM:
      raise socket.error('Invalid protocol')
    
    print 'New connection'
    
    SocketServer.BaseRequestHandler.__init__(self, request, client_address, server)
  
  def setup(self):
    # make sure there is only one connection
    if self.currentHandle != None:
      self.currentHandle.close()
    self.currentHandle = self
    
    # get the gdb server
    self._output = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    self._output.connect(('192.168.1.14', 2331))
    self._output.setblocking(1)
    self._output.sendall(magic)
    self._output.recv(7)
    self._output.setblocking(0)
    print 'Sent maigc and burnt reply'
    
    self._swo = SwoThread()
    
    # clean up the input socket
    self._input = self.request
    self._input.setblocking(0)
  
  def handle(self):
    last5inbuf = ' '*5
    inbuf = ''
    outbuf = ''
    readNoMore = False
    while self._run:
      try:
        buf = self._input.recv(4096)
        inbuf += buf
        last5inbuf += buf
        last5inbuf = last5inbuf[-5:]
        if len(inbuf) > 0:
          if len(inbuf) > 30:
            print '>: ' + inbuf[:26] + ' ...'
          else:
            print '>: ' + inbuf
      except Exception as e:
        if e.args[0] != 11:
          raise e
      try:
        outbuf += self._output.recv(4096)
        if len(outbuf) > 0:
          if len(outbuf) > 30:
            print '>: ' + outbuf[:26] + ' ...'
          else:
            print '>: ' + outbuf
      except Exception as e:
        if e.args[0] != 11:
          raise e
      try:
        sent = self._input.send(outbuf)
        outbuf = outbuf[sent:]
      except Exception as e:
        pass
      try:
        sent = self._output.send(inbuf)
        inbuf = inbuf[sent:]
      except Exception as e:
        pass
      
      if last5inbuf == '$k#6b':
        print 'found k'
        sleep(1)
        try:
          outbuf += self._output.recv(4096)
        except Exception as e:
          pass
        try:
          sent = self._input.send(outbuf)
        except Exception as e:
          pass
        self.close()
      
      # self.request is the TCP socket connected to the client
      #self.data = self.request.recv(1024).strip()
      #print "{} wrote:".format(self.client_address[0])
      #print self.data
      # just send back the same data, but upper-cased
      #self.request.send(self.data.upper())
  
  def close(self):
    self._run = False
  
  def finish(self):    
    if self._input != None:
      self._input.close()
    if self._output != None:
      self._output.close()
    if self._swo != None:
      self._swo.stop()
    if self.currentHandle != None:
      self.currentHandle = None
    print 'Connection closed'

if __name__ == "__main__":
  HOST, PORT = "localhost", 9999
  
  # Create the server, binding to localhost on port 9999
  server = SocketServer.TCPServer((HOST, PORT), MyTCPHandler)
  
  # Activate the server; this will keep running until you
  # interrupt the program with Ctrl-C
  try:
    print 'Started and waiting for connections ...'
    server.serve_forever()
    print 'Shutting down (internal) ...'
  except KeyboardInterrupt:
    print 'Shutting down (external) ...'
  server.shutdown()

