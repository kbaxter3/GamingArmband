# -*- coding: utf-8 -*-
# Created on Thu Feb 29 22:06:03 2024

# @author: Katerina

import os
# Importing Libraries
# import serial

import time
import copy

import matplotlib.pyplot as plt
import numpy as np
import random
import pygame
from pygame.locals import *
from collections import deque

f = open("/dev/null",  "w")
os.dup2(f.fileno(), 2)
f.close()

screen_width = 1600
screen_height = 900

# Define global variables
EMG_DATA_SIZE = screen_width - 100  # Assuming 100 samples per second for 5 seconds
emg_data_buffer = deque(maxlen=EMG_DATA_SIZE)
x_scale_factor = EMG_DATA_SIZE / 100  # Adjust the scale factor as needed

# stores newest data
class DataSample:
    emg = 0
    xOrientation = 0
    yOrientation = 0
    zOrientation = 0
    xGyro = 0
    yGyro = 0
    zGyro = 0
    xLinAccel = 0
    yLinAccel = 0
    zLinAccel = 0


# @William job
def initGraphicsEMG():
    pygame.init()
    screen = pygame.display.set_mode((screen_width, screen_height))
    pygame.display.set_caption("Live EMG Data Visualization")
    pygame.display.flip()
    return screen, True


# @William job
def updateGraphicsEMG(screen, dataPoint):
    if screen is None:
        return

    emg_data_buffer.append(dataPoint.emg)
    # Clear the screen
    screen.fill((19, 41, 75)) # uiuc orange background
    # (255, 95, 5) is uiuc orange
    # (19, 41, 75) is uiuc blue
    # pygame.draw.rect(screen, (19, 41, 75), (10, 10, screen_width / 2, screen_height / 2)) # top left
    # pygame.draw.rect(screen, (19, 41, 75), (screen_width / 2 + 20, screen_height / 2 + 20, screen_width / 2 - 30, screen_height / 2 - 30)) # bottom right
    # pygame.draw.rect(screen, (19, 41, 75), (10, screen_height / 2 + 20, screen_width / 2, screen_height / 2 - 30)) # bottom left
    # pygame.draw.rect(screen, (19, 41, 75), (screen_width / 2 + 20, 10, screen_width / 2 - 30, screen_height / 2)) # top right

    draw_emg_graph(screen)
    # Update the display
    pygame.display.flip()


def draw_emg_graph(screen):
    global emg_data_buffer

    x_margin = 10
    y_margin = 10

    graph_top_left_x = x_margin
    graph_top_left_y = y_margin
    graph_width = screen_width - x_margin
    graph_height = screen_height - y_margin

    pygame.draw.line(screen, (255, 255, 255), (graph_top_left_x, graph_top_left_y + graph_height - 50),
                     (graph_top_left_x + graph_width - x_margin, graph_top_left_y + graph_height - 50), 2)  # X-axis
    pygame.draw.line(screen, (255, 255, 255), (graph_top_left_x, graph_top_left_y),
                     (graph_top_left_x, graph_top_left_y + graph_height - y_margin), 2)  # Y-axis

    font_label = pygame.font.SysFont('Arial', 20)
    font_title = pygame.font.SysFont('Arial', 30)

    title = font_title.render('EMG Live Data', True, (255, 255, 255))
    label_y = font_label.render('Relative EMG Signal', True, (255, 255, 255))
    label_x = font_label.render('Time', True, (255, 255, 255))
    screen.blit(label_y, (graph_top_left_x + 10, graph_top_left_y))
    screen.blit(label_x, (graph_top_left_x + graph_width - 50, graph_top_left_y + graph_height - 50))
    screen.blit(title, (graph_top_left_x + screen_width / 2 - 100, graph_top_left_y))

    # emg_points = list(emg_data_buffer)
    max_emg_value = 1
    min_emg_value = 0

    max_val_label = font_label.render(f'{max_emg_value:.2f}', True, (255, 255, 255))
    min_val_label = font_label.render(f'{min_emg_value:.2f}', True, (255, 255, 255))

    screen.blit(max_val_label, (graph_top_left_x + 5, graph_top_left_y + 30))
    screen.blit(min_val_label, (graph_top_left_x + 5, graph_top_left_y + graph_height - 50))

    pygame.draw.line(screen, (255, 255, 255), (graph_top_left_x - 5, graph_top_left_y + 50), (graph_top_left_x + 5, graph_top_left_y + 50), 2)  # Max value tick
    pygame.draw.line(screen, (255, 255, 255), (graph_top_left_x - 5, graph_top_left_y + graph_height - 50), (graph_top_left_x + 5, graph_top_left_y + graph_height - 50), 2)  # Min value tick

    emg_points = list(emg_data_buffer)
    if len(emg_points) < 2:
        return

    # Scale the EMG data to fit the screen
    max_emg_value = max(emg_points) if max(emg_points) > 0 else 1
    scaled_emg_points = [int(((p / max_emg_value) * (graph_height - 100)) + graph_top_left_y + 50) for p in emg_points]

    # Draw lines connecting consecutive EMG data points
    for i in range(1, len(scaled_emg_points)):
        pygame.draw.line(screen, (255, 255, 255), (i - 1 + graph_top_left_x, scaled_emg_points[i - 1]), (i + graph_top_left_x, scaled_emg_points[i]), 1)


