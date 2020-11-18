#!/usr/bin/env python2

import socket
import sys
from Tkinter import *
import ttk

class CosmicClient:
    def __init__(self,root):
        root.title("Textual Cosmic")

        mainframe = ttk.Frame(root, padding="3 3 12 12")
        mainframe.grid(column=0, row=0, sticky=(N, W, E, S))
        root.columnconfigure(0, weight=1)
        root.rowconfigure(0, weight=1)

        self.server_ip = StringVar()
        server_ip_entry = ttk.Entry(mainframe, textvariable=self.server_ip)
        server_ip_entry.grid(column=1,row=0)

        self.server_port = StringVar()
        server_port_entry = ttk.Entry(mainframe, textvariable=self.server_port)
        server_port_entry.grid(column=1,row=1)

        ttk.Button(mainframe, text="Connect", command=self.connect_to_server).grid(column=0, row=2)

        ttk.Label(mainframe, text='Server IP address:').grid(column=0,row=0)
        ttk.Label(mainframe, text='Server Port:').grid(column=0, row=1)

        root.bind("<Return>", self.connect_to_server)

        self.s0 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def connect_to_server(self,*args):
        self.s0.connect((self.server_ip.get(),int(self.server_port.get())))
        root.destroy()

        #TODO: Create a new window for the actual game state...we probably don't want to use the root window for making connections either
        #      We can probably use the root window for game state and just hide it until we've connected
        #TODO: Use asyncio to execute this loop
        while True:
            buf = self.read_message_from_server()
            needs_response = False
            if buf == 'END':
                break
            elif buf.find('[needs_response]') != -1:
                needs_response = True

            if needs_response:
                print buf
                response = raw_input('How would you like to respond?\n')
                self.send_message_to_server(response)
            else:
                print buf

        sys.exit(0)

    #In our case the server sends and receives strings directly, so there's no need to use repr() or other methods to change the sent/received data to a string format
    def read_message_from_server(self):
        buf = self.s0.recv(4096) #The initial server messages are actually sometimes larger than 1 kB
        return buf

    def send_message_to_server(self,msg):
        self.s0.sendall(msg)


root = Tk()
CosmicClient(root)
root.mainloop()

