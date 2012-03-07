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

"""

http://davis.lbl.gov/Manuals/GDB/gdb_31.html
http://www.embecosm.com/appnotes/ean4/embecosm-howto-rsp-server-ean4-issue-2.html#sec_exchange_target_remote

http://infocenter.arm.com/help/topic/com.arm.doc.ddi0314h/DDI0314H_coresight_components_trm.pdf

http://www.embecosm.com/appnotes/ean4/embecosm-howto-rsp-server-ean4-issue-2.html
"""

import socket
import SocketServer
import threading
from time import sleep
import traceback
import sys

from Segger import GdbServer
from Swd import SwoServer

ENABLE_SWO = True

SWO_PROTO = GdbServer.PROTOCOL_UART
SWO_SPEED = 875000 # 19200

#GDB_SERVER = ('192.168.0.107', 2331)
GDB_SERVER = ('127.0.0.1',    2331)

#SWO_SERVER = ('192.168.0.107', 2332)
SWO_SERVER = ('127.0.0.1',    2332)

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
  
  def print_segger(self, to_print):
    for line in to_print.split('\n'):
      print ' '*40 + line
  
  def print_swo(self, to_print):
    for line in to_print.split('\n'):
      print ' '*80 + line
  
  def setup(self):
    # make sure there is only one connection
    if self.currentHandle != None:
      self.currentHandle.close()
      self.currentHandle.join(3) # 3second timeout
    self.currentHandle = self
    
    # get our gdb server
    self._output = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    self._output.connect(GDB_SERVER)
    self._output.setblocking(0)
    
    # get segger gdb
    segger = GdbServer('/opt/SEGGER/JLink_Linux_V443c', printter=self.print_segger)
    
    # SWO?
    if ENABLE_SWO:
      # send the magic
      self._output.setblocking(1)
      print segger.swoStart(GdbServer.TRANSPORT_SWD, SWO_PROTO, SWO_SPEED)
      self._output.sendall(segger.swoStart(GdbServer.TRANSPORT_SWD, SWO_PROTO, SWO_SPEED))
      self._output.recv(len(segger.SWO_REPLY))
      print 'Sent maigc and burnt reply'
      # start SWO reciver
      self._swo = SwoServer(SWO_SERVER, printter=self.print_swo)
      # reset socket
      self._output.setblocking(0)
    
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
            print '<: ' + outbuf[:26] + ' ...'
          else:
            print '<: ' + outbuf
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
    print 'Waiting for other end ...',
    sleep(1)
    print '.. OK'
  
  def __del__(self):
    self.finish()

if __name__ == "__main__":
  HOST, PORT = "localhost", 9999
  
  GdbServer('/opt/SEGGER/JLink_Linux_V443c')
  
  # Create the server, binding to localhost on port 9999
  print "Waiting for port ", 
  while True:
    try:
      server = SocketServer.TCPServer((HOST, PORT), MyTCPHandler)
      break
    except KeyboardInterrupt:
      break
    except Exception:
      print '.',
      sleep(1)
  print 'OK'
  
  # Start a thread with the server -- that thread will then start one
  # more thread for each request
  server_thread = threading.Thread(target=server.serve_forever)
  # Exit the server thread when the main thread terminates
  server_thread.daemon = True
  
  # Activate the server; this will keep running until you
  # interrupt the program with Ctrl-C
  try:
    print 'Started and waiting for connections ...'
    #server.serve_forever()
    server_thread.start()
    while server_thread.is_alive():
      pass
    print 'Shutting down (internal) ...'
  except KeyboardInterrupt:
    print 'Shutting down (external) ...'
  
  server.shutdown()

