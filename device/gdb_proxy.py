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

"""
mon speed 4000
mon interface SWD
mon flash device = EFM32G890F128
mon flash download = 1
mon flash breakpoints = 1
mon endian little
mon reg r13 = (0x00000000)
mon reg pc = (0x00000004)
set mem inaccessible-by-default off
mon semihosting enable
"""

import socket
import socketserver
import threading
from time import sleep
import traceback
import sys
import gc
import functools # TODO

from Segger import GdbServer
from Swd import SwoServer

HOST = ("localhost", 9999)

SWO_PROTO = GdbServer.PROTOCOL_UART
SWO_SPEED = 875000

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
    self._finished = False
    self._plock = threading.Lock()
    
    # make sure there is only one connection
    if GdbProxyHandler.currentHandle != None:
      self.print_gdb('New connection killing old connection ...')
      GdbProxyHandler.currentHandle.close(wait=True)
      self.print_gdb(' OK\n')
    GdbProxyHandler.currentHandle = self
    
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
    self._segger = GdbServer('/opt/SEGGER/JLink_Linux_V443c') #, printter=self.print_segger)
    #sleep(3)
    
    # get our gdb server
    self._output = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    self._output.connect(self._segger.get_gdb_host())
    self._input = self.request
    
    # SWO?
    if self._enable_swo:
      # send the magic
      self._output.setblocking(1)
      self._input.setblocking(1)
      self._output.sendall(
          bytes(
              self._segger.swoStart(GdbServer.TRANSPORT_SWD, SWO_PROTO, SWO_SPEED), 'UTF-8'
            )
        )
      self._output.recv(len(self._segger.SWO_REPLY))
      self.print_gdb('Sent maigc and burnt reply')
      # start SWO reciver
      self._swo = SwoServer(self._segger.get_swv_host(), printter=self.print_swo)
    
    # clean up the sockets
    self._input.setblocking(0)
    self._output.setblocking(0)
  
  def handle(self):
    last5inbuf = bytes()
    inbuf      = bytes()
    outbuf     = bytes()
    
    while self._run: # and self._segger.is_alive():
      try:
        buf = self._input.recv(4096)
        inbuf += buf
        last5inbuf += buf
        last5inbuf = last5inbuf[-5:]
        sbuf = str(buf, 'ASCII', 'replace')
        while len(sbuf) > 0:
          if len(sbuf) > 40:
            self.print_gdb('>: ' + sbuf[:36] + ' ...')
            sbuf = sbuf[36:]
          else:
            self.print_gdb('>: ' + sbuf)
            sbuf = ''
      except Exception as e:
        if e.args[0] != 11:
          raise e
      try:
        outbuf += self._output.recv(4096)
        sbuf = str(buf, 'ASCII', 'replace')
        while len(sbuf) > 0:
          if len(sbuf) > 40:
            self.print_gdb('>: ' + sbuf[:36] + ' ...')
            sbuf = sbuf[36:]
          else:
            self.print_gdb('>: ' + sbuf)
            sbuf = ''
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
      
      if last5inbuf == bytes('$k#6b', 'ASCII'):
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
        self._run = False
    
    if self._output != None and last5inbuf != bytes('$k#6b', 'ASCII'):
      self.print_gdb('Trying to safely close by detaching ...')
      self._output.send(bytes(self._formGdbMsg('D'), 'ASCII'))
  
  # TODO
  @staticmethod
  def _formGdbMsg(msg):
    msg = functools.reduce(lambda x, y: x+y, map(lambda x: x in '#$}' and '}'+chr(ord(x)^0x20) or x, msg))
    chksum =sum(map(ord, msg))%256
    return '${msg}#{chksum:02X}'.format(msg=msg, chksum=chksum)
  
  def close(self, wait=False):
    self._run = False
    if not wait:
      return
    while not self._finished:
      pass
  
  def finish(self):
    if self._input != None:
      self._input.close()
      self._input = None
    if self._output != None:
      self._output.close()
      self._output = None
    if self._swo != None:
      self._swo.stop()
      self._swo = None
    if self.currentHandle != None:
      self.currentHandle = None
    if self._segger != None:
      self._segger.stop()
      self._segger = None
    print('Connection closed')
    gc.collect() # help the gc
    print('Waiting for other end ...')
    sleep(1)
    print('.. OK')
    self._finished = True

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
      server = GdbProxyServer(HOST, GdbProxyHandler, enable_swo=True)
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

