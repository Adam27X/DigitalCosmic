#!/usr/bin/env python2

import socket
import sys
from Tkinter import *
import ttk
import threading #NOTE: For Python3 we may want asyncio here instead
import Queue

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
        self.queue = Queue.Queue()

    def connect_to_server(self,*args):
        self.s0.connect((self.server_ip.get(),int(self.server_port.get())))
        self.conn.destroy()
        root.state('normal')
        root.title("Textual Cosmic")
        self.server_log = Text(root, state='disabled', width=80, height=24, wrap='none') #TODO: Do we want to wrap text? TODO: Scrollbars
        self.server_log.grid()
        root.bind('<<server_message>>', self.write_to_server_log)

        #Recipe for specifying a string param to a callback function
        #cmd = root.register(self.write_to_server_log)
        #root.tk.call("bind", root, "<<server_message>>", cmd + " %d")

        #TODO: Create a new window for the actual game state...we probably don't want to use the root window for making connections either
        #      We can probably use the root window for game state and just hide it until we've connected
        comm_thread = threading.Thread(target=self.server_loop)
        comm_thread.daemon = True #If the user kills the GUI then the server thread should die too
        comm_thread.start()

    #In our case the server sends and receives strings directly, so there's no need to use repr() or other methods to change the sent/received data to a string format
    def read_message_from_server(self):
        buf = self.s0.recv(4096) #The initial server messages are actually sometimes larger than 1 kB
        return buf

    def send_message_to_server(self,msg):
        self.s0.sendall(msg)

    def server_loop(self):
        while True:
            buf = self.read_message_from_server()
            #NOTE: The queue + event_generate should be better than calling multiple tk commands from this side thread
            #self.write_to_server_log(buf)
            print 'Placing buf into queue: ' + buf
            self.queue.put(buf)
            root.event_generate('<<server_message>>') #TODO: Is there a way to move this generated event outside of this thread?
            #TODO: Figure out why the entire buffer doesn't make it to the text log
            #      It looks like text with blank lines in it messes up stuff here too (we saw something similar in the old C++ client)...try reworking the server to send separate messages instead of messages with blank lines
            #      This also seems to work when responses are needed but not otherwise. Note that the last player to join actually receives fewer messages
            #      The blank spaces could actually be a sign of separate messages that are somehow lost here rather than some issue specific to actual blank lines (indeed a quick test shows no issues with blank lines)
            #TODO: To ensure we don't have any thread safety issues here we can switch to Python3, where Tkinter is thread safe by default
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

    def write_to_server_log(self,*args):
        msg = self.queue.get()
        self.server_log['state'] = 'normal'
        self.server_log.insert('end',msg)
        self.server_log['state'] = 'disabled'


root = Tk()
CosmicClient(root)
root.mainloop()

