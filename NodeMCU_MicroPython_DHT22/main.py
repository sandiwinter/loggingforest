

import network
import dht
import machine
from machine import Timer

print("Program starting...")

wlan = network.WLAN(network.STA_IF) # create station interface
wlan.active(True)       # activate the interface
wlan.scan()             # scan for access points
wlan.isconnected()      # check if the station is connected to an AP
wlan.connect('YOUR_WIFI_NAME', 'YOUR_WIFI_PASSWORD') # connect to an AP
wlan.config('mac')      # get the interface's MAC adddress
wlan.ifconfig()         # get the interface's IP/netmask/gw/DNS addresses

def sendDataToServer(timer):
  import urequests
  print("Start sending data")
  d = dht.DHT22(machine.Pin(4))
  d.measure()
  d.temperature() # eg. 23.6 (°C)
  d.humidity()    # eg. 41.3 (% RH)import dht

  response = urequests.get("http://loggingforest.com/index.php/api?device_key=YOUR_DEVICE_KEY&secret_key=YOUR_DEVICE_PASSWORD&milis=100&temp="+str(d.temperature())+"&hum="+str(d.humidity()))
  response.close()
  print("data sent")

tim = Timer(-1)
tim.init(period=60000, mode=Timer.PERIODIC, callback=sendDataToServer)

print("Program end")



