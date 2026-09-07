import os
import queue
import socket
import threading
import time
import tkinter as tk
from tkinter import filedialog, messagebox, ttk

import numpy as np
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure

DEFAULT_HOST = "192.168.1.1"   # replace in GUI with your CyBot IP
DEFAULT_PORT = 288             # replace in GUI with your CyBot port
SCAN_FILE = os.path.join(os.path.dirname(__file__), "sensor-scan.txt")


class CyBotGui:
    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title("Advanced CyBot GUI")
        self.root.geometry("1200x760")

        self.outgoing: queue.Queue[str] = queue.Queue()
        self.incoming: queue.Queue[tuple[str, object]] = queue.Queue()
        self.stop_event = threading.Event()
        self.network_thread: threading.Thread | None = None
        self.socket_file = None
        self.socket_obj = None

        self.latest_angles = np.array([])
        self.latest_distances = np.array([])

        self.status_var = tk.StringVar(value="Disconnected")
        self.points_var = tk.StringVar(value="Points: 0")
        self.min_var = tk.StringVar(value="Min distance: --")
        self.max_var = tk.StringVar(value="Max distance: --")
        self.last_cmd_var = tk.StringVar(value="Last command: --")

        self.host_var = tk.StringVar(value=DEFAULT_HOST)
        self.port_var = tk.StringVar(value=str(DEFAULT_PORT))
        self.custom_var = tk.StringVar(value="H")

        self._build_ui()
        self.root.after(100, self._process_incoming)
        self.root.protocol("WM_DELETE_WINDOW", self.on_close)

    def _build_ui(self):
        root = self.root

        top = tk.Frame(root)
        top.pack(fill="x", padx=10, pady=8)

        tk.Label(top, text="CyBot IP:").grid(row=0, column=0, sticky="w")
        tk.Entry(top, textvariable=self.host_var, width=16).grid(row=0, column=1, padx=(5, 15))
        tk.Label(top, text="Port:").grid(row=0, column=2, sticky="w")
        tk.Entry(top, textvariable=self.port_var, width=8).grid(row=0, column=3, padx=(5, 15))
        tk.Button(top, text="Connect", width=12, command=self.connect).grid(row=0, column=4, padx=4)
        tk.Button(top, text="Disconnect", width=12, command=self.disconnect).grid(row=0, column=5, padx=4)
        tk.Button(top, text="Request Scan", width=14, command=self.request_scan).grid(row=0, column=6, padx=4)
        tk.Button(top, text="Hello", width=10, command=lambda: self.send_command("H")).grid(row=0, column=7, padx=4)
        tk.Button(top, text="Status", width=10, command=lambda: self.send_command("S")).grid(row=0, column=8, padx=4)

        cmd = tk.Frame(root)
        cmd.pack(fill="x", padx=10, pady=(0, 8))
        tk.Label(cmd, text="Custom command:").pack(side="left")
        tk.Entry(cmd, textvariable=self.custom_var, width=18).pack(side="left", padx=6)
        tk.Button(cmd, text="Send", command=self.send_custom).pack(side="left", padx=4)
        tk.Button(cmd, text="Save Scan As...", command=self.save_scan_as).pack(side="left", padx=4)
        tk.Button(cmd, text="Reload From File", command=self.reload_from_file).pack(side="left", padx=4)
        tk.Button(cmd, text="Clear Log", command=self.clear_log).pack(side="left", padx=4)

        info = tk.Frame(root)
        info.pack(fill="x", padx=10, pady=(0, 8))
        tk.Label(info, textvariable=self.status_var, anchor="w", width=36).pack(side="left")
        tk.Label(info, textvariable=self.last_cmd_var, anchor="w", width=24).pack(side="left")
        tk.Label(info, textvariable=self.points_var, anchor="w", width=16).pack(side="left")
        tk.Label(info, textvariable=self.min_var, anchor="w", width=20).pack(side="left")
        tk.Label(info, textvariable=self.max_var, anchor="w", width=20).pack(side="left")

        body = tk.PanedWindow(root, sashrelief="raised", orient=tk.HORIZONTAL)
        body.pack(fill="both", expand=True, padx=10, pady=8)

        left = tk.Frame(body)
        right = tk.Frame(body)
        body.add(left, minsize=700)
        body.add(right, minsize=350)

        self.figure = Figure(figsize=(7, 5), dpi=100)
        self.ax = self.figure.add_subplot(111, projection="polar")
        self.canvas = FigureCanvasTkAgg(self.figure, master=left)
        self.canvas.get_tk_widget().pack(fill="both", expand=True)
        self._draw_empty_plot()

        table_frame = tk.LabelFrame(left, text="Latest Scan Samples")
        table_frame.pack(fill="both", expand=False, pady=(8, 0))
        columns = ("angle", "distance")
        self.tree = ttk.Treeview(table_frame, columns=columns, show="headings", height=8)
        self.tree.heading("angle", text="Angle (deg)")
        self.tree.heading("distance", text="Distance (m)")
        self.tree.column("angle", width=110, anchor="center")
        self.tree.column("distance", width=110, anchor="center")
        scrollbar = ttk.Scrollbar(table_frame, orient="vertical", command=self.tree.yview)
        self.tree.configure(yscrollcommand=scrollbar.set)
        self.tree.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")

        log_frame = tk.LabelFrame(right, text="Socket / GUI Log")
        log_frame.pack(fill="both", expand=True)
        self.log_box = tk.Text(log_frame, state="disabled", wrap="word")
        self.log_box.pack(fill="both", expand=True)

        notes = tk.LabelFrame(right, text="Demo notes")
        notes.pack(fill="x", pady=(8, 0))
        tips = (
            "1) Put your CyBot IP and port from PuTTY at the top.\n"
            "2) Click Connect.\n"
            "3) Click Hello or Status for quick interaction.\n"
            "4) Click Request Scan to collect real scan data from the CyBot.\n"
            "5) The GUI saves the scan to sensor-scan.txt and plots it automatically."
        )
        tk.Label(notes, text=tips, justify="left", anchor="w").pack(fill="x", padx=8, pady=8)

    def _draw_empty_plot(self):
        self.ax.clear()
        self.ax.set_title("CyBot Polar Scan")
        self.ax.set_rmax(2.5)
        self.ax.set_rticks([0.5, 1.0, 1.5, 2.0, 2.5])
        self.ax.set_rlabel_position(-22.5)
        self.ax.set_thetamax(180)
        self.ax.set_xticks(np.arange(0, np.pi + 0.1, np.pi / 4))
        self.ax.grid(True)
        self.canvas.draw_idle()

    def log(self, text: str):
        timestamp = time.strftime("%H:%M:%S")
        self.log_box.configure(state="normal")
        self.log_box.insert(tk.END, f"[{timestamp}] {text}\n")
        self.log_box.see(tk.END)
        self.log_box.configure(state="disabled")

    def connect(self):
        if self.network_thread and self.network_thread.is_alive():
            self.log("Already connected or connection thread still running.")
            return

        self.stop_event.clear()
        self.network_thread = threading.Thread(target=self._network_worker, daemon=True)
        self.network_thread.start()

    def disconnect(self):
        self.send_command("quit")
        self.stop_event.set()
        self.status_var.set("Disconnecting...")

    def request_scan(self):
        self.send_command("M")

    def send_custom(self):
        cmd = self.custom_var.get().strip()
        if not cmd:
            return
        self.send_command(cmd)

    def send_command(self, command: str):
        command = command.strip()
        if not command:
            return
        self.last_cmd_var.set(f"Last command: {command}")
        self.outgoing.put(command + "\n")

    def save_scan_as(self):
        if not os.path.exists(SCAN_FILE):
            messagebox.showerror("No scan", "No sensor-scan.txt file exists yet.")
            return

        path = filedialog.asksaveasfilename(
            defaultextension=".txt",
            filetypes=[("Text files", "*.txt"), ("All files", "*.*")],
        )
        if not path:
            return

        with open(SCAN_FILE, "r", encoding="utf-8") as src, open(path, "w", encoding="utf-8") as dst:
            dst.write(src.read())
        self.log(f"Saved scan copy to {path}")

    def reload_from_file(self):
        angles, distances = self.parse_scan_file(SCAN_FILE)
        if len(angles) == 0:
            messagebox.showerror("No data", "sensor-scan.txt is missing or empty.")
            return
        self._update_scan_data(angles, distances)
        self.log("Reloaded scan from sensor-scan.txt")

    def clear_log(self):
        self.log_box.configure(state="normal")
        self.log_box.delete("1.0", tk.END)
        self.log_box.configure(state="disabled")

    def parse_scan_file(self, path: str):
        angles = []
        distances = []
        if not os.path.exists(path):
            return np.array([]), np.array([])

        with open(path, "r", encoding="utf-8") as file_obj:
            for raw_line in file_obj:
                line = raw_line.strip()
                if not line:
                    continue
                if line.upper() == "END":
                    break
                if line.lower().startswith("angle"):
                    continue
                parts = line.replace(",", " ").split()
                if len(parts) < 2:
                    continue
                try:
                    angle = float(parts[0])
                    distance = float(parts[1])
                except ValueError:
                    continue
                angles.append(angle)
                distances.append(distance)

        return np.array(angles), np.array(distances)

    def _update_scan_data(self, angles: np.ndarray, distances: np.ndarray):
        self.latest_angles = angles
        self.latest_distances = distances

        self.ax.clear()
        self.ax.plot(np.deg2rad(angles), distances, linewidth=2.5, marker="o", markersize=3)
        self.ax.set_title("CyBot Polar Scan")
        top = max(2.5, float(np.max(distances)) + 0.1)
        self.ax.set_rmax(top)
        self.ax.set_rticks(np.linspace(top / 5, top, 5))
        self.ax.set_rlabel_position(-22.5)
        self.ax.set_thetamax(180)
        self.ax.set_xticks(np.arange(0, np.pi + 0.1, np.pi / 4))
        self.ax.grid(True)
        self.canvas.draw_idle()

        self.points_var.set(f"Points: {len(angles)}")
        self.min_var.set(f"Min distance: {float(np.min(distances)):.3f} m")
        self.max_var.set(f"Max distance: {float(np.max(distances)):.3f} m")

        for item in self.tree.get_children():
            self.tree.delete(item)

        if len(angles) <= 14:
            rows = list(zip(angles, distances))
        else:
            rows = list(zip(angles[:7], distances[:7])) + list(zip(angles[-7:], distances[-7:]))

        for angle, distance in rows:
            self.tree.insert("", tk.END, values=(f"{angle:.0f}", f"{distance:.3f}"))

    def _process_incoming(self):
        while True:
            try:
                kind, payload = self.incoming.get_nowait()
            except queue.Empty:
                break

            if kind == "log":
                self.log(str(payload))
            elif kind == "status":
                self.status_var.set(str(payload))
                self.log(str(payload))
            elif kind == "scan":
                angles, distances = payload
                self._update_scan_data(angles, distances)
                self.status_var.set("Scan complete")
                self.log(f"Loaded {len(angles)} scan points and updated plot.")

        self.root.after(100, self._process_incoming)

    def _network_worker(self):
        host = self.host_var.get().strip()
        try:
            port = int(self.port_var.get().strip())
        except ValueError:
            self.incoming.put(("status", "Port must be an integer."))
            return

        try:
            self.socket_obj = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket_obj.settimeout(10)
            self.socket_obj.connect((host, port))
            self.socket_obj.settimeout(None)
            self.socket_file = self.socket_obj.makefile("rbw", buffering=0)
            self.incoming.put(("status", f"Connected to {host}:{port}"))

            while not self.stop_event.is_set():
                try:
                    send_message = self.outgoing.get(timeout=0.1)
                except queue.Empty:
                    continue

                self.socket_file.write(send_message.encode())
                self.incoming.put(("log", f"Sent -> {send_message.strip()}"))

                if send_message == "quit\n":
                    break

                if send_message.lower() == "m\n":
                    with open(SCAN_FILE, "w", encoding="utf-8") as scan_out:
                        while True:
                            rx = self.socket_file.readline()
                            if not rx:
                                raise ConnectionError("Connection closed while receiving scan data.")
                            line = rx.decode(errors="replace")
                            scan_out.write(line)
                            self.incoming.put(("log", f"RX scan: {line.strip()}"))
                            if line.strip().upper() == "END":
                                break

                    angles, distances = self.parse_scan_file(SCAN_FILE)
                    if len(angles) == 0:
                        self.incoming.put(("status", "Scan received but no numeric data was parsed."))
                    else:
                        self.incoming.put(("scan", (angles, distances)))
                else:
                    rx = self.socket_file.readline()
                    if not rx:
                        raise ConnectionError("Connection closed while waiting for reply.")
                    self.incoming.put(("status", f"Reply: {rx.decode(errors='replace').strip()}"))

        except Exception as error:
            self.incoming.put(("status", f"Error: {error}"))
        finally:
            try:
                if self.socket_file is not None:
                    self.socket_file.close()
            except Exception:
                pass
            try:
                if self.socket_obj is not None:
                    self.socket_obj.close()
            except Exception:
                pass
            self.socket_file = None
            self.socket_obj = None
            self.incoming.put(("log", "Socket closed."))

    def on_close(self):
        self.stop_event.set()
        try:
            self.outgoing.put_nowait("quit\n")
        except Exception:
            pass
        self.root.after(200, self.root.destroy)


def main():
    root = tk.Tk()
    app = CyBotGui(root)
    root.mainloop()


if __name__ == "__main__":
    main()
