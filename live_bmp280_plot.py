import csv
import serial
from collections import deque
from datetime import datetime, timedelta

import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from matplotlib.animation import FuncAnimation
from matplotlib.widgets import Button, CheckButtons

PORT = "COM8"          # change this to your STM32 COM port
BAUD = 115200
CSV_FILE = "bmp280_log.csv"
WINDOW_SECONDS = 60
MOVING_AVG_POINTS = 5   # smooth over last 5 samples

ser = serial.Serial(PORT, BAUD, timeout=1)

times = deque()
temps = deque()
pressures = deque()

paused = False

try:
    with open(CSV_FILE, "x", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["pc_time", "time_ms", "temperature_c", "pressure_hpa"])
except FileExistsError:
    pass

plt.style.use("dark_background")

fig = plt.figure(figsize=(11, 7))
ax1 = fig.add_axes([0.08, 0.58, 0.78, 0.30])
ax2 = fig.add_axes([0.08, 0.14, 0.78, 0.30])

line1, = ax1.plot([], [], linewidth=2.5, label="Temperature")
line2, = ax2.plot([], [], linewidth=2.5, label="Pressure")

ma_line1, = ax1.plot([], [], linestyle="--", linewidth=2.0, alpha=0.85, label="Temp Avg")
ma_line2, = ax2.plot([], [], linestyle="--", linewidth=2.0, alpha=0.85, label="Press Avg")

fill1 = None
fill2 = None

temp_min_marker, = ax1.plot([], [], marker="v", linestyle="None", markersize=8, label="Temp Min")
temp_max_marker, = ax1.plot([], [], marker="^", linestyle="None", markersize=8, label="Temp Max")

press_min_marker, = ax2.plot([], [], marker="v", linestyle="None", markersize=8, label="Press Min")
press_max_marker, = ax2.plot([], [], marker="^", linestyle="None", markersize=8, label="Press Max")

temp_text = ax1.text(
    0.98, 0.92, "",
    transform=ax1.transAxes,
    ha="right", va="top",
    fontsize=11,
    bbox=dict(boxstyle="round", facecolor="black", alpha=0.4, edgecolor="white")
)

press_text = ax2.text(
    0.98, 0.92, "",
    transform=ax2.transAxes,
    ha="right", va="top",
    fontsize=11,
    bbox=dict(boxstyle="round", facecolor="black", alpha=0.4, edgecolor="white")
)

ax1.set_title("Live Temperature", fontsize=14, pad=10)
ax1.set_ylabel("°C", fontsize=11)
ax1.grid(True, alpha=0.25)
ax1.legend(loc="upper left")

ax2.set_title("Live Pressure", fontsize=14, pad=10)
ax2.set_ylabel("hPa", fontsize=11)
ax2.set_xlabel("Time", fontsize=11)
ax2.grid(True, alpha=0.25)
ax2.legend(loc="upper left")

locator = mdates.AutoDateLocator()
formatter = mdates.DateFormatter("%H:%M:%S")
ax1.xaxis.set_major_locator(locator)
ax1.xaxis.set_major_formatter(formatter)
ax2.xaxis.set_major_locator(locator)
ax2.xaxis.set_major_formatter(formatter)

button_ax = fig.add_axes([0.88, 0.82, 0.10, 0.06])
pause_button = Button(button_ax, "Pause")

check_ax = fig.add_axes([0.88, 0.55, 0.10, 0.12])
checks = CheckButtons(check_ax, ["Temp", "Press"], [True, True])

def moving_average(data, window):
    result = []
    for i in range(len(data)):
        start = max(0, i - window + 1)
        chunk = data[start:i + 1]
        result.append(sum(chunk) / len(chunk))
    return result

def toggle_pause(event):
    global paused
    paused = not paused
    pause_button.label.set_text("Resume" if paused else "Pause")

pause_button.on_clicked(toggle_pause)

