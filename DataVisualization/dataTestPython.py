# -*- coding: utf-8 -*-
"""
Created on Thu Feb 29 22:06:03 2024

@author: Katerina
"""

# Importing Libraries 
import serial 
import time 
import copy
import numpy as np
import random
import matplotlib.pyplot as plt

# stores newest data
class DataSample:
    emg = 0;
    xOrientation = 0;
    yOrientation = 0;
    zOrientation = 0;
    xGyro = 0;
    yGyro = 0;
    zGyro = 0;
    xLinAccel = 0;
    yLinAccel = 0;
    zLinAccel = 0;
    
# @William job
def initGraphics():
    #do smth
    return 0
    
# @William job
def updateGraphics(dataPoint):
    # do smth
    return 0

# Reads all new points from Serial Port
# Returns DataSample class
def readData(serial):
    newData = DataSample()
    serialMsg = serial.readline()
    while(serialMsg == b''):
        serialMsg = serial.readline()
    
    newDataValues = serialMsg.decode('ascii').split(',')
    
    newData.emg            = int(newDataValues[0].split(':')[1]) / 4096
    newData.xOrientation   = float(newDataValues[1].split(':')[1])
    newData.yOrientation   = float(newDataValues[2].split(':')[1])
    newData.zOrientation   = float(newDataValues[3].split(':')[1])
    newData.xGyro          = float(newDataValues[4].split(':')[1])
    newData.yGyro          = float(newDataValues[5].split(':')[1])
    newData.zGyro          = float(newDataValues[6].split(':')[1])
    newData.xLinAccel      = float(newDataValues[7].split(':')[1])
    newData.yLinAccel      = float(newDataValues[8].split(':')[1])
    newData.zLinAccel      = float(newDataValues[9].split(':')[1])
    
    return newData

# Use for testing 
# randomly updates all values to simulate new readings
def testReadData(oldData):
    newData = copy.deepcopy(oldData)
    newData.emg            = newData.emg + random.random() - 0.5
    newData.xOrientation   = (newData.xOrientation + (random.random()-0.5) * 15) 
    newData.yOrientation   = (newData.yOrientation + (random.random()-0.5) * 15) 
    newData.zOrientation   = (newData.zOrientation + (random.random()-0.5) * 15)
    newData.xGyro          = (random.random()-0.5) * 10
    newData.yGyro          = (random.random()-0.5) * 10
    newData.zGyro          = (random.random()-0.5) * 10
    newData.xLinAccel      = (random.random()-0.5) * 10
    newData.yLinAccel      = (random.random()-0.5) * 10
    newData.zLinAccel      = (random.random()-0.5) * 10
    
    
    if(newData.emg < 0): 
        newData.emg = 0
    if(newData.emg > 1):
        newData.emg = 1
    if(newData.xOrientation > 360):
        newData.xOrientation -= 360
    if(newData.xOrientation < 0):
        newData.xOrientation += 360
    if(newData.yOrientation > 360):
        newData.yOrientation -= 360
    if(newData.yOrientation < 0):
        newData.yOrientation += 360
    if(newData.zOrientation > 360):
        newData.zOrientation -= 360
    if(newData.zOrientation < 0):
        newData.zOrientation += 360
    
    return newData
    

# set up serial
# commented out for testing
# arduino = serial.Serial(port='COM3', baudrate=115200, timeout=.1) 

initGraphics()

data = DataSample()

# Main loop
while(True): 
    
    data = testReadData(data)
    
    updateGraphics(data)
    
    
    
    
    
    
    
    
    
	
	