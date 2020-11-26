#!/usr/bin/env python3

from tkinter import *
from tkinter import ttk
import time
import threading
import random
import queue
import sys
import socket
import select
import re

class GuiPart(object):
    def __init__(self, master, queue, endCommand, socket):
        self.queue = queue
        self.s0 = socket

        #Initial setup for the game window, hidden until we use it in set_up_main_window
        self.master = master
        self.master.withdraw()

        #Server log
        self.server_log_frame = ttk.Frame(self.master, padding="5 5 5 5")
        self.server_log_frame.grid(column=2, row=1)
        self.server_log_label = Label(self.server_log_frame, text="Server log:")
        self.server_log_label.grid(column=0, row=0)
        self.text = Text(self.server_log_frame, state='disabled', width=50, height=24)
        self.text.grid(column=0,row=1)
        self.master.protocol("WM_DELETE_WINDOW", endCommand)

        #Player choices
        #TODO: Consider using a listbox instead if the number of options can ever be large. A listbox also fits in a specified area (possibly with a scrollbar)
        self.choice_frame = ttk.Frame(self.master, padding="5 5 5 5") #Use a frame to group the options and confirmation button together as one widget in the main window
        self.choice_frame.grid(column=0,row=1)
        self.choice_list = []
        self.client_choice = StringVar()
        self.choice_label_var = StringVar()
        self.choice_label = Label(self.choice_frame, textvariable=self.choice_label_var)
        self.confirmation_button = ttk.Button(self.choice_frame, text='Confirm choice', command=self.hide_options)

        #Player hand
        self.hand_cards = []
        self.hand_cards_wrapper = StringVar(value=self.hand_cards)
        self.hand_disp_label = Label(self.master, text='Player hand:')
        self.hand_disp = Listbox(self.master, height=8, listvariable=self.hand_cards_wrapper) #Height here is the number of lines the box will display without scrolling

        #Display the current turn phase
        self.turn_phase_frame = ttk.Frame(self.master, padding="5 5 5 5")
        self.turn_phase_frame.grid(column=0, columnspan=3, row=0)
        self.turn_phase_labels = []
        self.turn_phase_labels.append(Label(self.turn_phase_frame, text="Start Turn"))
        self.turn_phase_labels.append(Label(self.turn_phase_frame, text="Regroup"))
        self.turn_phase_labels.append(Label(self.turn_phase_frame, text="Destiny"))
        self.turn_phase_labels.append(Label(self.turn_phase_frame, text="Launch"))
        self.turn_phase_labels.append(Label(self.turn_phase_frame, text="Alliance"))
        self.turn_phase_labels.append(Label(self.turn_phase_frame, text="Planning"))
        self.turn_phase_labels.append(Label(self.turn_phase_frame, text="Reveal"))
        self.turn_phase_labels.append(Label(self.turn_phase_frame, text="Resolution"))
        #TODO: Add vertical separators between each phase for clarity?
        for i in range(len(self.turn_phase_labels)):
            self.turn_phase_labels[i].grid(column=i,row=0)
        ttk.Separator(self.turn_phase_frame, orient='horizontal').grid(column=0, columnspan=8, row=1, sticky='ew')
        self.default_label_bg = self.turn_phase_labels[0].cget('bg')

        #Game board
        self.game_board_frame = ttk.Frame(self.master, padding="5 5 5 5")
        self.game_board_frame.grid(column=1, row=1)
        self.warp_width = 400
        self.warp_height = 80
        self.warp_canvas = Canvas(self.game_board_frame, width=self.warp_width, height=self.warp_height, background="orange")
        self.warp_ships = [] #NOTE: It's unclear if this list is even necessary
        self.warp_canvas.grid() #Only object, for now

        #First, bring up the connection window
        self.conn = Toplevel(self.master)
        self.conn.title("Textual Cosmic -- Connect to Server")

        mainframe = ttk.Frame(self.conn, padding="3 3 12 12")
        mainframe.grid(column=0, row=0, sticky=(N,W,E,S))
        self.conn.columnconfigure(0, weight=1)
        self.conn.rowconfigure(0, weight=1)

        #For ease of debug, enter in the default server information
        self.server_ip = StringVar()
        self.server_port = StringVar()
        self.server_ip.set('192.168.0.29')
        self.server_port.set('3074')

        server_ip_entry = ttk.Entry(mainframe, textvariable=self.server_ip)
        server_ip_entry.grid(column=1, row=0)

        server_port_entry = ttk.Entry(mainframe, textvariable=self.server_port)
        server_port_entry.grid(column=1, row=1)

        ttk.Button(mainframe, text="Connect", command=self.set_up_main_window).grid(column=0, row=2)

        ttk.Label(mainframe, text='Server IP address:').grid(column=0, row=0)
        ttk.Label(mainframe, text='Server Port:').grid(column=0, row=1)

        self.conn.bind("<Return>", self.set_up_main_window)
        self.connected = False

    def set_up_main_window(self,*args):
        self.s0.connect((self.server_ip.get(),int(self.server_port.get())))
        self.connected = True
        # Set up the GUI
        # Add more GUI stuff here depending on your specific needs
        self.conn.destroy()
        self.master.state('normal')
        self.master.title("Textual Cosmic")

    def processIncoming(self):
        """ Handle all messages currently in the queue, if any. """
        while self.queue.qsize():
            try:
                msg = self.queue.get(0)
                print('From queue:\n')
                print(msg)
                #Update the server log
                self.text['state'] = 'normal'
                self.text.insert('end',str(msg)+'\n')
                self.text['state'] = 'disabled'
                self.text.see('end') #Focus on the end of the text dump after updating?
                #Process options if there are any
                if msg.find('[needs_response]') != -1:
                    option_num = None
                    self.choice_label_var.set("Please choose one of the following options:")
                    self.choice_label.grid(column=0, row=0)
                    for line in msg.splitlines():
                        option_match = re.match('([0-9]): (.*)',line) #TODO: Should we use [0-9]* here...this currently only works for up to 10 options
                        if line.find('[needs_response]') != -1: #This line is delivered after the options
                            break
                        if option_match:
                            option_num = option_match.group(1)
                            prompt = option_match.group(2)
                            self.choice_list.append(ttk.Radiobutton(self.choice_frame, text=prompt, variable=self.client_choice, value=option_num))
                            option_row = int(option_num)+1
                            self.choice_list[int(option_num)].grid(column=0,row=option_row)
                    if option_num == None:
                        print('ERROR:\n' + msg)
                        raise Exception('A response is required but we failed to find any options!')
                    confirmation_row = int(option_num)+2
                    self.confirmation_button.grid(column=0, row=confirmation_row)
                if msg.find('[player_hand]') != -1: #Update the player's hand
                    hand_found = False
                    for line in msg.splitlines():
                        if line == '[player_hand]':
                            hand_found = True
                            self.hand_cards = []
                            continue
                        if hand_found and len(line) > 0:
                            self.hand_cards.append(line)
                    assert hand_found, "Error processing player hand!"
                    #Anytime we change the list, we need to update the StringVar wrapper
                    self.hand_cards_wrapper.set(self.hand_cards)
                    self.hand_disp_label.grid(column=0, columnspan=3, row=2)
                    self.hand_disp.grid(column=0, columnspan=3, row=3)
                if msg.find('[turn_phase]') != -1: #Update the current turn phase
                    phase_index = None
                    if msg.find('Start') != -1:
                        phase_index = 0
                    elif msg.find('Regroup') != -1:
                        phase_index = 1
                    elif msg.find('Destiny') != -1:
                        phase_index = 2
                    elif msg.find('Launch') != -1:
                        phase_index = 3
                    elif msg.find('Alliance') != -1:
                        phase_index = 4
                    elif msg.find('Planning') != -1: #This covers Planning before and after cards are selected
                        phase_index = 5
                    elif msg.find('Reveal') != -1:
                        phase_index = 6
                    elif msg.find('Resolution') != -1:
                        phase_index = 7
                    assert phase_index is not None, "Error parsing the turn phase!"
                    for i in range(len(self.turn_phase_labels)):
                        if i == phase_index:
                            self.turn_phase_labels[i].config(bg="Orange")
                        else:
                            self.turn_phase_labels[i].config(bg=self.default_label_bg)
                if msg.find('[warp_update]') != -1: #Redraw the warp
                    ships = msg[msg.find('{')+1:msg.find('}')].split(',')
                    #Clear out the previous canvas objects
                    self.warp_canvas.delete("all")
                    self.warp_ships = []
                    if msg.find('{') != -1: #If the warp isn't empty, update it
                        ship_dict = {}
                        for ship in ships:
                            if ship not in ship_dict:
                                ship_dict[ship] = 1
                            else:
                                ship_dict[ship] += 1
                        colorcount = 0
                        for color,num in ship_dict.items():
                            left = colorcount*(self.warp_width/5)
                            right = (colorcount+1)*(self.warp_width/5)
                            center_hor = (left+right)/2
                            top = 0
                            bottom = self.warp_height
                            center_ver = (top+bottom)/2
                            self.warp_ships.append(self.warp_canvas.create_oval(left,top,right,bottom,fill=color,outline='black'))
                            self.warp_ships.append(self.warp_canvas.create_text(center_hor,center_ver,text=str(num),fill='white')) #Is white easier to see here? Can we make the text larger or bold it to make it more prominent?
                            colorcount += 1

            except queue.Empty:
                # just on general principles, although we don't expect this
                # branch to be taken in this case, ignore this exception!
                pass

    def is_connected(self):
        return self.connected

    def send_message_to_server(self,msg):
        self.s0.sendall(msg.encode("utf-8"))

    #NOTE: The GUI can send information to the server directly, so we don't need to send the choice back to the worker thread!
    def hide_options(self):
        if len(self.client_choice.get()) != 0: #Make sure an option was actually selected
            for option in self.choice_list:
                option.grid_forget()
            self.choice_list = []
            self.confirmation_button.grid_forget()
            self.choice_label_var.set("Waiting on other players...")
            self.send_message_to_server(self.client_choice.get())

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
        self.s0 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.gui = GuiPart(master, self.queue, self.endApplication, self.s0)
        # Set up the thread to do asynchronous I/O
        # More threads can also be created and used, if necessary
        self.running = True
        self.thread1 = threading.Thread(target=self.workerThread1)
        self.thread1.daemon = True #Not part of the original plan but this helps kill the thread when the user closes the GUI
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

    def read_message_from_server(self):
        buf = self.s0.recv(4096) #The initial server messages are actually sometimes larger than 1 kB
        return buf.decode("utf-8")

    def send_message_to_server(self,msg):
        self.s0.sendall(msg.encode("utf-8"))

    def workerThread1(self):
        """
        This is where we handle the asynchronous I/O. For example, it may be
        a 'select()'. One important thing to remember is that the thread has
        to yield control pretty regularly, be it by select or otherwise.
        """
        full_msg = ''
        sentinel = "END_MESSAGE\n"
        while self.running:
            #It's unclear how much select actually helps here, but it seems to work and has to be more efficient than just blocking
            #NOTE: This process is a bit paranoid because I was debugging a problem that wound up being a server-side issue where "[needs_response]" was sent in a separate message from the options
            #      With that problem resolved we might be able to just do something like self.queue.put(self.read_message_from_server()) instead of using full_msg and looking for a sentinel here
            if self.gui.is_connected():
                ready_to_read = select.select([self.s0],[],[],1.0) #The last arg here is a timeout in seconds. The longer this time is the more we clog stuff on the GUI
                if len(ready_to_read) != 0: #If we have a message ready, go ahead and read it
                    msg = self.read_message_from_server()
                    full_msg += msg
                    while full_msg.find(sentinel) != -1: #Found the sentinel, put the full message onto the queue. Note that we can extract multiple messages here, so loop until we've handled each sentinel if we find more than one
                        msg_for_queue = full_msg[:full_msg.find(sentinel)]
                        remainder = full_msg[full_msg.find(sentinel)+len(sentinel):]
                        self.queue.put(msg_for_queue)
                        full_msg = remainder

    def endApplication(self):
        self.running = False

rand = random.Random()
root = Tk()
client = ThreadedClient(root)
root.mainloop()

