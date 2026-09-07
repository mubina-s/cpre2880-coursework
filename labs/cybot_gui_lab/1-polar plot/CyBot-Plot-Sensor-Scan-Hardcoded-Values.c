# Description: Display using a polar plot the distance measurements collected by a CyBot 180 degree 
# sensor scan at 4 degree increments from 0 to 180 degrees. Data is hard coded into the program.

# Original Code example modified by: Phillip Jones (10/02/2021), (05/15/2023)
# Original polar plot code example from matplot: https://matplotlib.org/stable/gallery/pie_and_polar_charts/polar_demo.html

# Useful matplotlib tutorial: https://matplotlib.org/stable/tutorials/introductory/pyplot.html
# Useful best practices Quick Start: https://matplotlib.org/stable/tutorials/introductory/quick_start.html

# Quick YouTube Overviews (See above links as primary resources for additional details): 
# - Quick Polar Plot (subplot) Overview: https://www.youtube.com/watch?v=pb-pZtvkGPM
# - Quick subplots Overview : https://www.youtube.com/watch?v=Tqph7_qMujk

#Import/Include useful math and plotting functions
import numpy as np
import matplotlib.pyplot as plt

# angle_degrees: a vector (i.e., array of numbers) for which each element is an angle at which the sensor 
# makes a distance measurement.
# arange: used to to store into vector angle_degrees numbers from 0 degrees to 180 degrees, counting by 4's.
# Units: degrees
angle_degrees = np.arange(0,184,4)

# distance: a vector, where each element is the corresponding distance measured at each angle in vector angle_degrees
# Units: meters
distance = [2.5, 2.0, 2.0 ,2.0 ,2.0 ,2.0 ,2.0 ,2.0 ,2.0 ,2.0 ,2.0 ,2.0 ,2.5 ,2.5 ,2.5 ,2.0 ,2.0 ,2.0 ,2.0 ,2.0 ,2.5 ,2.5, 2.5, 
            2.5, 0.5, 0.5, 0.5, 0.5 ,0.5 ,0.5 ,0.5 ,0.5 ,0.5 ,0.5 ,2.5 ,2.5 ,2.5 ,2.5 ,2.5 ,1.5 ,1.5 ,1.5 ,1.5 ,1.5 ,1.5 ,2.5]

# angle_radians: a vector that contains converted elements of vector angle_degrees into radians 
# Units radians
angle_radians = (np.pi/180) * angle_degrees

# Create a polar plot
fig, ax = plt.subplots(subplot_kw={'projection': 'polar'}) # One subplot of type polar
ax.plot(angle_radians, distance, color='r', linewidth=4.0)  # Plot distance verse angle (in radians), using red, line width 4.0
ax.set_xlabel('Distance (m)', fontsize = 14.0)  # Label x axis
ax.set_ylabel('Angle (degrees)', fontsize = 14.0) # Label y axis
ax.xaxis.set_label_coords(0.5, 0.15) # Modify location of x axis label (Typically do not need or want this)
ax.tick_params(axis='both', which='major', labelsize=14) # set font size of tick labels
ax.set_rmax(2.5)                    # Saturate distance at 2.5 meters
ax.set_rticks([0.5, 1, 1.5, 2, 2.5])   # Set plot "distance" tick marks at .5, 1, 1.5, 2, and 2.5 meters
ax.set_rlabel_position(-22.5)     # Adjust location of the radial labels
ax.set_thetamax(180)              # Saturate angle to 180 degrees
ax.set_xticks(np.arange(0,np.pi+.1,np.pi/4)) # Set plot "angle" tick marks to pi/4 radians (i.e., displayed at 45 degree) increments
                                             # Note: added .1 to pi to go just beyond pi (i.e., 180 degrees) so that 180 degrees is displayed
ax.grid(True)                     # Show grid lines

# Create title for plot (font size = 14pt, y & pad controls title vertical location)
ax.set_title("Mock-up Polar Plot of CyBot Sensor Scan from 0 to 180 Degrees", size=14, y=1.0, pad=-24) 
plt.show()  # Display plot
