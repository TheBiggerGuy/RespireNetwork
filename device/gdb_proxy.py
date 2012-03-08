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

"""

http://davis.lbl.gov/Manuals/GDB/gdb_31.html
http://www.embecosm.com/appnotes/ean4/embecosm-howto-rsp-server-ean4-issue-2.html#sec_exchange_target_remote

http://infocenter.arm.com/help/topic/com.arm.doc.ddi0314h/DDI0314H_coresight_components_trm.pdf

http://www.embecosm.com/appnotes/ean4/embecosm-howto-rsp-server-ean4-issue-2.html
"""

import socket
import socketserver
import threading
from time import sleep
import traceback
import sys

from Segger import GdbServer
from Swd import SwoServer

HOST = ("localhost", 9999)

SWO_PROTO = GdbServer.PROTOCOL_UART
SWO_SPEED = 875000 # 19200

#SWO_SERVER = ('192.168.0.107', 2332)
SWO_SERVER = ('127.0.0.1',    2332)

class GdbProxyHandler(socketserver.BaseRequestHandler):
  """
  The RequestHandler class for our server.

  It is instantiated once per connection to the server, and must
  override the handle() method to implement communication to the
  client.
  """
  
  currentHandle = None
  
  def __init__(self, request, client_address, server, enable_swo=False):
    self._input = None
    self._output = None
    self._swo = None
    self._run = True
    self._segger = None
    self._enable_swo = enable_swo
   
    # make sure there is only one connection
    if self.currentHandle != None:
      self.print_gdb('New connection killing old connection ...')
      self.currentHandle.close()
      self.currentHandle.join(3) # 3second timeout
      self.print_gdb(' OK\n')
    self.currentHandle = self
    
    self._plock = threading.Lock()
    self.print_gdb('New connection')
    
    socketserver.BaseRequestHandler.__init__(self, request, client_address, server)
  
  def print_gdb(self, to_print):
    self._print_all(to_print, 0)
  
  def print_segger(self, to_print):
    self._print_all(to_print, 40)
  
  def print_swo(self, to_print):
    self._print_all(to_print, 80)
  
  def _print_all(self, to_print, padding):
    self._plock.acquire()
    for line in to_print.split('\n'):
      print( (' '*padding) + line )
    self._plock.release()
  
  def setup(self):
    """
              input            output
    Program ---------> pyGDB -----------> SEGGER GDB -------> Device
    """
    # get segger gdb
    self._segger = GdbServer('/opt/SEGGER/JLink_Linux_V443c', printter=self.print_segger)
    sleep(2)
    
    # get our gdb server
    self._output = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    self._output.connect(self._segger.get_host())
    self._output.setblocking(0)
    
    # SWO?
    if self._enable_swo:
      # send the magic
      self._output.setblocking(1)
      self._output.sendall(self._segger.swoStart(GdbServer.TRANSPORT_SWD, SWO_PROTO, SWO_SPEED))
      self._output.recv(len(self._segger.SWO_REPLY))
      self.print_gdb('Sent maigc and burnt reply')
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
    while self._run and self._segger.is_alive():
      try:
        buf = self._input.recv(4096)
        inbuf += buf
        last5inbuf += buf
        last5inbuf = last5inbuf[-5:]
        while len(inbuf) > 0:
          if len(inbuf) > 40:
            self.print_gdb('>: ' + inbuf[:36] + ' ...')
            inbuf = inbuf[36:]
          else:
            self.print_gdb('>: ' + inbuf)
            inbuf = ''
      except Exception as e:
        if e.args[0] != 11:
          raise e
      try:
        outbuf += self._output.recv(4096)
        while len(outbuf) > 0:
          if len(outbuf) > 40:
            self.print_gdb('<: ' + outbuf[:36] + ' ...')
            outbuf = outbuf[36:]
          else:
            self.print_gdb('<: ' + outbuf)
            outbuf = ''
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
        self.print_gdb('found k')
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
    if self._segger != None:
      self._segger.stop()
      self._segger = None
    print('Connection closed')
    print('Waiting for other end ...')
    sleep(1)
    print('.. OK')
  
  #def __del__(self):
  #  self.finish()


class GdbProxyServer(socketserver.ThreadingTCPServer):
  
  def __init__(self, server_address, RequestHandlerClass, bind_and_activate=True, enable_swo=True):
    self._enable_swo = enable_swo
    socketserver.TCPServer.__init__(self, server_address, RequestHandlerClass, bind_and_activate)
  
  def verify_request(self, request, client_address):
    return request.family == socket.AF_INET and request.type == socket.SOCK_STREAM

  def finish_request(self, request, client_address):
    """Finish one request by instantiating RequestHandlerClass."""
    self.RequestHandlerClass(request, client_address, self, enable_swo=self._enable_swo)

if __name__ == "__main__":
  
  # Create the server, binding to localhost on port 9999
  print("Waiting for port ") 
  while True:
    try:
      server = GdbProxyServer(HOST, GdbProxyHandler, enable_swo=False)
      break
    except KeyboardInterrupt:
      print('Shutting down (external) ...')
      sys.exit(1)
    except Exception:
      print('.')
      sleep(1)
  print('OK')
  
  # Activate the server; this will keep running until you
  # interrupt the program with Ctrl-C
  try:
    print('Started and waiting for connections ...')
    server.serve_forever()
    print('Shutting down (internal) ...')
  except KeyboardInterrupt:
    print('Shutting down (external) ...')
    
  server.shutdown()

