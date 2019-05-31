# -*- coding: utf-8 -*-

import os
import urllib2
import Adafruit_DHT as dht
import RPi.GPIO as GPIO
import threading

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)

def sendDataToServer():
    threading.Timer(60,sendDataToServer).start()
    print("Sensing...")
    h,t = dht.read_retry(dht.DHT22,4)
    print(h,", ",t)
    
    if h<=100 and t<=100:
        urllib2.urlopen("http://loggingforest.com/index.php/api?device_key=YOUR_DEVICE_KEY&secret_key=YOUR_DEVICE_PASSWORD&milis=100&temp="+str(t)+"&hum="+str(h)+"").read()

sendDataToServer()

exit()