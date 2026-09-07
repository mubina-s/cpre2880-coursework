"""
CyBot Archaeology Survey Rover GUI
CPRE 288 lab project GUI

What this GUI does:
- Connects to the CyBot TCP socket, usually 192.168.1.1 port 288
- Sends the same one-character commands your C code already uses
- Reads GUI_OBJECT / GUI_SENSOR / EVENT lines from the CyBot
- Draws a field map, robot position, scan objects, possible destination pillars, and emergency warnings

Run:
    python cybot_gui.py

For testing without the real robot:
    python mock_cybot_server.py
    then connect GUI to 127.0.0.1 port 288
"""

from __future__ import annotations

import math
import queue
import re
import socket
import threading
import time
import tkinter as tk
from dataclasses import dataclass
from tkinter import messagebox, scrolledtext, ttk


DEFAULT_HOST = "192.168.1.1"
DEFAULT_PORT = 288

# Field/map settings. Change these if your real field size is different.
FIELD_W_CM = 250.0
FIELD_H_CM = 250.0
CANVAS_W = 720
CANVAS_H = 720
MARGIN = 35


@dataclass
class MapObject:
    x_cm: float
    y_cm: float
    distance_cm: float
    angle_deg: float
    width_cm: float
    kind: str
    seen_at: float


