import serial
import serial.tools.list_ports
import matplotlib.pyplot as plt
from matplotlib import backend_bases
import numpy as np
from numpy import random
import os
from datetime import datetime

def on_close(event):
    print("Exiting")
    exit(0)

if __name__ == "__main__":
    ports = []
    print("Select port to connect to:")
    print("0. Exit")
    for i, port in enumerate(serial.tools.list_ports.comports()):
        print(f"{i+1}. {port}")
        ports.append(port)

    if len(ports) == 0:
        print("No ports found. Are you plugged in?")
        exit(1)

    selection = -1
    while selection > len(ports) or selection < 0:
        selection = int(input())
        if selection == 0:
            exit(0)
        elif selection < 0 or selection > len(ports):
            print("Invalid port. Please select a number from the list (i.e. 3).")
            selection = -1

    s = None
    try:
        s = serial.Serial(ports[selection-1].name, 115200, timeout=0)
        print(f"Connecting to {s}")
    except:
        print(f"Failed to connect to port {ports[selection-1].name}")
        exit(1)

    # plt.axis([0, 2048, 0, 1023])

    plt.ion()

    x = np.linspace(0, 2048, num=2048)
    y = np.sin(x)

    # p = fig.add_subplot(111)
    fig, ax = plt.subplots()
    lines, = ax.plot(x, y, 'r-')

    ax.set_ylim(0, 1023)
    ax.set_xlim(0, 2048)

    i = 0

    while True:
        fig.canvas.mpl_connect('close_event', on_close)
        """
        y = np.random.random()
        #p = plt.plot(np.linspace(1, 2048, num=2048), )
        plt.plot()
        plt.pause(0.05)
        i = i+1
        """

        #fig.start_event_loop()

        if s.in_waiting:
            out = s.readline().decode()
            arr = out.split(', ')
            arr = [int(i) for i in arr[0:-1]]
            print(f"Recieved {i}")
            i = i + 1
            ydata = np.array(arr)

            lines.set_ydata(ydata)
            fig.canvas.draw()
            fig.canvas.flush_events()

            # save to file
            with open("RamanLog-"+datetime.now().strftime("%Y-%m-%d-%H%M%S")+".txt", "w") as file:
                for i, val in enumerate(arr):
                    file.write(f"{i+1} {val}\n")

        plt.pause(0.05)
            
