# Joystick
<p align="center">
  <img src = "https://github.com/YttriLab/Joystick/blob/master/Demo/JSGithubDemo.gif">
</p>

<p style="text-align:center;">

The above mouse is performing a self-paced center out joystick task for a fluid reward. Following a successful reach, there is a delay to reward followed by fluid delivery and then an intertrial interval. The task is written in C++ for Arduino and uses Switch Case logic defined by “Event Markers”.  As seen in the left, the Event Marker changes with the task performance as follows:</p>
<p>EventMarker 0 : waiting for reach </p>
<p>EventMarker 1 : delay to reward </p>
<p>EventMarker 2 : fluid delivery </p>
<p>EventMarker 3 : intertrial interval.</p>
<p>Real time joystick position and threshold as well as task performance and parameters can be visualized using a Java based Processing sketch (above left). 
</p>


This work has been published in eNuro, [Belsey et al 2020](https://www.eneuro.org/content/7/2/ENEURO.0523-19.2020).
