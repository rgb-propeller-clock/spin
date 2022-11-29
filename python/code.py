import time
from adafruit_circuitplayground.express import cpx

# import adafruit_irremote
import pulseio
import board
import array

# Create a 'pulseio' output, to send infrared signals on the IR transmitter @ 38KHz
pulseout = pulseio.PulseOut(board.IR_TX, frequency=38000, duty_cycle=2 ** 7)

while True:
    if cpx.button_b:
        cpx.red_led = True
        for i in range(100):
            pulseout.send(array.array("H", [1000, 1500]))
        cpx.red_led = False
        time.sleep(0.5)
