from BusPirateRadio import BusPirateRadio

if __name__ == '__main__':
  
  radio = BusPirateRadio('/dev/ttyUSB0')
  
  #for i in range(1, 10):
  #  print map(ord, radio.getStatus(old=True))
  #  print map(ord, radio.getConfig(old=True))
    
  for i in range(1, 14):
    print str(i) + ': ' + str(ord(radio.getReg(i, old=False)))
