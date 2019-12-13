# Joystick
<p align="center">
  <img src = "https://github.com/YttriLab/Joystick/blob/master/Demo/JSGithubDemo.gif">
</p>

<p style="text-align:center;">

The above mouse is performing a self-paced center out joystick task for a fluid reward. Following a successful reach, there is a delay to reward followed by fluid delivery and then an intertrial interval. The task is written in C++ for Arduino and uses Switch Case logic defined by “Event Markers”.  As seen in the left, the Event Marker changes with the task performance as follows:</p>
<p>0 : waiting for reach </p>
<p>1 : delay to reward </p>
<p>2 : fluid delivery </p>
<p>3 : intertrial interval.</p>
<p>Real time joystick position and threshold as well as task performance and parameters can be visualized using a Java based Processing sketch (above left). 
</p>
