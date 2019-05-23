import network, time, machine, onewire, ds18x20
from machine import Timer
from machine import Pin

print("Program starting...")

wlan = network.WLAN(network.STA_IF) # create station interface
wlan.active(True)       # activate the interface
wlan.scan()             # scan for access points
wlan.isconnected()      # check if the station is connected to an AP
wlan.connect('YOUR_WIFI_NAME', 'YOUR_WIFI_PASSWORD') # connect to an AP
wlan.config('mac')      # get the interface's MAC adddress
wlan.ifconfig()         # get the interface's IP/netmask/gw/DNS addresses
 
ow1 = onewire.OneWire(Pin(4)) # create a OneWire D2
ow2 = onewire.OneWire(Pin(5)) # create a OneWire D1
ds1 = ds18x20.DS18X20(ow1)
ds2 = ds18x20.DS18X20(ow2)
roms1 = ds1.scan()
roms2 = ds2.scan()

def sendDataToServer(timer):
  import urequests
  print("Start sending data")
  
  ds1.convert_temp()
  for rom in roms1:
      temp1 = ds1.read_temp(rom)

  ds2.convert_temp()
  for rom in roms2:
      temp2 = ds2.read_temp(rom)

  response = urequests.get("http://loggingforest.com/index.php/api?device_key=YOUR_DEVICE_KEY&secret_key=YOUR_DEVICE_PASSWORD&milis=100&temp1="+str(temp1)+"&temp2="+str(temp2))
  response.close()
  print("data sent")

tim = Timer(-1)
tim.init(period=60000, mode=Timer.PERIODIC, callback=sendDataToServer)

print("Program end")




