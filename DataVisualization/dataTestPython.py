# -*- coding: utf-8 -*-
# Created on Thu Feb 29 22:06:03 2024

# @author: Katerina

import os
# Importing Libraries
import serial

import time
import copy

import matplotlib.pyplot as plt
import numpy as np
import random
import pygame
from pygame import Vector3
from pygame.locals import *
from collections import deque
from matplotlib.animation import FuncAnimation
from mpl_toolkits.mplot3d import Axes3D

# resolve mac errors
# f = open("/dev/null",  "w")
# os.dup2(f.fileno(), 2)
# f.close()

screen_width = 1600
screen_height = 900

# Define global variables
EMG_DATA_SIZE = screen_width / 2  # Assuming 100 samples per second for 5 seconds
emg_data_buffer = deque(maxlen=int(EMG_DATA_SIZE))
x_scale_factor = EMG_DATA_SIZE / 1  # Adjust the scale factor as needed
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

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
def initGraphics():
    pygame.init()
    screen = pygame.display.set_mode((screen_width, screen_height))
    pygame.display.set_caption("Live Data Visualization")
    pygame.display.flip()
    return screen, True


# @William job
def updateGraphics(screen, dataPoint):
    if screen is None:
        return

    emg_data_buffer.append(dataPoint.emg)
    # Clear the screen
    screen.fill((19, 41, 75)) # uiuc orange background
    # (255, 95, 5) is uiuc orange
    # (19, 41, 75) is uiuc blue

    draw_emg_graph(screen)

    v_prime = calc_rotation(dataPoint)

    # vector3d = Vector3(v_prime)

    scale = 300
    distance = 6
    shift = screen_width / 4
    z_factor = distance / (distance + v_prime[2])
    x_2d = screen_width / 2 + v_prime[0] * scale * z_factor
    y_2d = screen_height / 2 - v_prime[1] * scale * z_factor
    end_point = (int(x_2d) + shift, int(y_2d))

    # Draw the 3D vector on the screen
    center_point = (screen_width * 0.5 + shift, screen_height * 0.5)  # Center of the screen
    pygame.draw.line(screen, (255, 95, 5), center_point, end_point, 5)  # Draw the vector with uiuc blue color

    axis_color = (255, 255, 255)  # White for visibility
    axis_length = 300  # Adjust as needed

    # Draw X-axis (Red)
    pygame.draw.line(screen, (255, 0, 0), center_point, (center_point[0] + axis_length, center_point[1]), 2)
    pygame.draw.line(screen, (255, 0, 0), center_point, (center_point[0] - axis_length, center_point[1]), 2)

    # Draw Y-axis (Green)
    pygame.draw.line(screen, (0, 255, 0), center_point, (center_point[0], center_point[1] + axis_length), 2)
    pygame.draw.line(screen, (0, 255, 0), center_point, (center_point[0], center_point[1] - axis_length), 2)

    # Draw Z-axis (White) - Simulating 3D effect
    pygame.draw.line(screen, (255, 255, 255), center_point, (center_point[0] + axis_length // 2, center_point[1] - axis_length // 2), 2)
    pygame.draw.line(screen, (255, 255, 255), center_point, (center_point[0] - axis_length // 2, center_point[1] + axis_length // 2), 2)

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

    pygame.draw.line(screen, (255, 255, 255), (graph_top_left_x, graph_top_left_y + graph_height - 40),
                     (graph_top_left_x + graph_width / 2 + 10, graph_top_left_y + graph_height - 40), 2)  # X-axis
    pygame.draw.line(screen, (255, 255, 255), (graph_top_left_x, graph_top_left_y),
                     (graph_top_left_x, graph_top_left_y + graph_height - y_margin), 2)  # Y-axis

    font_label = pygame.font.SysFont('Arial', 20)
    font_title = pygame.font.SysFont('Arial', 30)

    title = font_title.render('Live Data', True, (255, 255, 255))
    label_y = font_label.render('Relative EMG Signal', True, (255, 255, 255))
    label_x = font_label.render('Time', True, (255, 255, 255))
    screen.blit(label_y, (graph_top_left_x + 10, graph_top_left_y))
    screen.blit(label_x, (graph_top_left_x + graph_width / 2 - 40, graph_top_left_y + graph_height - 40))
    screen.blit(title, (graph_top_left_x + screen_width / 2 - 100, graph_top_left_y))

    # emg_points = list(emg_data_buffer)
    max_emg_value = 1  # max(emg_points) if emg_points else 0
    min_emg_value = 0

    max_val_label = font_label.render(f'{max_emg_value:.2f}', True, (255, 255, 255))
    min_val_label = font_label.render(f'{min_emg_value:.2f}', True, (255, 255, 255))

    screen.blit(max_val_label, (graph_top_left_x + 5, graph_top_left_y + 30))
    screen.blit(min_val_label, (graph_top_left_x + 5, graph_top_left_y + graph_height - 40))

    pygame.draw.line(screen, (255, 255, 255), (graph_top_left_x - 5, graph_top_left_y + 50), (graph_top_left_x + 5, graph_top_left_y + 50), 2)  # Max value tick
    pygame.draw.line(screen, (255, 255, 255), (graph_top_left_x - 5, graph_top_left_y + graph_height - 40), (graph_top_left_x + 5, graph_top_left_y + graph_height - 40), 2)  # Min value tick

    emg_points = list(emg_data_buffer)
    if len(emg_points) < 2:
        return

    # Scale the EMG data to fit the screen
    max_emg = max(emg_points) if max(emg_points) > 0 else 1
    scaled_emg_points = [int(((p / max_emg_value) * (graph_height - 90)) + graph_top_left_y + 50) for p in emg_points]

    chunk_size = (screen_height - 100) / max_emg_value
    scale_shift = (screen_height - 100) - chunk_size

    # Draw lines connecting consecutive EMG data points
    for i in range(1, len(scaled_emg_points)):
        pygame.draw.line(screen, (255, 255, 255), (i - 1 + graph_top_left_x, scaled_emg_points[i - 1] + scale_shift - max_emg_value), (i + graph_top_left_x, scaled_emg_points[i] + scale_shift - max_emg_value), 1)


def calc_rotation(dataPoint):
    # Define the Euler angles
    alpha = np.radians(dataPoint.xOrientation)
    beta = np.radians(dataPoint.yOrientation)
    gamma = np.radians(dataPoint.zOrientation)

    # Define the rotation matrices
    Rz_alpha = np.array([
        [np.cos(alpha), -np.sin(alpha), 0],
        [np.sin(alpha), np.cos(alpha), 0],
        [0, 0, 1]
    ])

    Rx_beta = np.array([
        [np.cos(beta), 0, np.sin(beta)],
        [0, 1, 0],
        [-np.sin(beta), 0, np.cos(beta)]
    ])

    Rz_gamma = np.array([
        [1, 0, 0],
        [0, np.cos(gamma), -np.sin(gamma)],
        [0, np.sin(gamma), np.cos(gamma)]
    ])

    # The initial vector
    v = np.array([1, 0, 0])

    # Apply the rotations
    v_prime = Rz_gamma.dot(Rx_beta.dot(Rz_alpha.dot(v)))

    return v_prime


# Reads all new points from Serial Port
# Returns DataSample class
def readData(serial):
    newData = DataSample()
    serialMsg = serial.readline()
    while serialMsg == b'':
        serialMsg = serial.readline()

    newDataValues = serialMsg.decode('ascii').split(',')
    newData.emg = int(newDataValues[0].split(':')[1]) / float(4096)
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

screen, window_created = initGraphics()
if not window_created:
    print("Failed to create Pygame window")
    exit()

# Main loop
running = True
data = DataSample()
# port_loc = self.builder.get_variable('port_location')
# port = port_loc.get()
ser = serial.Serial(port='/dev/cu.usbmodem1101', baudrate=115200, timeout=.01)
# ser.baudrate(9600)
# ser.port = "/dev.cu.usbmodem1101"
# ser.open()
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
    temp = time.time()
    data = readData(ser)
    updateGraphics(screen, data)

pygame.quit()
