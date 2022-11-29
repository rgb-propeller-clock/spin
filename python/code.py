# this program runs on an Adafruit Circuit Python Express board and turns on the onboard IR led when a button is pressed
import time
from adafruit_circuitplayground.express import cpx

# import adafruit_irremote
import pulseio
import board
import array

# Create a 'pulseio' output, to send infrared signals on the IR transmitter @ 38KHz
pulseout = pulseio.PulseOut(board.IR_TX, frequency=38000, duty_cycle=2 ** 7) # duty_cycle reduced from normal for dimmer light to reduce reflection off walls

while True:
    if cpx.button_b:
        cpx.red_led = True
        for i in range(200):
            pulseout.send(array.array("H", [1000, 1500]))
        cpx.red_led = False
        time.sleep(0.5)
