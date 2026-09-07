"""
Simple mock CyBot server for testing cybot_gui.py without the real robot.
Run this first:
    python mock_cybot_server.py
Then in the GUI connect to:
    127.0.0.1 port 288
"""

import socket
import threading
import time

HOST = "127.0.0.1"
PORT = 288


def send(conn, text):
    conn.sendall(text.replace("\n", "\r\n").encode("ascii", errors="ignore"))


def handle_client(conn, addr):
    print(f"Client connected: {addr}")
    send(conn, "CYBOT MANUAL MODE READY\n")
    try:
        while True:
            data = conn.recv(1024)
            if not data:
                break
            for ch in data.decode(errors="ignore"):
                print("cmd:", repr(ch))
                conn.sendall(ch.encode())  # echo, like UART code
                if ch == "S":
                    send(conn, "\nEVENT:SCAN_SYSTEM_READY\n")
                elif ch == "1":
                    send(conn, "\nEVENT:MOVE_COMPLETE\nNET_MOVEMENT_MM:100.47\n")
                elif ch == "2":
                    send(conn, "\nEVENT:MOVE_COMPLETE\nNET_MOVEMENT_MM:150.21\n")
                elif ch == "3":
                    send(conn, "\nEVENT:MOVE_COMPLETE\nNET_MOVEMENT_MM:250.66\n")
                elif ch == "4":
                    send(conn, "\nEVENT:MOVE_COMPLETE\nNET_MOVEMENT_MM:400.78\n")
                elif ch == "5":
                    send(conn, "\nEVENT:MOVE_COMPLETE\nNET_MOVEMENT_MM:150.10\n")
                elif ch == "8":
                    send(conn, "\nEVENT:TURN_COMPLETE\nTURN_LEFT_DEG:10.85\n")
                elif ch == "9":
                    send(conn, "\nEVENT:TURN_COMPLETE\nTURN_LEFT_DEG:45.43\n")
                elif ch == "0":
                    send(conn, "\nEVENT:TURN_COMPLETE\nTURN_LEFT_DEG:92.16\n")
                elif ch == "w":
                    send(conn, "\nEVENT:TURN_COMPLETE\nTURN_RIGHT_DEG:11.09\n")
                elif ch == "e":
                    send(conn, "\nEVENT:TURN_COMPLETE\nTURN_RIGHT_DEG:46.26\n")
                elif ch == "r":
                    send(conn, "\nEVENT:TURN_COMPLETE\nTURN_RIGHT_DEG:91.81\n")
                elif ch == "t":
                    send(conn, "\nEVENT:STOP\n")
                elif ch == "s":
                    send(conn, "\nGUI_SENSOR,bumper=0,cliff=0,boundary=0,bumpLeft=0,bumpRight=0,cliffLeft=0,cliffFrontLeft=0,cliffFrontRight=0,cliffRight=0,wheelDropLeft=0,wheelDropRight=0\n")
                elif ch == "y":
                    send(conn, "\nEVENT:SCAN180_START,threshold=110\n")
                    time.sleep(0.2)
                    send(conn, "GUI_OBJECT,mid=8,distance=105,width=5.00\n")
                    send(conn, "GUI_OBJECT,mid=45,distance=82,width=42.45\n")
                    send(conn, "GUI_OBJECT,mid=96,distance=103,width=67.07\n")
                    send(conn, "GUI_OBJECT,mid=145,distance=122,width=7.26\n")
                    send(conn, "GUI_OBJECT_COUNT,4\nEVENT:SCAN180_COMPLETE\n")
                elif ch == "u":
                    send(conn, "\nEVENT:SCAN360_START\nEVENT:SCAN360_FRONT_HALF\n")
                    send(conn, "EVENT:SCAN180_START,threshold=110\n")
                    send(conn, "GUI_OBJECT,mid=20,distance=90,width=8.0\n")
                    send(conn, "GUI_OBJECT,mid=82,distance=105,width=55.0\n")
                    send(conn, "GUI_OBJECT_COUNT,2\nEVENT:SCAN180_COMPLETE\n")
                    send(conn, "EVENT:SCAN360_TURNING_180\nEVENT:TURN_COMPLETE\nTURN_RIGHT_DEG:180.51\n")
                    send(conn, "EVENT:SCAN360_BACK_HALF\nEVENT:SCAN180_START,threshold=110\n")
                    send(conn, "GUI_OBJECT,mid=115,distance=70,width=6.0\n")
                    send(conn, "GUI_OBJECT_COUNT,1\nEVENT:SCAN180_COMPLETE\n")
                    send(conn, "EVENT:SCAN360_COMPLETE,total_objects=3,front_objects=2,back_objects=1\n")
                    send(conn, "EVENT:TURN_COMPLETE\nTURN_LEFT_DEG:181.94\n")
    finally:
        print("Client disconnected")
        conn.close()


def main():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server.bind((HOST, PORT))
        server.listen(1)
        print(f"Mock CyBot server listening on {HOST}:{PORT}")
        while True:
            conn, addr = server.accept()
            threading.Thread(target=handle_client, args=(conn, addr), daemon=True).start()


if __name__ == "__main__":
    main()
