#!/usr/bin/env python3

from tkinter import *
from tkinter import ttk
import time
import threading
import random
import queue
import sys

class GuiPart(object):
    def __init__(self, master, queue, endCommand):
        self.queue = queue

        #Initial setup for the game window, hidden until we use it in set_up_main_window
        self.master = master
        self.master.withdraw()
        self.text = Text(self.master, state='disabled', width=80, height=24)
        self.text.grid(column=0,row=0)
        ttk.Button(self.master, text='Done', command=endCommand).grid(column=0,row=1)
        self.master.protocol("WM_DELETE_WINDOW", endCommand)

        #First, bring up the connection window
        self.conn = Toplevel(self.master)
        self.conn.title("Textual Cosmic -- Connect to Server")

        mainframe = ttk.Frame(self.conn, padding="3 3 12 12")
        mainframe.grid(column=0, row=0, sticky=(N,W,E,S))
        self.conn.columnconfigure(0, weight=1)
        self.conn.rowconfigure(0, weight=1)

        server_ip = StringVar()
        server_ip_entry = ttk.Entry(mainframe, textvariable=server_ip)
        server_ip_entry.grid(column=1, row=0)

        server_port = StringVar()
        server_port_entry = ttk.Entry(mainframe, textvariable=server_port)
        server_port_entry.grid(column=1, row=1)

        ttk.Button(mainframe, text="Connect", command=self.set_up_main_window).grid(column=0, row=2)

        ttk.Label(mainframe, text='Server IP address:').grid(column=0, row=0)
        ttk.Label(mainframe, text='Server Port:').grid(column=0, row=1)

        self.conn.bind("<Return>", self.set_up_main_window)

    def set_up_main_window(self,*args):
        self.conn.destroy()
        self.master.state('normal')
        self.master.title("Textual Cosmic")
        # Set up the GUI
        # Add more GUI stuff here depending on your specific needs

    def processIncoming(self):
        """ Handle all messages currently in the queue, if any. """
        while self.queue.qsize():
            try:
                msg = self.queue.get(0)
                # Check contents of message and do whatever is needed. As a
                # simple example, let's print it (in real life, you would
                # suitably update the GUI's display in a richer fashion).
                print(msg)
                self.text['state'] = 'normal'
                self.text.insert('end',str(msg)+'\n')
                self.text['state'] = 'disabled'
            except queue.Empty:
                # just on general principles, although we don't expect this
                # branch to be taken in this case, ignore this exception!
                pass

class ThreadedClient(object):
    """
    Launch the “main” part of the GUI and the worker thread. periodicCall and
    endApplication could reside in the GUI part, but putting them here
    means that you have all the thread controls in a single place.
    """
    def __init__(self, master):
        """
        Start the GUI and the asynchronous threads. We are in the “main”
        (original) thread of the application, which will later be used by
        the GUI as well. We spawn a new thread for the worker (I/O).
        """
        self.master = master
        # Create the queue
        self.queue = queue.Queue()
        # Set up the GUI part
        self.gui = GuiPart(master, self.queue, self.endApplication)
        # Set up the thread to do asynchronous I/O
        # More threads can also be created and used, if necessary
        self.running = True
        self.thread1 = threading.Thread(target=self.workerThread1)
        self.thread1.start()
        # Start the periodic call in the GUI to check the queue
        self.periodicCall()

    def periodicCall(self):
        """ Check every 200 ms if there is something new in the queue. """
        self.gui.processIncoming()
        if not self.running:
            # This is the brutal stop of the system. You may want to do
            # some cleanup before actually shutting it down.
            sys.exit(0)
        #NOTE: The original recipe calls this first, but I think calling it later is slightly better because we can catch errors before recursing
        #      Also, we the 200ms here is really a parameter. The smaller we make it the more resource intensive we make the application
        self.master.after(200, self.periodicCall)

    def workerThread1(self):
        """
        This is where we handle the asynchronous I/O. For example, it may be
        a 'select()'. One important thing to remember is that the thread has
        to yield control pretty regularly, be it by select or otherwise.
        """
        while self.running:
            # To simulate asynchronous I/O, create a random number at random
            # intervals. Replace the following two lines with the real thing.
            time.sleep(rand.random( ) * 1.5)
            msg = rand.random( )
            self.queue.put(msg)

    def endApplication(self):
        self.running = False

rand = random.Random()
root = Tk()
client = ThreadedClient(root)
root.mainloop()

