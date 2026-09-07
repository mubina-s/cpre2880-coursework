C PATCH SUMMARY FOR GUI BUTTON REQUIREMENTS

Your current C code already works with the GUI message format. The only mismatch is the exact movement distances requested by the GUI requirements.

Change handleManualCommand() in main.c like this:

Forward buttons:
  '1' -> move_forward_manual(sensor_data, 100);   // 10 cm
  '2' -> move_forward_manual(sensor_data, 150);   // 15 cm
  '3' -> move_forward_manual(sensor_data, 250);   // 25 cm
  '4' -> move_forward_manual(sensor_data, 400);   // 40 cm

Backward button:
  '5' -> move_backward_manual(sensor_data, 150);  // 15 cm

Turns already match the GUI:
  '8' left 10, '9' left 45, '0' left 90
  'w' right 10, 'e' right 45, 'r' right 90

Scan/status/stop already match:
  'S' start, 't' stop, 's' sensor status, 'y' 180 scan, 'u' 360 site scan

I also included main_PATCHED_for_gui.c. You can compare it with your current main.c or replace your main.c with it after backing up your project.
