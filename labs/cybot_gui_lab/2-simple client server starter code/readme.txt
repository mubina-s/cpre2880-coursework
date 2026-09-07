Simple client/server socket starter code

simple-Socket-or-UART-client.c: Simple client socket starter code (must rename from .c to .py)

simple-mock-Cybot-Socket-server.c: Simple server socket starter code (must rename from .c to .py)

simple-CyBot-echo.c: Simple Cybot C program that can communicate with the simple client socket starter code


Suggestions for running the socket program without a CyBot

1) Server: Start the Server program (simple-mock-Cybot-Socket-server.py).
2) Client: Start the Client program (simple-Socket-or-UART-client.py).
3) Echo: Within the Client program’s prompt, type messages to the server. Verify the Server sends the message back to you.
4) Modify Server: Review the Server program. Modify it to be a “Chat” server, instead of an “Echo” server. This should only require changing 1-2 lines of the Server program.
5) Chat: Repeat steps 1) and 2), using your modified Server program. You should be able to chat back and forth between the Client and Server socket programs.


Suggestions for interfacing the socket program with the CyBot

Instead of having the Client communicate with the given Server program, instead have it communicate with your CyBot.

1) Client: Update your client program to use the IP address and Port number that you have been using to set up PuTTY communication.
2) CyBot: Review and use the Simple-CyBot-echo.c C program to communicate between the Client and CyBot. 
3) CyBot: Based on the sample code in 2, modify your Lab 3 CyBot program, as needed, to be compatible with the Client (or you need to modify the Client). Use Lab 3 code or any lab that communicates with the PC.
4) Drive the CyBot: Try using the Client to manually drive your CyBot. In other words, drive the CyBot using your Client program instead of PuTTY.
