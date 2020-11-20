#!/usr/bin/env python3

import socket
import sys
from tkinter import *
from tkinter import ttk
import threading
import re

class CosmicClient:
    def __init__(self,root):
        root.withdraw() #Hide the root window for now and have it reappear once we've connected to the server
        self.conn = Toplevel(root)
        self.conn.title("Textual Cosmic - Connect to server")

        mainframe = ttk.Frame(self.conn, padding="3 3 12 12")
        mainframe.grid(column=0, row=0, sticky=(N, W, E, S))
        self.conn.columnconfigure(0, weight=1)
        self.conn.rowconfigure(0, weight=1)

        self.server_ip = StringVar()
        server_ip_entry = ttk.Entry(mainframe, textvariable=self.server_ip)
        server_ip_entry.grid(column=1,row=0)

        self.server_port = StringVar()
        server_port_entry = ttk.Entry(mainframe, textvariable=self.server_port)
        server_port_entry.grid(column=1,row=1)

        ttk.Button(mainframe, text="Connect", command=self.connect_to_server).grid(column=0, row=2)

        ttk.Label(mainframe, text='Server IP address:').grid(column=0,row=0)
        ttk.Label(mainframe, text='Server Port:').grid(column=0, row=1)

        self.conn.bind("<Return>", self.connect_to_server)

        self.s0 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    def connect_to_server(self,*args):
        self.s0.connect((self.server_ip.get(),int(self.server_port.get())))
        self.conn.destroy()
        root.state('normal')
        root.title("Textual Cosmic")

        #TODO: Add a frame here for organization?
        self.server_log = Text(root, state='disabled', width=80, height=24, wrap='none') #TODO: Do we want to wrap text? TODO: Scrollbars
        self.server_log.grid(column=1,row=0,rowspan=5)

        #TODO: Add a label to tell the user that they have a choice (and what they're choosing?)
        self.client_choice = StringVar()
        self.choice_list = []

        comm_thread = threading.Thread(target=self.server_loop)
        comm_thread.daemon = True #If the user kills the GUI then the server thread should die too
        comm_thread.start()

    def read_message_from_server(self):
        buf = self.s0.recv(4096) #The initial server messages are actually sometimes larger than 1 kB
        return buf.decode("utf-8")

    def send_message_to_server(self,msg):
        self.s0.sendall(msg.encode("utf-8"))

    #TODO: Use asyncio for this event loop?
    def server_loop(self):
        while True:
            buf = self.read_message_from_server()
            self.write_to_server_log(buf)
            needs_response = False
            if buf == 'END':
                break
            elif buf.find('[needs_response]') != -1:
                needs_response = True

            if needs_response:
                print(buf)
                self.convert_options_to_buttons(buf)
                response = input('How would you like to respond?\n')
                self.send_message_to_server(response)
                self.hide_options()
            else:
                print(buf)

        sys.exit(0)

    def write_to_server_log(self,msg):
        #For some reason not all of the text will show up in the server log if we try to jam it all in there at once, hence the line by line insertion
        #I wonder if adding root.update() would help here too? Still, for these functions it's probably best to use a virtual event and let the main thread handle the GUI changes
        self.server_log['state'] = 'normal'
        for line in msg:
            self.server_log.insert('end',line)
        self.server_log['state'] = 'disabled'

    def convert_options_to_buttons(self,buf):
        for line in buf.splitlines():
            #FIXME: The game stack will also trip this regex, ugh
            option_match = re.match('([0-9]): (.*)',line)
            if line.find('[needs_response]') != -1: #This line is delivered after the options
                break
            if option_match:
                option_num = option_match.group(1)
                prompt = option_match.group(2)
                print('Found option ' + option_num + ' with desc ' + prompt)
                self.choice_list.append(ttk.Radiobutton(root, text=prompt, variable=self.client_choice, value=option_num))
                self.choice_list[int(option_num)].grid(column=0,row=int(option_num))
                #TODO: Ensure these options display before we attempt to send a message back to the server!
                #Using root.update helps, but we may need to use something like after or some other loop that executes every n milliseconds...
        #self.radio_select = ttk.Button(root, text='Confirm choice', command=self.send_message_to_server) #TODO: How do we integrate this with the other thread? Perhaps this process even creates a thread?
        root.update()

    def hide_options(self):
        for option in self.choice_list:
            option.grid_forget()
        self.choice_list = []
        root.update()


root = Tk()
CosmicClient(root)
root.mainloop()

