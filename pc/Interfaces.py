"""



"""

"""
Object for a 'meesge' sent over the radio, ie a packet
"""
class Message(object):
  def __init__(self):
    raise NotImplementedError()
  
  def getFrom(self):
    raise NotImplementedError()
  
  def getTo(self):
    raise NotImplementedError()
  
  def getPayload(self):
    raise NotImplementedError()

"""
Interface for any radio
"""
class Radio(object):
  def __init__(self):
    raise NotImplementedError()
  
  def regNewMsgCallback(self, callback):
    raise NotImplementedError()
  
  def sendMsg(self, msg):
    raise NotImplementedError()
  
  def setChannel(self, channel):
    raise NotImplementedError()
  
  def getChannel(self):
    raise NotImplementedError()

"""
Interface for the nRF24L01+ radio
"""
class NRF24L01P(Radio):
  def __init__(self):
    raise NotImplementedError()
  
  def connect(self):
    raise NotImplementedError()
  
  def enableTx(self, yes):
    raise NotImplementedError()
  
  def enableRx(self, yes):
    raise NotImplementedError()
  
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
