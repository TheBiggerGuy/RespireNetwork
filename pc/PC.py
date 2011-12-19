import RespireNRF

def int2bin(n, count=8):
  """returns the binary of integer n, using count number of digits"""
  return "".join([str((n >> y) & 1) for y in range(count-1, -1, -1)])

if __name__ == '__main__':
  
  radio = RespireNRF.BusPirateNRF('/dev/ttyUSB0')
  
  #for i in range(1, 10):
  #  print map(ord, radio.getStatus(old=True))
  #  print map(ord, radio.getConfig(old=True))
  
  radio.setState(RespireNRF.NrfStates.STANDBY1)
  
  for i in range(0, 14):
    print str(i) + ': ' + int2bin(radio.getReg(i))
    print 'IRQ: ' + str(radio.getIRQ())
