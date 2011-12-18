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

"""
Interface for the nRF24L01+ radio
"""
class NRF24L01P(Radio):
  def __init__(sef):
    raise NotImplementedError()
  
  def connect(self):
    raise NotImplementedError()
