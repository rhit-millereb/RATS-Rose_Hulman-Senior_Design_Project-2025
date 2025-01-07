import matplotlib.pyplot as plt

# Define parameters
circle_radius = 300  # radius of circles
zone_size = 800    # size of the square area

# Calculate spacing between circle centers
spacing = circle_radius * (1 - 1/9)
#spacing = circle_radius * (1 - 1/20)  # Allowing for overlap to ensure coverage by at least 3 circles
#spacing = circle_radius * (1 - 1/9)

# Calculate the number of circles needed along each dimension
num_circles_per_side = int(zone_size / spacing) + 1 # Extra circles for full coverage
#num_circles_per_side = int(zone_size / spacing) + 2

# Generate circle centers
centers = []
circles = []
for i in range(num_circles_per_side):
    for j in range(num_circles_per_side):
        center_x = i * spacing
        center_y = j * spacing
        centers.append((center_x, center_y))

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

legend = []
for x in range((int)(zone_size/5)):
    for y in range((int)(zone_size/5)):
        point = (x*5,y*5)
        overlap = 0
        for circle in circles:
            if circle.contains_point(ax.transData.transform(point)):
                overlap += 1
        if overlap == 3:
            background = plt.Circle(point, 5 ,color='lime', fill=True, linewidth=1, zorder = -2, label = 'lime = 3 overlaps' )
            if len(legend) == 0:
                legend.insert(0,background)
            ax.add_artist(background)
        elif overlap == 4:
            background = plt.Circle(point, 5 ,color='teal', fill=True, linewidth=1, zorder = -1, label = 'Teal = 4 overlaps')
            if len(legend) == 1:
                legend.insert(1,background)
            ax.add_artist(background)
        elif overlap == 5:
            background = plt.Circle(point, 5 ,color='yellow', fill=True, linewidth=1, zorder = 0, label = 'Yellow = 5 overlaps')
            if len(legend) == 2:
                legend.insert(2,background)
            ax.add_artist(background)
        else:
            background = plt.Circle(point, 5 ,color='red', fill=True, linewidth=1, zorder = 1, label = 'Red = Less than 3 overlaps')
            if len(legend) == 3:
                legend.insert(3,background)
            ax.add_artist(background)

# Add grid and labels
ax.set_title('Circles Covering a 800m x 800m Area')
print(len(centers))
plt.grid(True)
plt.legend(handles = [legend[3], legend[0], legend[1], legend[2]], loc = 4)
plt.show()
            
            