class CyBotGUI(tk.Tk):
    def __init__(self) -> None:
        super().__init__()
        self.title("CyBot Archaeology Survey Rover GUI")
        self.geometry("1220x820")
        self.minsize(1120, 760)

        self.sock: socket.socket | None = None
        self.reader_thread: threading.Thread | None = None
        self.connected = False
        self.rx_queue: queue.Queue[str] = queue.Queue()
        self.stop_reader = threading.Event()

        # Robot pose in cm/degrees. Heading uses math angle: 90 = up/north.
        self.robot_x = 45.0
        self.robot_y = 35.0
        self.robot_heading = 90.0
        self.last_motion_cmd: str | None = None
        self.scan_objects: list[MapObject] = []
        self.map_objects: list[MapObject] = []
        self.current_scan_mode = "idle"
        self.emergency_active = False

        self.buttons_to_disable: list[ttk.Button] = []
        self._build_ui()
        self._draw_map()
        self.after(50, self._process_rx_queue)

    # ---------- UI ----------
    def _build_ui(self) -> None:
        root = ttk.Frame(self, padding=8)
        root.pack(fill=tk.BOTH, expand=True)

        top = ttk.LabelFrame(root, text="Connection")
        top.pack(fill=tk.X, side=tk.TOP)

        ttk.Label(top, text="Host/IP:").pack(side=tk.LEFT, padx=(8, 2), pady=6)
        self.host_var = tk.StringVar(value=DEFAULT_HOST)
        ttk.Entry(top, textvariable=self.host_var, width=18).pack(side=tk.LEFT, padx=2)

        ttk.Label(top, text="Port:").pack(side=tk.LEFT, padx=(8, 2))
        self.port_var = tk.IntVar(value=DEFAULT_PORT)
        ttk.Entry(top, textvariable=self.port_var, width=7).pack(side=tk.LEFT, padx=2)

        self.connect_btn = ttk.Button(top, text="Connect", command=self.connect)
        self.connect_btn.pack(side=tk.LEFT, padx=5)
        self.disconnect_btn = ttk.Button(top, text="Disconnect", command=self.disconnect, state=tk.DISABLED)
        self.disconnect_btn.pack(side=tk.LEFT, padx=5)

        self.status_var = tk.StringVar(value="Not connected")
        self.status_label = ttk.Label(top, textvariable=self.status_var)
        self.status_label.pack(side=tk.LEFT, padx=15)

        self.pose_var = tk.StringVar(value="Pose: x=45.0 cm, y=35.0 cm, heading=90.0°")
        ttk.Label(top, textvariable=self.pose_var).pack(side=tk.RIGHT, padx=10)

        body = ttk.Frame(root)
        body.pack(fill=tk.BOTH, expand=True, pady=(8, 0))

        left = ttk.Frame(body)
        left.pack(side=tk.LEFT, fill=tk.Y, padx=(0, 8))

        controls = ttk.LabelFrame(left, text="CyBot Controls")
        controls.pack(fill=tk.X)

        self.start_btn = self._control_button(controls, "Start / Wake CyBot", lambda: self.send_command("S"), row=0, col=0, colspan=2)
        self.stop_btn = self._control_button(controls, "STOP", lambda: self.send_command("t"), row=0, col=2, colspan=2, disable=False)

        ttk.Label(controls, text="Forward").grid(row=1, column=0, sticky="w", padx=6, pady=(10, 0))
        self._control_button(controls, "10 cm", lambda: self.send_move("1", "forward"), row=2, col=0)
        self._control_button(controls, "15 cm", lambda: self.send_move("2", "forward"), row=2, col=1)
        self._control_button(controls, "25 cm", lambda: self.send_move("3", "forward"), row=2, col=2)
        self._control_button(controls, "40 cm", lambda: self.send_move("4", "forward"), row=2, col=3)

        ttk.Label(controls, text="Backward").grid(row=3, column=0, sticky="w", padx=6, pady=(10, 0))
        self._control_button(controls, "15 cm", lambda: self.send_move("5", "backward"), row=4, col=0)

        ttk.Label(controls, text="Turn Left").grid(row=5, column=0, sticky="w", padx=6, pady=(10, 0))
        self._control_button(controls, "10°", lambda: self.send_command("8"), row=6, col=0)
        self._control_button(controls, "45°", lambda: self.send_command("9"), row=6, col=1)
        self._control_button(controls, "90°", lambda: self.send_command("0"), row=6, col=2)

        ttk.Label(controls, text="Turn Right").grid(row=7, column=0, sticky="w", padx=6, pady=(10, 0))
        self._control_button(controls, "10°", lambda: self.send_command("w"), row=8, col=0)
        self._control_button(controls, "45°", lambda: self.send_command("e"), row=8, col=1)
        self._control_button(controls, "90°", lambda: self.send_command("r"), row=8, col=2)

        ttk.Label(controls, text="Scan / Status").grid(row=9, column=0, sticky="w", padx=6, pady=(10, 0))
        self._control_button(controls, "180 Scan", lambda: self.send_scan("y"), row=10, col=0)
        self._control_button(controls, "Site Scan 360", lambda: self.send_scan("u"), row=10, col=1, colspan=2)
        self._control_button(controls, "Sensor Status", lambda: self.send_command("s"), row=10, col=3, disable=False)

        custom = ttk.LabelFrame(left, text="Custom Raw Command")
        custom.pack(fill=tk.X, pady=(8, 0))
        self.custom_var = tk.StringVar()
        ttk.Entry(custom, textvariable=self.custom_var, width=12).pack(side=tk.LEFT, padx=6, pady=6)
        ttk.Button(custom, text="Send", command=self.send_custom).pack(side=tk.LEFT, padx=4)
        ttk.Label(custom, text="Use only commands your C code accepts.").pack(side=tk.LEFT, padx=8)

        map_tools = ttk.LabelFrame(left, text="Map Tools")
        map_tools.pack(fill=tk.X, pady=(8, 0))
        ttk.Button(map_tools, text="Clear Scan Dots", command=self.clear_scans).pack(fill=tk.X, padx=6, pady=3)
        ttk.Button(map_tools, text="Reset Robot Pose", command=self.reset_pose).pack(fill=tk.X, padx=6, pady=3)
        ttk.Button(map_tools, text="Clear Emergency / Re-enable", command=self.clear_emergency).pack(fill=tk.X, padx=6, pady=3)

        legend = ttk.LabelFrame(left, text="Legend")
        legend.pack(fill=tk.X, pady=(8, 0))
        legend_text = (
            "Black triangle = CyBot\n"
            "Gray dots = normal objects/hazards\n"
            "Yellow dots = possible destination pillars\n"
            "Green zone = start\n"
            "Blue zone = survey/exhibition\n"
            "Gold zone = destination\n"
            "Red ring = emergency/boundary/hole"
        )
        ttk.Label(legend, text=legend_text, justify=tk.LEFT).pack(anchor="w", padx=8, pady=6)

        right = ttk.Frame(body)
        right.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        canvas_frame = ttk.LabelFrame(right, text="Field Map + Data Visualization")
        canvas_frame.pack(fill=tk.BOTH, expand=True)
        self.canvas = tk.Canvas(canvas_frame, width=CANVAS_W, height=CANVAS_H, bg="white")
        self.canvas.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)

        log_frame = ttk.LabelFrame(right, text="CyBot Messages / Debug Log")
        log_frame.pack(fill=tk.X, pady=(8, 0))
        self.log = scrolledtext.ScrolledText(log_frame, height=8, wrap=tk.WORD)
        self.log.pack(fill=tk.X, expand=False, padx=8, pady=8)

    def _control_button(self, parent: ttk.Frame, text: str, command, row: int, col: int, colspan: int = 1, disable: bool = True) -> ttk.Button:
        btn = ttk.Button(parent, text=text, command=command)
        btn.grid(row=row, column=col, columnspan=colspan, sticky="ew", padx=4, pady=4)
        for c in range(4):
            parent.columnconfigure(c, weight=1)
        if disable:
            self.buttons_to_disable.append(btn)
        return btn

    # ---------- Connection ----------
    def connect(self) -> None:
        if self.connected:
            return
        host = self.host_var.get().strip()
        port = int(self.port_var.get())
        try:
            self.sock = socket.create_connection((host, port), timeout=5)
            self.sock.settimeout(0.5)
        except OSError as exc:
            messagebox.showerror("Connection failed", f"Could not connect to {host}:{port}\n\n{exc}")
            return

        self.connected = True
        self.stop_reader.clear()
        self.reader_thread = threading.Thread(target=self._socket_reader, daemon=True)
        self.reader_thread.start()
        self.connect_btn.config(state=tk.DISABLED)
        self.disconnect_btn.config(state=tk.NORMAL)
        self.status_var.set(f"Connected to {host}:{port}")
        self._log(f"[GUI] Connected to {host}:{port}")

    def disconnect(self) -> None:
        self.connected = False
        self.stop_reader.set()
        if self.sock:
            try:
                self.sock.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
            try:
                self.sock.close()
            except OSError:
                pass
        self.sock = None
        self.connect_btn.config(state=tk.NORMAL)
        self.disconnect_btn.config(state=tk.DISABLED)
        self.status_var.set("Disconnected")
        self._log("[GUI] Disconnected")

    def _socket_reader(self) -> None:
        buffer = ""
        while not self.stop_reader.is_set() and self.sock:
            try:
                data = self.sock.recv(4096)
                if not data:
                    self.rx_queue.put("[GUI] Connection closed by server")
                    break
                buffer += data.decode(errors="replace")
                while "\n" in buffer:
                    line, buffer = buffer.split("\n", 1)
                    line = line.strip()
                    if line:
                        self.rx_queue.put(line)
            except socket.timeout:
                continue
            except OSError as exc:
                self.rx_queue.put(f"[GUI] Socket error: {exc}")
                break
        self.rx_queue.put("[GUI] Reader stopped")

    # ---------- Commands ----------
    def send_command(self, cmd: str) -> None:
        if not cmd:
            return
        if not self.sock or not self.connected:
            messagebox.showwarning("Not connected", "Connect to the CyBot first.")
            return
        try:
            self.sock.sendall(cmd.encode("ascii"))
            self._log(f"[GUI -> CYBOT] {cmd}")
        except OSError as exc:
            messagebox.showerror("Send failed", str(exc))
            self.disconnect()

    def send_move(self, cmd: str, direction: str) -> None:
        self.last_motion_cmd = direction
        self.send_command(cmd)

    def send_scan(self, cmd: str) -> None:
        self.current_scan_mode = "scan180" if cmd == "y" else "scan360"
        self.send_command(cmd)

    def send_custom(self) -> None:
        cmd = self.custom_var.get().strip()
        if not cmd:
            return
        for ch in cmd:
            self.send_command(ch)
        self.custom_var.set("")

    # ---------- Parsing ----------
    def _process_rx_queue(self) -> None:
        try:
            while True:
                line = self.rx_queue.get_nowait()
                self._log(line)
                self._parse_line(line)
        except queue.Empty:
            pass
        self.after(50, self._process_rx_queue)

    def _parse_line(self, line: str) -> None:
        if line.startswith("[GUI]"):
            if "Connection closed" in line or "Socket error" in line:
                self.disconnect()
            return

        if line.startswith("EVENT:SCAN180_START"):
            self.scan_objects.clear()
            self.status_var.set("Scanning 180...")
            self._draw_map()
            return

        if line.startswith("EVENT:SCAN360_START"):
            self.status_var.set("Scanning 360...")
            return

        if line.startswith("EVENT:SCAN360_FRONT_HALF"):
            self.status_var.set("360 scan: front half")
            return

        if line.startswith("EVENT:SCAN360_BACK_HALF"):
            self.status_var.set("360 scan: back half")
            return

        if line.startswith("EVENT:SCAN180_COMPLETE"):
            self.status_var.set(f"180 scan complete: {len(self.scan_objects)} object(s)")
            self._draw_map()
            return

        if line.startswith("EVENT:SCAN360_COMPLETE"):
            self.status_var.set("360 site scan complete")
            self._draw_map()
            return

        if line.startswith("GUI_OBJECT"):
            self._parse_gui_object(line)
            return

        if line.startswith("GUI_OBJECT_COUNT"):
            self._draw_map()
            return

        if line.startswith("NET_MOVEMENT_MM"):
            self._parse_movement(line)
            return

        if line.startswith("TURN_LEFT_DEG"):
            self._parse_turn(line, left=True)
            return

        if line.startswith("TURN_RIGHT_DEG"):
            self._parse_turn(line, left=False)
            return

        if line.startswith("GUI_SENSOR"):
            self._parse_sensor(line)
            return

        if line.startswith("EMERGENCY DETECTED"):
            self.emergency_active = True
            self.status_var.set("EMERGENCY DETECTED - buttons disabled")
            self._set_control_buttons_state(False)
            self._draw_map()
            return

        if line.startswith("EVENT:MOVE_COMPLETE") or line.startswith("EVENT:TURN_COMPLETE"):
            self.status_var.set(line.replace("EVENT:", ""))
            return

    def _parse_gui_object(self, line: str) -> None:
        # Expected: GUI_OBJECT,mid=70,distance=99,width=61.19
        parts = dict(re.findall(r"(mid|distance|width)=(-?\d+(?:\.\d+)?)", line))
        if not {"mid", "distance", "width"}.issubset(parts):
            return
        mid = float(parts["mid"])
        distance = float(parts["distance"])
        width = float(parts["width"])

        # Servo: 90 is straight ahead, 0 is right, 180 is left.
        rel = mid - 90.0
        world_angle = self.robot_heading + rel
        rad = math.radians(world_angle)
        x = self.robot_x + distance * math.cos(rad)
        y = self.robot_y + distance * math.sin(rad)

        kind = self._classify_object(width, distance)
        obj = MapObject(x, y, distance, mid, width, kind, time.time())
        self.scan_objects.append(obj)
        self.map_objects.append(obj)
        self._draw_map()

    def _classify_object(self, width: float, distance: float) -> str:
        # This is a simple GUI-side guess only. Your C code is still the real source of truth.
        if 0.0 <= width <= 12.0:
            return "destination_pillar"
        if width >= 30.0:
            return "wide_object"
        return "object"

    def _parse_movement(self, line: str) -> None:
        match = re.search(r"NET_MOVEMENT_MM: *(-?\d+(?:\.\d+)?)", line)
        if not match:
            return
        mm = float(match.group(1))
        cm = abs(mm) / 10.0
        if self.last_motion_cmd == "backward":
            cm = -cm
        self.robot_x += cm * math.cos(math.radians(self.robot_heading))
        self.robot_y += cm * math.sin(math.radians(self.robot_heading))
        self._clamp_robot_to_field()
        self.last_motion_cmd = None
        self.status_var.set(f"Moved {mm:.1f} mm")
        self._update_pose_label()
        self._draw_map()

    def _parse_turn(self, line: str, left: bool) -> None:
        match = re.search(r": *(-?\d+(?:\.\d+)?)", line)
        if not match:
            return
        deg = abs(float(match.group(1)))
        if left:
            self.robot_heading += deg
        else:
            self.robot_heading -= deg
        self.robot_heading %= 360.0
        self._update_pose_label()
        self._draw_map()

    def _parse_sensor(self, line: str) -> None:
        sensor_values = dict(re.findall(r"(\w+)=(-?\d+)", line))
        emergency_keys = [
            "bumper", "cliff", "boundary", "bumpLeft", "bumpRight",
            "cliffLeft", "cliffFrontLeft", "cliffFrontRight", "cliffRight",
            "wheelDropLeft", "wheelDropRight",
        ]
        emergency = any(sensor_values.get(key) == "1" for key in emergency_keys)
        self.emergency_active = emergency
        if emergency:
            self.status_var.set("Emergency/boundary/hole sensor active")
            self._set_control_buttons_state(False)
        else:
            self.status_var.set("Sensors clear")
            self._set_control_buttons_state(True)
        self._draw_map()

    # ---------- Map drawing ----------
    def _cm_to_px(self, x_cm: float, y_cm: float) -> tuple[float, float]:
        width = max(self.canvas.winfo_width(), CANVAS_W)
        height = max(self.canvas.winfo_height(), CANVAS_H)
        scale_x = (width - 2 * MARGIN) / FIELD_W_CM
        scale_y = (height - 2 * MARGIN) / FIELD_H_CM
        scale = min(scale_x, scale_y)
        px = MARGIN + x_cm * scale
        py = height - MARGIN - y_cm * scale
        return px, py

    def _draw_map(self) -> None:
        self.canvas.delete("all")
        w = max(self.canvas.winfo_width(), CANVAS_W)
        h = max(self.canvas.winfo_height(), CANVAS_H)
        scale = min((w - 2 * MARGIN) / FIELD_W_CM, (h - 2 * MARGIN) / FIELD_H_CM)

        # Field boundary
        x0, y0 = self._cm_to_px(0, 0)
        x1, y1 = self._cm_to_px(FIELD_W_CM, FIELD_H_CM)
        self.canvas.create_rectangle(x0, y1, x1, y0, outline="black", width=3)
        self.canvas.create_text((x0+x1)/2, y1-15, text="Archaeology Dig Site Field", font=("Arial", 12, "bold"))

        # Zones. These are default visual guides and can be adjusted later.
        self._draw_zone(10, 10, 55, 55, "Start Zone", "#d7f5d7")
        self._draw_zone(95, 95, 155, 155, "Survey / Exhibition Zone", "#d8ecff")
        self._draw_zone(185, 185, 240, 240, "Destination Zone", "#fff2a8")

        # Grid every 25 cm
        for cm in range(25, int(FIELD_W_CM), 25):
            px_a, py_a = self._cm_to_px(cm, 0)
            px_b, py_b = self._cm_to_px(cm, FIELD_H_CM)
            self.canvas.create_line(px_a, py_a, px_b, py_b, fill="#eeeeee")
            self.canvas.create_text(px_a, y0 + 12, text=str(cm), fill="#888888", font=("Arial", 8))
        for cm in range(25, int(FIELD_H_CM), 25):
            px_a, py_a = self._cm_to_px(0, cm)
            px_b, py_b = self._cm_to_px(FIELD_W_CM, cm)
            self.canvas.create_line(px_a, py_a, px_b, py_b, fill="#eeeeee")
            self.canvas.create_text(x0 + 14, py_a, text=str(cm), fill="#888888", font=("Arial", 8))

        # All stored map objects
        now = time.time()
        for obj in self.map_objects[-80:]:
            age = now - obj.seen_at
            if age > 240:
                continue
            self._draw_object(obj, current=False)

        # Current scan objects are drawn slightly bigger
        for obj in self.scan_objects:
            self._draw_object(obj, current=True)

        self._draw_robot(scale)

        if self.emergency_active:
            rx, ry = self._cm_to_px(self.robot_x, self.robot_y)
            self.canvas.create_oval(rx-32, ry-32, rx+32, ry+32, outline="red", width=5)
            self.canvas.create_text(rx, ry+48, text="EMERGENCY / BOUNDARY / HOLE", fill="red", font=("Arial", 11, "bold"))

    def _draw_zone(self, x1: float, y1: float, x2: float, y2: float, label: str, color: str) -> None:
        px1, py1 = self._cm_to_px(x1, y1)
        px2, py2 = self._cm_to_px(x2, y2)
        self.canvas.create_rectangle(px1, py2, px2, py1, fill=color, outline="#777777", stipple="gray25")
        self.canvas.create_text((px1+px2)/2, (py1+py2)/2, text=label, font=("Arial", 9, "bold"))

    def _draw_object(self, obj: MapObject, current: bool) -> None:
        px, py = self._cm_to_px(obj.x_cm, obj.y_cm)
        r = 7 if current else 5
        if obj.kind == "destination_pillar":
            fill = "#ffd400"
            outline = "#8a6d00"
        elif obj.kind == "wide_object":
            fill = "#777777"
            outline = "black"
        else:
            fill = "#bbbbbb"
            outline = "black"
        self.canvas.create_oval(px-r, py-r, px+r, py+r, fill=fill, outline=outline, width=2)
        if current:
            text = f"{obj.distance_cm:.0f}cm / w{obj.width_cm:.0f}"
            self.canvas.create_text(px+35, py-10, text=text, font=("Arial", 8), anchor="w")

    def _draw_robot(self, scale: float) -> None:
        px, py = self._cm_to_px(self.robot_x, self.robot_y)
        heading = math.radians(self.robot_heading)
        size_cm = 16.0
        front = (self.robot_x + size_cm * math.cos(heading), self.robot_y + size_cm * math.sin(heading))
        left = (self.robot_x + 8 * math.cos(heading + 2.4), self.robot_y + 8 * math.sin(heading + 2.4))
        right = (self.robot_x + 8 * math.cos(heading - 2.4), self.robot_y + 8 * math.sin(heading - 2.4))
        points = []
        for p in (front, left, right):
            points.extend(self._cm_to_px(*p))
        self.canvas.create_polygon(points, fill="black", outline="black")
        self.canvas.create_oval(px-5, py-5, px+5, py+5, fill="white", outline="black")
        self.canvas.create_text(px, py+24, text="CyBot", font=("Arial", 9, "bold"))

        # Draw front scan cone guide
        left_angle = math.radians(self.robot_heading + 90)
        right_angle = math.radians(self.robot_heading - 90)
        cone_len = 110
        lpt = self._cm_to_px(self.robot_x + cone_len * math.cos(left_angle), self.robot_y + cone_len * math.sin(left_angle))
        rpt = self._cm_to_px(self.robot_x + cone_len * math.cos(right_angle), self.robot_y + cone_len * math.sin(right_angle))
        self.canvas.create_line(px, py, lpt[0], lpt[1], fill="#dddddd", dash=(4, 3))
        self.canvas.create_line(px, py, rpt[0], rpt[1], fill="#dddddd", dash=(4, 3))

    # ---------- Helpers ----------
    def _set_control_buttons_state(self, enabled: bool) -> None:
        state = tk.NORMAL if enabled else tk.DISABLED
        for btn in self.buttons_to_disable:
            btn.config(state=state)
        # Stop must always be available while connected.
        self.stop_btn.config(state=tk.NORMAL)

    def clear_scans(self) -> None:
        self.scan_objects.clear()
        self.map_objects.clear()
        self._draw_map()

    def reset_pose(self) -> None:
        self.robot_x = 45.0
        self.robot_y = 35.0
        self.robot_heading = 90.0
        self._update_pose_label()
        self._draw_map()

    def clear_emergency(self) -> None:
        self.emergency_active = False
        self._set_control_buttons_state(True)
        self.status_var.set("Emergency cleared on GUI side")
        self._draw_map()

    def _clamp_robot_to_field(self) -> None:
        self.robot_x = max(0.0, min(FIELD_W_CM, self.robot_x))
        self.robot_y = max(0.0, min(FIELD_H_CM, self.robot_y))

    def _update_pose_label(self) -> None:
        self.pose_var.set(f"Pose: x={self.robot_x:.1f} cm, y={self.robot_y:.1f} cm, heading={self.robot_heading:.1f}°")

    def _log(self, msg: str) -> None:
        timestamp = time.strftime("%H:%M:%S")
        self.log.insert(tk.END, f"{timestamp}  {msg}\n")
        self.log.see(tk.END)

    def on_close(self) -> None:
        self.disconnect()
        self.destroy()


if __name__ == "__main__":
    app = CyBotGUI()
    app.protocol("WM_DELETE_WINDOW", app.on_close)
    app.mainloop()
