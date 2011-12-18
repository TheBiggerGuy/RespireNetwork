"""



"""

"""
Object for a 'meesge' sent over the radio, ie a packet
"""
class RadioMessage(object):  
  def getFrom(self):
    return self._from
  
  def getTo(self):
    return self._to
  
  def getPayload(self):
    return self._msg


class RadioMessageEncoder(RadioMessage):
  def __init__(self, from_, to, msg):
    RadioMessage.__init__(self)
    self._from = from_
    self._to   = to
    self._msg  = msg

class RadioMessageDecoder(RadioMessage):
  def __init__(self, data):
    RadioMessage.__init__(self)
    self._from = from_
    self._to   = to
    self._msg  = msg
