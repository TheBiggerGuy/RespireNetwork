from threading import Thread
import subprocess

class GdbServer(Thread):
  
  SWO_REPLY = '+$OK#9a'
  
  TRANSPORT_JTAG = 'JTAG'
  TRANSPORT_SWD  = 'SWD'
  
  PROTOCOL_UART = 'UART'
  PROTOCOL_MAN  = 'Manchester'
  
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
        bufsize=0, shell=False,
        stdout=subprocess.PIPE, stderr=subprocess.PIPE, stdin=None,
        cwd=seggerRoot, env={'LD_LIBRARY_PATH': seggerRoot}
      )
    self._out = self._pHandle.stdout
    self._err = self._pHandle.stderr
    self.start()
  
  def run(self):
    self._printter("\nSEGGER GDB Server started\n")
    while self._pHandle.returncode == None:
      # self._printter(self._pHandle.communicate(input=None)) #stdout.read(100)
      self._printter('OUT: ' + self._out.readline())
      self._printter('ERR: ' + self._err.readline())
      self._pHandle.poll()
    self._printter("\nSEGGER GDB Server ended\n")
  
  def stop(self):
    if self._pHandle != None:
      self._pHandle.terminate()
      self._pHandle = None
  
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

