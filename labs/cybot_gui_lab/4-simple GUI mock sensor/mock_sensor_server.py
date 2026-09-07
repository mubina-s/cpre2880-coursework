import os
import socket
import time

absolute_path = os.path.dirname(__file__)
filename = os.path.join(absolute_path, "mock-cybot-sensor-scan.txt")

HOST = "127.0.0.1"
PORT = 65432

print("Creating socket")
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind((HOST, PORT))
server_socket.listen()
print(f"Listening on {HOST}:{PORT}")

conn, addr = server_socket.accept()
print(f"Client connected from {addr}")

while True:
    try:
        data = conn.recv(1024)
    except socket.error as error:
        print(f"Connection closed: {error}")
        break

    if not data:
        break

    message = data.decode()
    print(f"Received: {message!r}")

    if message == "quit\n":
        print("Server quitting")
        break

    if message.lower() == "m\n":
        with open(filename, "r") as file_object:
            for line in file_object:
                conn.send(line.encode())
                print(f"Sent: {line.strip()}")
        continue

    conn.send(message.encode())
    print(f"Echoed: {message!r}")

print("Server exiting")
time.sleep(1)
conn.close()
server_socket.close()