def toggle_visibility(label):
    if label == "Temp":
        visible = not line1.get_visible()
        line1.set_visible(visible)
        ma_line1.set_visible(visible)
        temp_min_marker.set_visible(visible)
        temp_max_marker.set_visible(visible)
        temp_text.set_visible(visible)
    elif label == "Press":
        visible = not line2.get_visible()
        line2.set_visible(visible)
        ma_line2.set_visible(visible)
        press_min_marker.set_visible(visible)
        press_max_marker.set_visible(visible)
        press_text.set_visible(visible)
    fig.canvas.draw_idle()

checks.on_clicked(toggle_visibility)

def update(frame):
    global fill1, fill2

    if paused:
        return (
            line1, line2, ma_line1, ma_line2,
            temp_min_marker, temp_max_marker,
            press_min_marker, press_max_marker,
            temp_text, press_text
        )

    try:
        raw = ser.readline().decode("utf-8", errors="ignore").strip()
        if raw:
            parts = raw.split(",")
            if len(parts) == 3:
                time_ms = int(parts[0])
                temp_c = float(parts[1])
                pressure_hpa = float(parts[2])

                now = datetime.now()

                times.append(now)
                temps.append(temp_c)
                pressures.append(pressure_hpa)

                with open(CSV_FILE, "a", newline="") as f:
                    writer = csv.writer(f)
                    writer.writerow([now.strftime("%Y-%m-%d %H:%M:%S"), time_ms, temp_c, pressure_hpa])

        cutoff = datetime.now() - timedelta(seconds=WINDOW_SECONDS)
        while times and times[0] < cutoff:
            times.popleft()
            temps.popleft()
            pressures.popleft()

        if not times:
            return (
                line1, line2, ma_line1, ma_line2,
                temp_min_marker, temp_max_marker,
                press_min_marker, press_max_marker,
                temp_text, press_text
            )

        times_list = list(times)
        temps_list = list(temps)
        pressures_list = list(pressures)

        temp_ma = moving_average(temps_list, MOVING_AVG_POINTS)
        press_ma = moving_average(pressures_list, MOVING_AVG_POINTS)

        line1.set_data(times_list, temps_list)
        line2.set_data(times_list, pressures_list)

        ma_line1.set_data(times_list, temp_ma)
        ma_line2.set_data(times_list, press_ma)

        if fill1 is not None:
            fill1.remove()
        if fill2 is not None:
            fill2.remove()

        if len(times_list) > 1:
            fill1 = ax1.fill_between(times_list, temps_list, alpha=0.15)
            fill2 = ax2.fill_between(times_list, pressures_list, alpha=0.15)

        temp_min_idx = temps_list.index(min(temps_list))
        temp_max_idx = temps_list.index(max(temps_list))
        press_min_idx = pressures_list.index(min(pressures_list))
        press_max_idx = pressures_list.index(max(pressures_list))

        temp_min_marker.set_data([times_list[temp_min_idx]], [temps_list[temp_min_idx]])
        temp_max_marker.set_data([times_list[temp_max_idx]], [temps_list[temp_max_idx]])

        press_min_marker.set_data([times_list[press_min_idx]], [pressures_list[press_min_idx]])
        press_max_marker.set_data([times_list[press_max_idx]], [pressures_list[press_max_idx]])

        temp_text.set_text(
            f"Temp: {temps_list[-1]:.2f} °C\n"
            f"Min: {min(temps_list):.2f}\n"
            f"Max: {max(temps_list):.2f}"
        )

        press_text.set_text(
            f"Press: {pressures_list[-1]:.2f} hPa\n"
            f"Min: {min(pressures_list):.2f}\n"
            f"Max: {max(pressures_list):.2f}"
        )

        ax1.relim()
        ax1.autoscale_view()

        ax2.relim()
        ax2.autoscale_view()

        xmin = max(times_list[-1] - timedelta(seconds=WINDOW_SECONDS), times_list[0])
        xmax = times_list[-1] + timedelta(seconds=2)

        ax1.set_xlim(xmin, xmax)
        ax2.set_xlim(xmin, xmax)

    except Exception as e:
        print("Read error:", e)

    return (
        line1, line2, ma_line1, ma_line2,
        temp_min_marker, temp_max_marker,
        press_min_marker, press_max_marker,
        temp_text, press_text
    )

ani = FuncAnimation(fig, update, interval=1000, blit=False)

plt.show()