# Reads all new points from Serial Port
# Returns DataSample class
def readData(serial):
    newData = DataSample()
    serialMsg = serial.readline()
    while serialMsg == b'':
        serialMsg = serial.readline()

    newDataValues = serialMsg.decode('ascii').split(',')

    newData.emg = int(newDataValues[0].split(':')[1]) / 4096
    newData.xOrientation = float(newDataValues[1].split(':')[1])
    newData.yOrientation = float(newDataValues[2].split(':')[1])
    newData.zOrientation = float(newDataValues[3].split(':')[1])
    newData.xGyro = float(newDataValues[4].split(':')[1])
    newData.yGyro = float(newDataValues[5].split(':')[1])
    newData.zGyro = float(newDataValues[6].split(':')[1])
    newData.xLinAccel = float(newDataValues[7].split(':')[1])
    newData.yLinAccel = float(newDataValues[8].split(':')[1])
    newData.zLinAccel = float(newDataValues[9].split(':')[1])

    return newData


# Use for testing
# randomly updates all values to simulate new readings
def testReadData(oldData):
    newData = copy.deepcopy(oldData)
    newData.emg = newData.emg + random.random() - 0.5
    newData.xOrientation = (newData.xOrientation + (random.random() - 0.5) * 15)
    newData.yOrientation = (newData.yOrientation + (random.random() - 0.5) * 15)
    newData.zOrientation = (newData.zOrientation + (random.random() - 0.5) * 15)
    newData.xGyro = (random.random() - 0.5) * 10
    newData.yGyro = (random.random() - 0.5) * 10
    newData.zGyro = (random.random() - 0.5) * 10
    newData.xLinAccel = (random.random() - 0.5) * 10
    newData.yLinAccel = (random.random() - 0.5) * 10
    newData.zLinAccel = (random.random() - 0.5) * 10

    if newData.emg < 0:
        newData.emg = 0
    if newData.emg > 1:
        newData.emg = 1
    if newData.xOrientation > 360:
        newData.xOrientation -= 360
    if newData.xOrientation < 0:
        newData.xOrientation += 360
    if newData.yOrientation > 360:
        newData.yOrientation -= 360
    if newData.yOrientation < 0:
        newData.yOrientation += 360
    if newData.zOrientation > 360:
        newData.zOrientation -= 360
    if newData.zOrientation < 0:
        newData.zOrientation += 360

    return newData


# set up serial
# commented out for testing
# arduino = serial.Serial(port='COM3', baudrate=115200, timeout=.1) 


screen_emg, window_emg_created = initGraphicsEMG()
if not window_emg_created:
    print("Failed to create Pygame window")
    exit()

# Main loop
running = True
data = DataSample()
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    data = testReadData(data)
    updateGraphicsEMG(screen_emg, data)

pygame.quit()
