import os
import queue
import socket
import threading
import tkinter as tk
from tkinter import messagebox

import matplotlib.pyplot as plt
import numpy as np

HOST = "127.0.0.1"   # Change this to your CyBot IP later
PORT = 65432         # Change this to your CyBot port later
SCAN_FILE = os.path.join(os.path.dirname(__file__), "sensor-scan.txt")

outgoing = queue.Queue()
incoming = queue.Queue()
stop_event = threading.Event()


def parse_scan_file(path: str):
    angles = []
    distances = []

    if not os.path.exists(path):
        return np.array([]), np.array([])

    with open(path, "r") as file_object:
        for line in file_object:
            line = line.strip()

            if not line:
                continue
            if line.startswith("Angle("):
                continue
            if line == "END":
                break

            parts = line.split()
            if len(parts) < 2:
                continue

            try:
                angles.append(float(parts[0]))
                distances.append(float(parts[1]))
            except ValueError:
                continue

    return np.array(angles), np.array(distances)


def plot_last_scan():
    angles, distances = parse_scan_file(SCAN_FILE)

    if len(angles) == 0:
        messagebox.showerror("No data", "No scan data found yet. Press Scan first.")
        return

    angle_radians = np.deg2rad(angles)

    fig, ax = plt.subplots(subplot_kw={"projection": "polar"})
    ax.plot(angle_radians, distances, linewidth=3.0)
    ax.set_rmax(2.5)
    ax.set_rticks([0.5, 1.0, 1.5, 2.0, 2.5])
    ax.set_rlabel_position(-22.5)
    ax.set_thetamax(180)
    ax.set_xticks(np.arange(0, np.pi + 0.1, np.pi / 4))
    ax.grid(True)
    ax.set_title("CyBot Sensor Scan")
    plt.show()


def log_message(text: str):
    log_box.configure(state="normal")
    log_box.insert(tk.END, text + "\n")
    log_box.see(tk.END)
    log_box.configure(state="disabled")


def process_incoming():
    while True:
        try:
            _, text = incoming.get_nowait()
        except queue.Empty:
            break

        status_var.set(text)
        log_message(text)

    window.after(100, process_incoming)


def send_hello():
    outgoing.put("Hello\n")


def send_scan():
    outgoing.put("M\n")


def send_quit():
    outgoing.put("quit\n")
    stop_event.set()
    window.after(700, window.destroy)


def network_thread():
    sock = None
    cybot = None

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((HOST, PORT))
        cybot = sock.makefile("rbw", buffering=0)
        incoming.put(("status", f"Connected to {HOST}:{PORT}"))

        while not stop_event.is_set():
            try:
                send_message = outgoing.get(timeout=0.1)
            except queue.Empty:
                continue

            cybot.write(send_message.encode())
            incoming.put(("status", f"Sent: {send_message.strip()}"))

            if send_message == "quit\n":
                break

            if send_message.lower() == "m\n":
                with open(SCAN_FILE, "w") as file_object:
                    while True:
                        rx = cybot.readline()
                        if not rx:
                            incoming.put(("status", "Connection closed while receiving scan data"))
                            stop_event.set()
                            break

                        line = rx.decode()
                        file_object.write(line)

                        if line.strip() == "END":
                            break

                incoming.put(("status", "Scan saved to sensor-scan.txt"))
            else:
                rx = cybot.readline()
                if not rx:
                    incoming.put(("status", "Connection closed"))
                    stop_event.set()
                    break

                incoming.put(("status", f"Reply: {rx.decode().strip()}"))

    except Exception as error:
        incoming.put(("status", f"Error: {error}"))

    finally:
        try:
            if cybot is not None:
                cybot.close()
        except Exception:
            pass

        try:
            if sock is not None:
                sock.close()
        except Exception:
            pass

        incoming.put(("status", "Socket closed"))


def main():
    global window, status_var, log_box

    window = tk.Tk()
    window.title("CyBot GUI")
    window.geometry("560x380")

    tk.Label(window, text="CyBot GUI", font=("Arial", 16, "bold")).pack(pady=10)

    button_frame = tk.Frame(window)
    button_frame.pack(pady=5)

    tk.Button(button_frame, text="Send Hello", width=12, command=send_hello).grid(row=0, column=0, padx=5, pady=5)
    tk.Button(button_frame, text="Scan", width=12, command=send_scan).grid(row=0, column=1, padx=5, pady=5)
    tk.Button(button_frame, text="Plot Last Scan", width=12, command=plot_last_scan).grid(row=0, column=2, padx=5, pady=5)
    tk.Button(button_frame, text="Quit", width=12, command=send_quit).grid(row=0, column=3, padx=5, pady=5)

    status_var = tk.StringVar(value="Starting...")
    tk.Label(window, textvariable=status_var, anchor="w").pack(fill="x", padx=10, pady=5)

    log_box = tk.Text(window, height=14, width=70, state="disabled")
    log_box.pack(fill="both", expand=True, padx=10, pady=10)

    threading.Thread(target=network_thread, daemon=True).start()
    process_incoming()
    window.mainloop()


if __name__ == "__main__":
    main()
