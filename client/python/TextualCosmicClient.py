#!/usr/bin/env python3

import socket
import sys
from tkinter import *
from tkinter import ttk
import threading

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
        #TODO: Hide the radiobuttons and choice when it isn't the client's turn
        self.client_choice = StringVar()
        self.choice0 = ttk.Radiobutton(root, text='Blue Planet 0', variable=self.client_choice, value='0')
        self.choice1 = ttk.Radiobutton(root, text='Blue Planet 1', variable=self.client_choice, value='1')
        self.choice2 = ttk.Radiobutton(root, text='Blue Planet 2', variable=self.client_choice, value='2')
        self.choice3 = ttk.Radiobutton(root, text='Blue Planet 3', variable=self.client_choice, value='3')
        self.choice4 = ttk.Radiobutton(root, text='Blue Planet 4', variable=self.client_choice, value='4')
        #self.radio_select = ttk.Button(root, text='Confirm choice', command=self.send_message_to_server) #TODO: How do we integrate this with the other thread? Perhaps this process even creates a thread?
        self.choice0.grid(column=0,row=0)
        self.choice1.grid(column=0,row=1)
        self.choice2.grid(column=0,row=2)
        self.choice3.grid(column=0,row=3)
        self.choice4.grid(column=0,row=4)
        self.choice2.grid_forget()

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
                response = input('How would you like to respond?\n')
                self.send_message_to_server(response)
            else:
                print(buf)

        sys.exit(0)

    def write_to_server_log(self,msg,*args):
        #For some reason not all of the text will show up in the server log if we try to jam it all in there at once, hence the line by line insertion
        self.server_log['state'] = 'normal'
        for line in msg:
            self.server_log.insert('end',line)
        self.server_log['state'] = 'disabled'


root = Tk()
CosmicClient(root)
root.mainloop()

