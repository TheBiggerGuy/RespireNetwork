#!/usr/bin/python2.7

import logging
from time import sleep
import serial.tools.list_ports

import RespireNRF

from gi.repository import Gtk, Gio, GObject

class RespireWindow(Gtk.Window):
  
  def __init__(self, app):
    Gtk.Window.__init__(
        self,
        type=Gtk.WindowType.TOPLEVEL
      )
    self.set_title("Respire Netowrk")
    
    self._app = app
    
    self.connect("delete-event", self.destroy)
    
    #window.set_border_width(24)
    
    # make the COM port drop down list
    name_store = Gtk.ListStore(str)
    for (port, name, id_) in serial.tools.list_ports.comports():
      name_store.append([name])
    name_combo = Gtk.ComboBox.new_with_model_and_entry(name_store)
    name_combo.set_entry_text_column(0)
    name_combo.connect("changed", self.on_com_changed)
    
    com_button = Gtk.Button('Connect')
    com_button.connect("clicked", self.on_com_click)
        
    hbox_com = Gtk.HBox()
    hbox_com.pack_start(name_combo, False, False, 0)
    hbox_com.pack_start(com_button, False, False, 0)
    
    # make other
    vbox = Gtk.VBox()
    label1 = Gtk.Label("Hello World!")
    label2 = Gtk.Label("Hello World!")
    
    # make all
    vbox.pack_start(label1, False, False, 0)
    vbox.pack_start(label2, False, False, 0)
    vbox.pack_start(hbox_com, False, False, 0)
    
    self.add(vbox)
    self.show_all()
  
  """ Called on COM port drop down is chnaged """
  def on_com_changed(self, data=None):
    if data == None:
      return
    tree_iter = data.get_active_iter()
    if tree_iter != None:
      model = data.get_model()
      name = str(model[tree_iter][0])
      print 'Selected: name=' + name
    else:
      entry = data.get_child()
      print 'Entered: ' + entry.get_text()
  
  """ Called on COM port 'connect' button """
  def on_com_click(self, data=None):
    print 'click'
  
  """ TODO """
  def destroy(self, widget=None, data=None):
    pass

class RespireApp(Gtk.Application):
  
  """ Start the app """
  def __init__(self):
    Gtk.Application.__init__(
        self,
        application_id="com.thebiggerguy.uni.respire.main",
        flags=Gio.ApplicationFlags.FLAGS_NONE
      )
    self.connect("activate", self.on_activate)
    
    self._radio = RadioController(app)
  
  """ Called when app is ready for a window """
  def on_activate(self, data=None):
    self.add_window(RespireWindow(self))

class RadioController(object):
  
  def __init__(self, app):
    self._app = app
    radio = RespireNRF.BusPirateNRF('/dev/ttyUSB0')
    
    radio.setState(RespireNRF.NrfStates.STANDBY1)
    
    radio.setReg(radio.STATUS, radio.RX_DR | radio.TX_DS | radio.MAX_RT)     # reset STATUS
    radio.setReg(radio.CONFIG, radio.CRCO | radio.EN_CRC | radio.MASK_RX_DR) # config CONFIG
  
  def main(self):
    while True:
      if not radio.getIRQ():
        print '.',
        sleep(1)
        continue
      
      print str(i) + ': ' + int2bin(radio.getReg(i))
      print 'IRQ: ' + str(radio.getIRQ())

if __name__ == '__main__':
  
  logging.basicConfig(
    filename='pylogger.log',
    level=logging.INFO
  )
  
  app = RespireApp()  
  # run Gtk main loop
  app.run(None)


