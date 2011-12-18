"""



"""

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

