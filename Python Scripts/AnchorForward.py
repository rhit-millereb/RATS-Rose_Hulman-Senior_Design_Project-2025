import random
import time
import numpy                                             
from numpy import sqrt, dot, cross                       
from numpy.linalg import norm  
import matplotlib.pyplot as plt
from matplotlib.patches import Ellipse

class Anchor:
    def __init__(self, x, y, z, is_server):
        self.x = x
        self.y = y
        self.z = z
        if (is_server):
            self.server_distance = 0
        
        else:
            self.server_distance = 1000
        
        self.next_node = Anchor
    
        

def main():
    Anchors = [0] * 3
    Anchors[0] = Anchor(0.0,0.0,0.0,False)
    Anchors[1] = Anchor(250.0,0.0,0.0,False)
    Anchors[2] = Anchor(125.0,125.0,0.0,False)
    Roams = [0] * 4
    Roams[0] = numpy.array([125.0,62.0,10.0])
    Roams[1] = numpy.array([275.0,62.0,0.0])
    Roams[2] = numpy.array([300.0,200.0,0.0])
    Roams[3] = numpy.array([0.0,400.0,10.0])
    #find_error(P1,P2,P3,Roams[0])
    node_sim(Anchors,Roams)
    
def node_sim(P1,P2,P3,Roams):
    plt.ion()
    plt.show()
    plt.gca().set_aspect('equal')
    plt.scatter((P1[0], P2[0], P3[0]),(P1[1], P2[1], P3[1]), color = 'b')

    for x in range(100):
        y = 0
        for Roam in Roams:
            range = findrange(P1, P2, P3, Roam)
            
            plt.gca().add_patch(circles1[y])
            plt.gca().add_patch(circles2[y])
            plt.gca().add_patch(circles3[y])
            plt.grid()
            plt.draw()
            plt.pause(1)
            
            plt.grid()
            plt.draw()
            plt.pause(1)
        
def find_error(P1, P2, P3, Roam):  
    x_list = [0] * 1000
    y_list = [0] * 1000
    z_list = [0] * 1000
    x_error = 0
    y_error = 0
    z_error = 0
    for x in range(1000):
        ranges = findrange(P1, P2, P3, Roam)
        output = trilaterate(P1,P2,P3, ranges[0],ranges[1],ranges[2])
        x_list[x] = output[0]
        y_list[x] = output[1] 
        z_list[x] = output[2]
    for x in range(1000):
        x_error += pow(x_list[x]-Roam[0],2)
        y_error += pow(y_list[x]-Roam[1],2) 
        z_error += pow(z_list[x]-Roam[2],2) 
    x_error = sqrt(x_error/1000)
    y_error = sqrt(y_error/1000)
    z_error = sqrt(z_error/1000)
    print(x_error)
    print(y_error)
    print(z_error)
    ellipsis_xy = Ellipse((Roam[0],Roam[1]), x_error*2,y_error*2, color = 'r', alpha = 0.3, zorder = 3)
    ellipsis_xz = Ellipse((Roam[0],Roam[2]), x_error*2,z_error*2, color = 'r', alpha = 0.3, zorder = 3)
    plt.scatter(x_list, y_list, zorder = 2)
    plt.scatter(Roam[0], Roam[1], zorder = 2)
    plt.gca().add_patch(ellipsis_xy)
    plt.gca().set_box_aspect(1)
    plt.xlabel("x-axis (m)")
    plt.ylabel("y-axis (m)")
    plt.grid()
    #plt.show()
    plt.scatter(x_list, z_list, zorder =2)
    plt.scatter(Roam[0], Roam[2], zorder = 2)
    plt.gca().add_patch(ellipsis_xz)
    plt.xlabel("x-axis (m)")
    plt.ylabel("z-axis (m)")
    plt.grid()
    #plt.show()


def findrange(P1, P2):
    range = sqrt(pow(P1[0]- P2[0],2)+pow(P1[1]-P2[1],2)+pow(P1[2]-P2[2],2)) + random.uniform(-0.05,0.05)
    
    return range

# Find the intersection of three spheres                 
# P1,P2,P3 are the centers, r1,r2,r3 are the radii       
# Implementaton based on Wikipedia Trilateration article.                              
def trilaterate(P1,P2,P3,r1,r2,r3):                      
    temp1 = P2-P1                                        
    e_x = temp1/norm(temp1)                              
    temp2 = P3-P1                                        
    i = dot(e_x,temp2)                                   
    temp3 = temp2 - i*e_x                                
    e_y = temp3/norm(temp3)                              
    e_z = cross(e_x,e_y)                                 
    d = norm(P2-P1)                                      
    j = dot(e_y,temp2)                                   
    x = (r1*r1 - r2*r2 + d*d) / (2*d)                    
    y = (r1*r1 - r3*r3 -2*i*x + i*i + j*j) / (2*j)       
    temp4 = r1*r1 - x*x - y*y
    z = sqrt(abs(temp4))                               
    p_12_a = P1 + x*e_x + y*e_y + z*e_z                  
    return p_12_a  

if __name__ == "__main__":
    main();      
