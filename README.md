# Propellor Clock

This repository powers an arduino project consisting of a spinning display of LEDs that shows the current time to the user. Let's see it in action:

![](https://github.com/rgb-propeller-clock/spin/blob/main/propellor_display.gif)

# High Level Architecture
![Propellor_diagrams drawio (14)](https://user-images.githubusercontent.com/47846691/206894422-15a5a0f6-1dc2-4070-9469-f3a7d94a63ba.png)

# Requirements
1. If the start button is pressed when the motor is stopped (1.5 seconds have passed since last full rotation), the motor shall be commanded to begin spinning, approaching a speed of 10 rotations per second as measured using the beam break sensor
2. When the device is spinning steadily, the LEDs shall display an image of the current time in 24-hour hh:mm:ss format
3. If the stop button is pressed at any time, the motor shall be powered off

# Sample Operation
After pressing the on button, the motor speed is set and the device spins up. Once the rotation speed threshold is reached, the LEDs begin to display an image representation of the current time.

A quick summary of how an image is actually displayed is that each time the beam break sensor detects a rotation, if the clock is in the running state, then the beam break ISR starts or adjusts the rate of a timer interrupt, and the timer interrupt updates the LEDs for each column of the image as the clock spins around.

![Propellor_diagrams drawio (13)](https://user-images.githubusercontent.com/47846691/206894760-a541a390-96aa-418b-9b59-aca229215c41.png)

# Finite State Machine (FSM)
![Propellor_diagrams drawio (15)](https://user-images.githubusercontent.com/47846691/206894883-d7a30349-659d-4f78-a0ce-565e341e03e8.png)

# CAD

Onshape CAD for the 3d printed components can be found [here](https://cad.onshape.com/documents/4283ba1e515a79f05b37f05b/w/2211f0aba85311ab91e320af/e/22419e38b691b59c04773b7b?renderMode=0&uiState=63938c81ef86430bb119cc47).
