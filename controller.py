import pygame
import sys
import math
import socket
import struct
import time
import platform
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# UDP Setup 
ESP32_IP = "192.168.4.1"
PORT = 4200
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.settimeout(0.01)

# Stick Drift Correction
dead_zone = 30

# killswitch
ks = 0

# weapon scaling
weapon_scale = 0.4  # must be between 0.0 and 1.0

# Detect OS for joystick mapping
if platform.system() == "Linux":
    AXIS_RIGHT_X = 3
    AXIS_RIGHT_Y = 4
    AXIS_LEFT_TRIGGER = 2
    AXIS_RIGHT_TRIGGER = 5
else:
    AXIS_RIGHT_X = 2
    AXIS_RIGHT_Y = 3
    AXIS_LEFT_TRIGGER = 4
    AXIS_RIGHT_TRIGGER = 5

# --- Plot setup ---
channels = ["ch1", "ch2", "ch3"]
values = [1500, 1500, 1500]
fig, ax = plt.subplots()
bars = ax.bar(channels, values, color="skyblue")
ax.set_ylim(1000, 2000)
ax.set_ylabel("Value")
ax.set_title("Live Channel Values")
ks_text = ax.text(0.5, 1.05, "ks: ?", transform=ax.transAxes,
                  ha="center", va="bottom", fontsize=14, fontweight="bold")

# --- UDP send ---
def send_only(values):
    packet = struct.pack('HHHH', *values)
    sock.sendto(packet, (ESP32_IP, PORT))

# --- Gamepad Setup ---
pygame.init()
pygame.joystick.init()
if pygame.joystick.get_count() == 0:
    print("No joystick connected.")
    exit()
joystick = pygame.joystick.Joystick(0)
joystick.init()
print(f"Using joystick: {joystick.get_name()}")

# --- Helper functions ---
def scale_axis(value, flip):
    if flip:
        return 2000 - int((value + 1) * 500)
    else:
        return int((value + 1) * 500 + 1000)

def scale_axis_spinner(value, flip):
    value = (value + 1) / 2
    if flip:
        return 1500 - int(value * 500 * weapon_scale)
    else:
        return 1500 + int(value * 500 * weapon_scale)

def check_dead_zone(a, b):
    a1 = abs(1500 - a)
    b1 = abs(1500 - b)
    if math.sqrt(a1 * a1 + b1 * b1) <= dead_zone:
        return 1500, 1500
    else:
        return a, b

# Store latest joystick data
latest_data = {"ch1": 1500, "ch2": 1500, "ch3": 1500, "lt": 1500, "ks": 0}

# --- Joystick reading loop (runs in background via FuncAnimation) ---
pressed = True
def update(frame):
    global ks, pressed
    # Read joystick
    pygame.event.pump()
    raw_ch1 = joystick.get_axis(AXIS_RIGHT_X)
    raw_ch2 = joystick.get_axis(AXIS_RIGHT_Y)
    raw_ch3 = joystick.get_axis(AXIS_RIGHT_TRIGGER)
    raw_ch4 = joystick.get_axis(AXIS_LEFT_TRIGGER)

    ch1 = scale_axis(raw_ch1, False)
    ch2 = scale_axis(raw_ch2, True)
    ch3 = scale_axis_spinner(raw_ch3, True)
    ch1, ch2 = check_dead_zone(ch1, ch2)

    # Killswitch logic
    if not pressed and raw_ch4 > 0:
        pressed = True
        ks = 2 if ks == 0 else 0
    elif pressed and raw_ch4 == -1.0:
        pressed = False

    # Store for plotting
    latest_data["ch1"] = ch1
    latest_data["ch2"] = ch2
    latest_data["ch3"] = ch3
    latest_data["ks"] = ks

    # Update bars
    bar_values = [latest_data["ch1"], latest_data["ch2"], latest_data["ch3"]]
    for bar, val in zip(bars, bar_values):
        bar.set_height(val)
    
    if ks == 0:
        ks_label = "killswitch: safe mode"
    elif ks == 2:
        ks_label = "killswitch: battle mode"
    else:
        ks_label = f"killswitch: {ks}"  # fallback for other values

    # Update ks label (color based on value)
    ks_color = {0: "green", 1: "orange", 2: "red"}.get(ks, "black")
    ks_text.set_text(ks_label)
    ks_text.set_color(ks_color)

    # Send to ESP32
    send_only([ch1, ch2, ch3, ks])

    return list(bars) + [ks_text]


# Run live plot
ani = FuncAnimation(fig, update, interval=50, blit=False)
plt.show()
