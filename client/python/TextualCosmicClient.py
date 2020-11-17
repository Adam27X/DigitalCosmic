#!/usr/bin/env python2

import socket
import sys

#In our case the server sends and receives strings directly, so there's no need to use repr() or other methods to change the sent/received data to a string format
def read_message_from_server(s0):
    buf = s0.recv(4096) #The initial server messages are actually sometimes larger than 1 kB
    return buf

def send_message_to_server(s0,msg):
    s0.sendall(msg)

server_ip = raw_input('Enter the IP address of the host server:\n')
server_port = raw_input('Enter the port of the host server:\n')

print 'Connecting to server at ' + server_ip + ':' + server_port

#Create a socket
s0 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s0.connect((server_ip,int(server_port)))

while True:
    buf = read_message_from_server(s0)
    needs_response = False
    if buf == 'END':
        break
    elif buf.find('[needs_response]') != -1:
        needs_response = True

    if needs_response:
        print buf
        response = raw_input('How would you like to respond?\n')
        send_message_to_server(s0,response)
    else:
        print buf

s0.close()

sys.exit(0)
