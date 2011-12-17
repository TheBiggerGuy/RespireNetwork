
Message(object):
  def __init__(self):
    pass
  
  def getFrom(self):
    pass
  
  def getTo(self):
    pass
  
  def getPayload(self):
    pass

Radio(object):
  def __init__(self):
    pass
  
  def regNewMsgCallback(self, callback):
    pass
  
  def sendMsg(self, msg):
    pass

BusPirateRadio(Radio):
  def __init__(self, port):
    pass
  
  def connect(self):
    
    # Open serial port
    try:
      spi = SPI('/dev/ttyUSB0', 115200)
    except SerialException as ex:
      raise IOError('Unable to open serial port')
    
    # Enter binmode
    if not spi.BBmode():
      raise IOError('Unable to start binary mode')
    
    # Enter raw SPI mode
    if not spi.enter_SPI():
      raise IOError('Unable to start SPI')
    
    # Configuring SPI peripherals
    if not spi.cfg_pins(PinCfg.POWER | PinCfg.CS):
      raise IOError('Unable to start SPI')
    
    # Configure SPI speed
    if spi.set_speed(SPISpeed._1MHZ):
      raise IOError('Unable to start SPI')
    
    # Configure SPI configuration
    #if spi.cfg_spi(SPICfg.OUT_TYPE | ~SPICfg.IDLE | ~SPICfg.CLK_EDGE | ~SPICfg.SAMPLE ):
    if not spi.cfg_spi(SPICfg.OUT_TYPE):
      raise IOError('Unable to start SPI')
    spi.timeout(0.2)

    print 'Waiting 1s: ', 
    time.sleep(1)
    print 'OK.'

