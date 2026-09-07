# CyBot Archaeology Survey Rover GUI

This GUI is made for the CPRE 288 lab project. It works with your current CyBot/C code style where the robot sends text lines through Wi-Fi/UART/TCP.

## What it meets

- GUI buttons for user interaction
- Start / stop control
- Move forward 10, 15, 25, and 40 cm
- Move backward 15 cm
- Turn left/right 10, 45, and 90 degrees
- 180 scan button
- 360 site scan button
- Sensor status button
- Custom raw command input
- Field map with start zone, survey/exhibition zone, and destination zone
- Robot position plotted on the map
- Objects plotted from scan data
- Possible destination pillars highlighted yellow
- Emergency/boundary/hole state shown in red
- Movement/scan buttons disabled during emergency state
- Message log for TA demo/debugging

## Files

```text
gui/cybot_gui.py                Main GUI program
gui/mock_cybot_server.py        Fake CyBot server for testing without robot
c_patch/main_PATCHED_for_gui.c  Patched main.c matching GUI button distances
c_patch/READ_ME_C_PATCH.txt     Small explanation of C changes
```

## C/CyBot side

Your current code already prints the important GUI lines:

```text
GUI_OBJECT,mid=70,distance=99,width=61.19
GUI_OBJECT_COUNT,4
GUI_SENSOR,bumper=0,cliff=1,boundary=1,...
EVENT:MOVE_COMPLETE
NET_MOVEMENT_MM:100.47
EVENT:TURN_COMPLETE
TURN_LEFT_DEG:45.43
```

The GUI parses these exact lines.

### Important patch

Your original `main.c` used:

- `2` = forward 20 cm
- `3` = forward 30 cm
- `5` = backward 10 cm

The GUI/lab requirement wants:

- forward 10, 15, 25, 40 cm
- backward 15 cm

So use the included `c_patch/main_PATCHED_for_gui.c`, or manually change your `handleManualCommand()` distances:

```c
case '1': move_forward_manual(sensor_data, 100); break; // 10 cm
case '2': move_forward_manual(sensor_data, 150); break; // 15 cm
case '3': move_forward_manual(sensor_data, 250); break; // 25 cm
case '4': move_forward_manual(sensor_data, 400); break; // 40 cm
case '5': move_backward_manual(sensor_data, 150); break; // 15 cm back
```

## How to run with the real CyBot

1. Open the C project in Code Composer Studio.
2. Back up your current `main.c`.
3. Replace it with `c_patch/main_PATCHED_for_gui.c`, or manually apply the distance changes.
4. Build the project.
5. Flash/run it on the CyBot.
6. Connect the PC to the CyBot Wi-Fi.
7. Run the GUI:

```bash
cd gui
python cybot_gui.py
```

8. In the GUI, use:

```text
Host/IP: 192.168.1.1
Port:    288
```

9. Click **Connect**.
10. Click **Start / Wake CyBot** once. This sends `S`, because your C code waits for `S` before entering manual mode.
11. Use movement, turn, scan, sensor, and stop buttons.

## How to test without the real CyBot

Open Terminal/PowerShell window 1:

```bash
cd gui
python mock_cybot_server.py
```

Open Terminal/PowerShell window 2:

```bash
cd gui
python cybot_gui.py
```

In the GUI, connect to:

```text
Host/IP: 127.0.0.1
Port:    288
```

Then press Start, move, turn, 180 scan, and 360 site scan. The fake server will send sample messages so you can show/test the GUI logic.

## Simple demo script for TA

1. Connect to CyBot.
2. Press **Start / Wake CyBot**.
3. Press **Sensor Status** and show the GUI log.
4. Press **Forward 10 cm** and show the CyBot position changes on the map.
5. Press **Turn Left 45°** and show robot heading changes.
6. Press **180 Scan** and show object dots appear.
7. Explain yellow dots are possible destination pillars because their width is small.
8. Trigger/show an emergency sensor if possible. The GUI turns red and disables movement/scan buttons.
9. Press **STOP** to show base station control.

## Notes

- The GUI estimates robot location by using movement and turn messages from the CyBot.
- The object map is an approximation, not a perfect SLAM map.
- The field zones are default visual zones. You can edit `FIELD_W_CM`, `FIELD_H_CM`, and the `_draw_zone(...)` calls in `cybot_gui.py` if your real test field has different dimensions.
