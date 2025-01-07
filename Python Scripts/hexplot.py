import matplotlib.pyplot as plt
import numpy as np
import math

def hex_grid(side_length, radius):
    fig, ax = plt.subplots()


import matplotlib.pyplot as plt

# Define parameters
circle_radius = 300  # radius of circles
zone_size = 800    # size of the square area

 # Calculate the height of a hexagon
hex_height = (1/2) * circle_radius
    # Calculate the width of a hexagon
hex_width = math.sqrt(3) * circle_radius * (1/2)

    # Calculate the number of rows and columns
num_rows = int(zone_size / hex_height) 
num_cols = int(zone_size / hex_width) 

centers = []
circles = []
for row in range(num_rows):
    y = row * hex_height
    for col in range(num_cols):
        x = col * hex_width
        if row % 2 == 1:
            x += hex_width / 2
        centers.append((x,y))

# Plotting the circles
fig, ax = plt.subplots()
for center in centers:
    circle = plt.Circle(center, circle_radius, edgecolor='blue', fill=False, linewidth=1, zorder = 2)
    circles.append(circle)
    point = plt.Circle(center, 10 ,color='orange', fill=True, linewidth=1, zorder = 2)
    ax.add_artist(circle)
    ax.add_artist(point)

# Set limits and aspect
ax.set_xlim(-circle_radius, zone_size + circle_radius)
ax.set_ylim(-circle_radius, zone_size + circle_radius)
ax.set_aspect('equal', adjustable='box')

for x in range((int)(zone_size/5)):
    for y in range((int)(zone_size/5)):
        point = (x*5,y*5)
        overlap = 0
        for circle in circles:
            if circle.contains_point(ax.transData.transform(point)):
                overlap += 1
        if overlap == 3:
            background = plt.Circle(point, 5 ,color='lime', fill=True, linewidth=1, zorder = -2)
            ax.add_artist(background)
        elif overlap == 4:
            background = plt.Circle(point, 5 ,color='teal', fill=True, linewidth=1, zorder = -1)
            ax.add_artist(background)
        elif overlap == 5:
            background = plt.Circle(point, 5 ,color='yellow', fill=True, linewidth=1, zorder = 0)
            ax.add_artist(background)
        else:
            background = plt.Circle(point, 5 ,color='red', fill=True, linewidth=1, zorder = 1)
            ax.add_artist(background)

# Add grid and labels
ax.set_title('Circles Covering a 800m x 800m Area')
print(len(centers))
plt.grid(True)
plt.show()
            
            
