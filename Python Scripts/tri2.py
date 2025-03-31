import random
import time
import numpy
from numpy import sqrt, dot, cross
from numpy.linalg import norm
import matplotlib.pyplot as plt


def trilaterate(P1, P2, P3, r1, r2, r3):
    temp1 = P2 - P1
    e_x = temp1 / norm(temp1)
    temp2 = P3 - P1
    i = dot(e_x, temp2)
    temp3 = temp2 - i * e_x
    e_y = temp3 / norm(temp3)
    e_z = cross(e_x, e_y)
    d = norm(P2 - P1)
    j = dot(e_y, temp2)
    x = (r1 * r1 - r2 * r2 + d * d) / (2 * d)
    y = (r1 * r1 - r3 * r3 - 2 * i * x + i * i + j * j) / (2 * j)
    temp4 = r1 * r1 - x * x - y * y
    z = sqrt(abs(temp4))
    p_12_a = P1 + x * e_x + y * e_y + z * e_z
    return p_12_a

if __name__ == "__main__":
    
    P1 = numpy.array([0.0, 0.0, 0.0])
    P2 = numpy.array([4.5, 0.0, 0.0])
    P3 = numpy.array([2.0, 1.5, 0.0])
    
    
    rs1 = [
        1.91, 1.92, 1.95, 1.9, 1.92, 1.88, 1.91, 1.96, 1.94, 1.91,
        1.92, 1.96, 1.93, 1.92, 1.92, 1.94, 1.93, 1.91, 1.94, 1.89,
        1.91, 1.92, 1.92, 1.95, 1.96, 1.94, 1.92, 1.95, 1.94, 1.93,
        1.96, 1.92, 1.92, 1.92, 1.94, 1.91, 1.9, 1.93, 1.92, 1.9,
        1.91, 1.91, 1.94, 1.91, 1.99, 1.94, 1.91, 1.94, 1.93, 1.96,
        1.95, 1.89, 1.91, 1.9, 1.92, 1.92, 1.91, 1.92, 1.92, 1.94,
        1.91, 1.9, 1.93, 1.95, 1.92, 1.89, 1.93, 1.91, 1.92, 1.91,
        1.9, 1.9, 1.94, 1.94, 1.92, 1.92, 1.94, 1.93, 1.92, 1.92,
        1.9, 1.96, 1.95, 1.93, 1.91, 1.88, 1.92, 1.95, 1.93, 1.92,
        1.92, 1.91, 1.94, 1.92, 1.95, 1.93, 1.94, 1.92, 1.91, 1.94
    ]
    rs2 = [
        2.45, 2.43, 2.43, 2.43, 2.44, 2.47, 2.44, 2.43, 2.47, 2.41,
        2.46, 2.44, 2.43, 2.4, 2.45, 2.42, 2.4, 2.45, 2.46, 2.42,
        2.43, 2.43, 2.45, 2.45, 2.43, 2.43, 2.44, 2.43, 2.44, 2.43,
        2.43, 2.44, 2.43, 2.42, 2.44, 2.44, 2.42, 2.44, 2.42, 2.43,
        2.42, 2.43, 2.42, 2.42, 2.45, 2.42, 2.43, 2.42, 2.43, 2.42,
        2.43, 2.45, 2.43, 2.41, 2.43, 2.42, 2.42, 2.42, 2.44, 2.42,
        2.43, 2.41, 2.42, 2.43, 2.43, 2.41, 2.45, 2.42, 2.42, 2.46,
        2.43, 2.41, 2.42, 2.43, 2.41, 2.43, 2.44, 2.45, 2.4, 2.4,
        2.42, 2.41, 2.41, 2.43, 2.46, 2.44, 2.42, 2.41, 2.42, 2.44,
        2.43, 2.42, 2.42, 2.44, 2.41, 2.43, 2.45, 2.44, 2.4, 2.43
    ]
    rs3 = [
        1.39, 1.42, 1.41, 1.4, 1.41, 1.47, 1.43, 1.4, 1.42, 1.41,
        1.4, 1.41, 1.44, 1.4, 1.41, 1.43, 1.41, 1.38, 1.41, 1.37,
        1.39, 1.38, 1.38, 1.39, 1.41, 1.45, 1.37, 1.41, 1.38, 1.38,
        1.4, 1.39, 1.38, 1.37, 1.38, 1.38, 1.41, 1.37, 1.37, 1.39,
        1.42, 1.4, 1.41, 1.41, 1.38, 1.4, 1.38, 1.38, 1.39, 1.39,
        1.39, 1.43, 1.4, 1.4, 1.41, 1.4, 1.4, 1.38, 1.38, 1.39,
        1.4, 1.39, 1.43, 1.4, 1.4, 1.43, 1.41, 1.39, 1.36, 1.38,
        1.38, 1.37, 1.38, 1.43, 1.38, 1.4, 1.43, 1.37, 1.4, 1.39,
        1.4, 1.39, 1.42, 1.43, 1.4, 1.39, 1.42, 1.4, 1.39, 1.39,
        1.42, 1.4, 1.38, 1.4, 1.39, 1.4, 1.39, 1.38, 1.41, 1.42
    ]
    

    fig, axs = plt.subplots(2, 2, figsize=(14, 8))

    ax1 = axs[0, 0]  
    ax2 = axs[0, 1] 
    ax3 = axs[1, 0]  
    ax4 = axs[1, 1]  
    
    points = []
    for i in range(100):
        pt = trilaterate(P1, P2, P3, rs1[i], rs2[i], rs3[i])
        points.append(pt)
        if i == 0:
            ax1.scatter(pt[0], pt[1], color="r", label="Predicted Roaming Locations (meters)")
        else:
            ax1.scatter(pt[0], pt[1], color="r")
    
  
    ax1.scatter([P1[0], P2[0], P3[0]], [P1[1], P2[1], P3[1]], color='b', label="Anchor Nodes")
    actual_x, actual_y = 2, 0
    ax1.scatter(actual_x, actual_y, color="g", label="Actual Roaming Location (meters)")
    ax1.grid(True)
    ax1.legend()
    ax1.set_title("Two-Way-Ranging Trilateration (3 Anchors)")
    
    
    zoom_width = 0.1  # zoom in
    ax2.scatter([pt[0] for pt in points], [pt[1] for pt in points], color="r")
    ax2.scatter(actual_x, actual_y, color="g")
    ax2.set_xlim(actual_x - zoom_width, actual_x + zoom_width)
    ax2.set_ylim(actual_y - zoom_width, actual_y + zoom_width)
    ax2.grid(True)
    ax2.set_title("Zoomed-In View of Two-Way-Ranging Trilateration")

    red_points_np = numpy.array(points)
    x_errors = red_points_np[:, 0] - actual_x
    y_errors = red_points_np[:, 1] - actual_y
    
    mean_x_error = numpy.mean(x_errors)
    std_x_error = numpy.std(x_errors)
    mean_y_error = numpy.mean(y_errors)
    std_y_error = numpy.std(y_errors)

    indices = range(1, len(x_errors) + 1)

    # x errors
    ax3.scatter(indices, x_errors, color='blue', label='X Errors')
    ax3.axhline(mean_x_error, color='blue', linestyle='--', label='Mean X Error')
    ax3.axhline(mean_x_error + std_x_error, color='blue', linestyle=':', label='Mean X + Std')
    ax3.axhline(mean_x_error - std_x_error, color='blue', linestyle=':', label='Mean X - Std')
    ax3.set_xlabel('Measurement Index')
    ax3.set_ylabel('X Error (meters)')
    ax3.set_title('X Errors with Std Deviation + / -')
    ax3.legend(loc='lower right', framealpha=0.4)
    ax3.grid(True)
    
    # y errors
    ax4.scatter(indices, y_errors, color='orange', label='Y Errors')
    ax4.axhline(mean_y_error, color='orange', linestyle='--', label='Mean Y Error')
    ax4.axhline(mean_y_error + std_y_error, color='orange', linestyle=':', label='Mean Y + Std')
    ax4.axhline(mean_y_error - std_y_error, color='orange', linestyle=':', label='Mean Y - Std')
    ax4.set_xlabel('Measurement Index')
    ax4.set_ylabel('Y Error (meters)')
    ax4.set_title('Y Errors with Std Deviation + / -')
    ax4.legend(loc='lower right', framealpha=0.4)
    
    ax4.grid(True)
    
    plt.tight_layout()
    plt.show()
    plt.savefig("output.png")
    
    print("Average X error (meters):", mean_x_error)
    print("Standard deviation of X errors (meters):", std_x_error)
    print("Average Y error (meters):", mean_y_error)
    print("Standard deviation of Y errors (meters):", std_y_error)