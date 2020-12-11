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
        #Give the window some arbitrary starting size so the user doesn't see a bunch of resizing at the start of the program
        self.master.geometry(str(self.master.winfo_screenwidth()-100)+'x'+str(self.master.winfo_screenheight()-100))
        self.master.withdraw()

        #Master frame that contains all subframes, mainly used to add scrolling
        self.container_frame = ttk.Frame(self.master)
        self.container_frame.rowconfigure(0, weight=1)
        self.container_frame.columnconfigure(0, weight=1)
        self.container_canvas = Canvas(self.container_frame)
        self.master.bind('<Configure>', lambda e: self.resize_canvas(e))
        self.master_yscroll = ttk.Scrollbar(self.container_frame, orient='vertical', command=self.container_canvas.yview)
        self.master_xscroll = ttk.Scrollbar(self.container_frame, orient='horizontal', command=self.container_canvas.xview)
        self.master_frame = ttk.Frame(self.container_canvas)
        self.master_frame.bind('<Configure>', lambda e: self.container_canvas.configure(scrollregion=self.container_canvas.bbox('all')))
        self.container_canvas.create_window((0,0), window=self.master_frame, anchor='nw')
        self.container_canvas.configure(yscrollcommand=self.master_yscroll.set, xscrollcommand=self.master_xscroll.set)
        self.container_frame.grid()
        self.container_canvas.grid(column=0, row=0, sticky=(N,S,E,W))
        self.master_yscroll.grid(column=1, row=0, sticky=(N,S))
        self.master_xscroll.grid(column=0, row=1, sticky=(E,W))

        #Server log
        self.server_log_frame = ttk.Frame(self.master_frame, padding="5 5 5 5")
        self.server_log_frame.grid(column=4, row=1)
        self.server_log_label = Label(self.server_log_frame, text="Server log:")
        self.server_log_label.grid(column=0, row=0)
        self.text = Text(self.server_log_frame, state='disabled', width=50, height=24)
        self.text.grid(column=0,row=1)
        self.server_log_scroll = ttk.Scrollbar(self.server_log_frame, orient=VERTICAL, command=self.text.yview)
        self.text['yscrollcommand'] = self.server_log_scroll.set
        self.server_log_scroll.grid(column=1,row=1,sticky=(N,S))
        self.master.protocol("WM_DELETE_WINDOW", endCommand)

        #Player choices
        #TODO: Consider using a listbox instead if the number of options can ever be large. A listbox also fits in a specified area (possibly with a scrollbar)
        self.choice_frame = ttk.Frame(self.master_frame, padding="5 5 5 5") #Use a frame to group the options and confirmation button together as one widget in the main window
        self.choice_frame.grid(column=0,row=1)
        self.choice_list = []
        self.client_choice = StringVar()
        self.choice_label_var = StringVar()
        self.choice_label = Label(self.choice_frame, textvariable=self.choice_label_var)
        self.confirmation_button = ttk.Button(self.choice_frame, text='Confirm choice', command=self.hide_options)

        #Player/Turn info
        self.player_info_frame = ttk.Frame(self.master_frame, padding="5 5 5 5")
        self.player_info_frame.grid(column=0,row=2)
        self.player_color = StringVar()
        self.player_color_label = Label(self.player_info_frame, textvariable=self.player_color)
        self.player_color_canvas = Canvas(self.player_info_frame, width=20, height=20)
        self.alien_name = StringVar()
        self.alien_desc = ''
        self.alien_label = Label(self.player_info_frame, textvariable=self.alien_name)
        self.alien_label.bind('<Enter>', lambda e: self.update_alien_info())
        #TODO: Could add offensive/defensive allies here, but we already have the hyperspace gate and defensive ally displays, hmm
        self.offense_color = StringVar()
        self.offense_color_label = Label(self.player_info_frame, textvariable=self.offense_color)
        self.offense_color_canvas = Canvas(self.player_info_frame, width=20, height=20)
        self.offense_color_label.grid(column=0,row=2)
        self.offense_color_canvas.grid(column=1,row=2)
        self.defense_color = StringVar()
        self.defense_color_label = Label(self.player_info_frame, textvariable=self.defense_color)
        self.defense_color_canvas = Canvas(self.player_info_frame, width=20, height=20)
        self.defense_color_label.grid(column=0,row=3)
        self.defense_color_canvas.grid(column=1,row=3)

        #A box to describe portions of the board that the user is interacting with (cards, aliens, etc.)
        self.description_box_frame = ttk.Frame(self.master_frame, padding="5 5 5 5")
        self.description_box_frame.grid(column=4, row=2)
        self.description_box_label = Label(self.description_box_frame, text='Card/Alien description:')
        self.description_box_label.grid(column=0, row=0)
        self.description_box = Text(self.description_box_frame, state='disabled', width=50, height=12)
        self.description_box.grid(column=0, row=1)
        #TODO: Should probably add a scrollbar in case there's some Alien with a really long description

        #Player hand
        self.hand_frame = ttk.Frame(self.master_frame, padding="5 5 5 5") #Group the label, hand, and scrollbar together
        self.hand_cards = []
        self.hand_cards_wrapper = StringVar(value=self.hand_cards)
        self.hand_disp_label = Label(self.hand_frame, text='Player hand:')
        self.hand_disp = Listbox(self.hand_frame, height=8, listvariable=self.hand_cards_wrapper, selectmode='browse') #Height here is the number of lines the box will display without scrolling; 'browse' indicates only one item can be selected at a time
        self.hand_disp.bind("<<ListboxSelect>>", lambda e: self.update_hand_info(self.hand_disp.curselection(),self.hand_cards))
        self.hand_disp_scroll = ttk.Scrollbar(self.hand_frame, orient=VERTICAL, command=self.hand_disp.yview)
        self.hand_disp['yscrollcommand'] = self.hand_disp_scroll.set
        self.hand_frame.grid(column=3,row=2)

        #Cosmic discard pile
        self.cosmic_discard_frame = ttk.Frame(self.master_frame, padding="5 5 5 5")
        self.cosmic_discard_cards = []
        self.cosmic_discard_cards_wrapper = StringVar(value=self.cosmic_discard_cards)
        self.cosmic_discard_label = Label(self.cosmic_discard_frame, text='Cosmic discard pile:')
        self.cosmic_discard_disp = Listbox(self.cosmic_discard_frame, height=8, listvariable=self.cosmic_discard_cards_wrapper, selectmode='browse')
        self.cosmic_discard_disp.bind("<<ListboxSelect>>", lambda e: self.update_hand_info(self.cosmic_discard_disp.curselection(),self.cosmic_discard_cards))
        self.cosmic_discard_scroll = ttk.Scrollbar(self.cosmic_discard_frame, orient=VERTICAL, command=self.cosmic_discard_disp.yview)
        self.cosmic_discard_disp['yscrollcommand'] = self.cosmic_discard_scroll.set
        self.cosmic_discard_frame.grid(column=2,row=2)

        #Destiny discard pile
        self.destiny_discard_frame = ttk.Frame(self.master_frame, padding="5 5 5 5")
        self.destiny_discard_cards = []
        self.destiny_discard_cards_wrapper = StringVar(value=self.destiny_discard_cards)
        self.destiny_discard_label = Label(self.destiny_discard_frame, text='Cosmic discard pile:')
        self.destiny_discard_disp = Listbox(self.destiny_discard_frame, height=8, listvariable=self.destiny_discard_cards_wrapper, selectmode='browse')
        #TODO: Support destiny card explanations and add the following binding once supported
        #self.destiny_discard_disp.bind("<<ListboxSelect>>", lambda e: self.update_hand_info(self.destiny_discard_disp.curselection(),self.destiny_discard_cards))
        self.destiny_discard_scroll = ttk.Scrollbar(self.destiny_discard_frame, orient=VERTICAL, command=self.destiny_discard_disp.yview)
        self.destiny_discard_disp['yscrollcommand'] = self.destiny_discard_scroll.set
        self.destiny_discard_frame.grid(column=1,row=2)

        #Display the current turn phase
        self.turn_phase_frame = ttk.Frame(self.master_frame, padding="5 5 5 5")
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
        #FIXME: Shrink these elements to be more friendly to lower resolution settings
        self.game_board_frame = ttk.Frame(self.master_frame, padding="5 5 5 5")
        self.game_board_frame.grid(column=1, row=1, columnspan=3)
        self.warp_width = 400
        self.warp_height = 80

        self.warp_canvas = Canvas(self.game_board_frame, width=self.warp_width, height=self.warp_height, background="orange")
        self.warp_ships = []
        Label(self.game_board_frame, text="The Warp:").grid(column=0,row=0)
        self.warp_canvas.grid(column=0,row=1)

        self.hyperspace_gate_canvas = Canvas(self.game_board_frame, width=self.warp_width, height=self.warp_height, background="orange")
        self.hyperspace_gate_ships = []
        Label(self.game_board_frame, text="Hyperspace gate:").grid(column=1,row=0)
        self.hyperspace_gate_canvas.grid(column=1,row=1)

        self.defensive_ally_canvas = Canvas(self.game_board_frame, width=self.warp_width, height=self.warp_height, background="orange") #TODO: Could be neat to make this the same color as the defense?
        self.defensive_ally_ships = []
        Label(self.game_board_frame, text="Defensive ally ships:").grid(column=2,row=0)
        self.defensive_ally_canvas.grid(column=2,row=1)

        self.player_aliens = {} #Dict that maps player color to a tuple of alien information (stringvar,label,desc)

        #Treat player planets and the warp as a similar entity (both are essentially containers for ships)
        self.planet_canvases = []
        self.planets = []
        self.planet_labels = []
        self.planet_labels_written = False

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

    #NOTE: The offset of 20 here is so the scrollbars are visible
    def resize_canvas(self, event):
        if self.master.winfo_width() > 20 and self.master.winfo_height() > 20:
            self.container_canvas.configure(width=self.master.winfo_width()-20, height=self.master.winfo_height()-20)

    def update_source(self, msg, canvas, ship_list):
        ships = msg[msg.find('{')+1:msg.find('}')].split(',')
        #Clear out the previous canvas objects
        canvas.delete("all")
        ship_list = []
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
                ship_list.append(canvas.create_oval(left,top,right,bottom,fill=color,outline='black'))
                ship_list.append(canvas.create_text(center_hor,center_ver,text=str(num),fill='white')) #Is white easier to see here? Can we make the text larger or bold it to make it more prominent?
                bbox = canvas.bbox(ship_list[-1]) #Get a bounding box for the newly created text object
                ship_list.append(canvas.create_rectangle(bbox, fill="black")) #Add a black background to the bounding box
                canvas.tag_raise(ship_list[-2],ship_list[-1]) #Bring the text in front of the background
                colorcount += 1

    #The user clicked on a card in their hand so display what the card does in the description box
    def update_hand_info(self, current_selection, array):
        if len(current_selection) != 1: #The user should only be able to select one item at a time but this protects against the case where no item is selected
            return
        sel = current_selection[0]
        card = array[sel]
        msg = array[sel] + '\n'
        attack_match = re.match('Attack (.*)',card)
        reinforcement_match = re.match('Reinforcement \+(.*)',card)
        if attack_match:
            msg += '[Encounter card] Opposed by attack: Higher total (ships + ' + attack_match.group(1) + ') wins. Opposed by Negotiate: Wins, but opponent collects compensation.'
        elif re.match('Negotiate',card):
            msg += '[Encounter card] Opposed by attack: Loses, but collects compensation. Opposed by Negotiate: Players have one minute to make a deal or lose three ships to the warp.'
        elif re.match('Morph',card):
            msg += '[Encounter card] Duplicates opponent\'s encounter card when revealed.'
        elif reinforcement_match:
            msg += '[Reinforcement card] Adds ' + reinforcement_match.group(1) + ' to either side\'s total. Play after encounter cards are revealed. [Play as a main player or ally only] [Play during the reveal phase only]'
        elif re.match('Card Zap',card):
            msg += '[Artifact card] Negates Cards. Play this card at any time to negate a flare or artifact card just as a player attempts to use it. The flare or artifact must then be discarded. [As any player] [During any turn phase]'
        elif re.match('Cosmic Zap',card):
            msg += '[Artifact card] Stops Power. Play this card at any time to cancel one *use* of any alien\'s power, including your own. That power may not be used again during the current encounter. [As any player] [During any turn phase]'
        elif re.match('Mobius Tubes',card):
            msg += '[Artifact card] Frees Ships. Play at the start of one of your encounters to free all ships from the warp. Freed ships may return to any of their owners\' colonies. [Play as the offense only] [ Play during the regroup phase only]'
        elif re.match('Plague',card):
            msg += '[Artifact card] Harms Player. Play at the start of any encounter and choose a player. That player loses three ships of his or her choice to the warp (if possible) and must discard one card of each type that he or she has in hand (such as attack, negotiate, artifact, flare, etc.). [As any player] [Play during the regroup phase only]'
        elif re.match('Force Field',card):
            msg += '[Artifact card] Stops Allies. Play after alliances are formed during an encounter. You may cancel the alliances of any or all players. Cancelled allies return their ships to any of their colonies. [As any player] [Play during the alliance phase only]'
        elif re.match('Emotion Control',card):
            msg += '[Artifact card] Alters Attack. Play after encounter cards are revealed to treat all attack cards played this encounter as negotiate cards. The main players must then attempt to make a deal. [As any player] [Play during the reveal phase only]'
        elif re.match('Quash',card):
            msg += '[Artifact card] Kills Deal. Play after a deal is made successfully. Cancel the deal, and the dealing players suffer the penalties for a failed deal. [As any player] [Play during the resolution phase only]'
        elif re.match('Ionic Gas',card):
            msg += '[Artifact card] Stops Compensation and Rewards. Play after the winner of an encounter is determined. No compensation or defensive rewards may be collected this encounter. [As any player] [Play during the resolution phase only]'
        self.description_box['state'] = 'normal'
        self.description_box.delete(1.0,'end')
        self.description_box.insert('end',str(msg)+'\n')
        self.description_box['state'] = 'disabled'

    #The user moused over their Alien's name so display what the alien does in the description box
    def update_alien_info(self):
        self.description_box['state'] = 'normal'
        self.description_box.delete(1.0,'end')
        self.description_box.insert('end',str(self.alien_desc)+'\n')
        self.description_box['state'] = 'disabled'

    def update_revealed_alien_info(self, desc):
        self.description_box['state'] = 'normal'
        self.description_box.delete(1.0,'end')
        self.description_box.insert('end',str(desc)+'\n')
        self.description_box['state'] = 'disabled'

    def processIncoming(self):
        """ Handle all messages currently in the queue, if any. """
        while self.queue.qsize():
            try:
                msg = self.queue.get(0)
                print('From queue:\n')
                print(msg)
                #Process options if there are any
                #TODO: Display the number of cards in hand for each opponent?
                #TODO: Make it so that choices involving colonies receive input from the colonies and choices involving cards require submitting a card
                #TODO: Highlight the current planet that is under attack (normally we would point the hyperspace gate at it)
                #TODO: Add GUI support for the number of Tick-Tock tokens remaining
                #TODO: Subclassify diagnostics to improve the UI
                tag_found = False
                if msg.find('[needs_response]') != -1:
                    tag_found = True
                    option_num = None
                    self.choice_label_var.set("Please choose one of the following options:")
                    self.choice_label.grid(column=0, row=0)
                    for line in msg.splitlines():
                        option_match = re.match('([0-9]*): (.*)',line)
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
                    tag_found = True
                    if len(self.player_color.get()) == 0:
                        player_color_match = re.search('Hand for the (.*) player',msg)
                        if player_color_match:
                            self.player_color.set("You are the " + player_color_match.group(1) + " player")
                            self.player_color_label.grid(column=0,row=0)
                            self.player_color_canvas.configure(bg=player_color_match.group(1))
                            self.player_color_canvas.grid(column=1,row=0)
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
                    self.hand_disp_label.grid(column=0, row=0)
                    self.hand_disp.grid(column=0,row=1)
                    self.hand_disp_scroll.grid(column=1,row=1, sticky=(N,S))
                if msg.find('[turn_phase]') != -1: #Update the current turn phase
                    tag_found = True
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
                    tag_found = True
                    self.update_source(msg,self.warp_canvas,self.warp_ships)
                if msg.find('[hyperspace_gate_update]') != -1: #Redraw the hyperspace gate
                    tag_found = True
                    self.update_source(msg,self.hyperspace_gate_canvas,self.hyperspace_gate_ships)
                if msg.find('[defensive_ally_ships_update]') != -1: #Redraw the set of defensive allies
                    tag_found = True
                    self.update_source(msg,self.defensive_ally_canvas,self.defensive_ally_ships)
                if msg.find('[planet_update]') != -1: #Redraw player planets
                    tag_found = True
                    players = msg.split('\n')[1:]
                    players = list(filter(None, players)) #Remove empty entries
                    num_planets = 5
                    if len(self.planet_canvases) == 0: #Draw the canvases for the first time
                        #TODO: We can probably do a better job of organizing this data, but this is a good start
                        for i in range(len(players)):
                            for j in range(num_planets): #Create a row for each planet
                                self.planet_canvases.append(Canvas(self.game_board_frame, width=self.warp_width, height=self.warp_height, background="black")) #TODO: What background to use here? Perhaps an image of stars like space? #TODO: Adjust the width of the canvas to the number of colonies on each planet? Hmm
                                self.planet_canvases[(num_planets*i)+j].grid(column=i,row=3+(2*j)+1)
                    else: #Reset the canvas
                        #Reset the canvases
                        for i in range(len(players)):
                            for j in range(num_planets):
                                self.planet_canvases[(num_planets*i)+j].delete("all")
                        self.planets = []
                    #Fill in the details
                    planet_labels_written = False
                    for i in range(len(players)):
                        for j in range(num_planets):
                            #TODO: Improve on this approach by sending over basic server data such as the number of players and the colors chosen, etc.
                            if len(self.planet_labels) < len(players)*num_planets and players[0].split(' ')[0] != players[len(players)-1].split(' ')[0] and not self.planet_labels_written: #Only fill in the labels the first time we have complete planet information, ugh
                                player = players[i].split(' ')[0]
                                labeltext = player + ' Planet ' + str(j) + ':'
                                self.planet_labels.append(Label(self.game_board_frame,text=labeltext).grid(column=i,row=3+2*j))
                                #Add a label for the player's Alien
                                if j == 0:
                                    self.player_aliens[player] = (StringVar(),)
                                    self.player_aliens[player] += (Label(self.game_board_frame, textvariable=self.player_aliens[player][0]),)
                                    self.player_aliens[player][0].set(player + ' player alien: unrevealed') #Keep the client's Alien in an unrevealed state to let them know that thier alien is still unknown to others (they can see their own alien info elsewhere)
                                    self.player_aliens[player][1].grid(column=i,row=2)
                                planet_labels_written = True
                    if planet_labels_written:
                        self.planet_labels_written = True
                    for i in range(len(players)):
                        player = players[i].split(' ')[0]
                        last_lbrace = players[i].find('{')
                        last_rbrace = 0
                        planet_id = 0
                        while players[i].find('{',last_lbrace+1) != -1:
                            current_planet_lbrace = players[i].find('{',last_lbrace+1)
                            current_planet_rbrace = players[i].find('}',last_rbrace+1)
                            current_planet = players[i][current_planet_lbrace+1:current_planet_rbrace].split(',')
                            if len(players[i][current_planet_lbrace+1:current_planet_rbrace]) == 0: #No ships on this planet, go to the next one
                                last_lbrace = current_planet_lbrace
                                last_rbrace = current_planet_rbrace
                                planet_id += 1
                                continue
                            last_lbrace = current_planet_lbrace
                            last_rbrace = current_planet_rbrace
                            planet_dict = {}
                            for ship in current_planet:
                                if ship not in planet_dict:
                                    planet_dict[ship] = 1
                                else:
                                    planet_dict[ship] += 1
                            colorcount = 0
                            for color,num in planet_dict.items():
                                left = colorcount*(self.warp_width/5)
                                right = (colorcount+1)*(self.warp_width/5)
                                center_hor = (left+right)/2
                                top = 0
                                bottom = self.warp_height
                                center_ver = (top+bottom)/2
                                self.planets.append(self.planet_canvases[(num_planets*i)+planet_id].create_oval(left,top,right,bottom,fill=color,outline='black'))
                                self.planets.append(self.planet_canvases[(num_planets*i)+planet_id].create_text(center_hor,center_ver,text=str(num),fill='white')) #Is white easier to see here? Can we make the text larger or bold it to make it more prominent?
                                bbox = self.planet_canvases[(num_planets*i)+planet_id].bbox(self.planets[-1]) #Get a bounding box for the newly created text object
                                self.planets.append(self.planet_canvases[(num_planets*i)+planet_id].create_rectangle(bbox, fill="black")) #Add a black background to the bounding box
                                self.planet_canvases[(num_planets*i)+planet_id].tag_raise(self.planets[-2],self.planets[-1]) #Bring the text in front of the background
                                colorcount += 1
                            planet_id += 1
                #TODO: Handle PlayerColors::Invalid for offense and defense and clear this info from the GUI when relevant? Not a huge deal because it's obvious when these are valid via turn phase, but this is a "nice to have"
                if msg.find('[offense_update]') != -1: #Update the offense label
                    tag_found = True
                    offense_color_match = re.search('\[offense_update\] (.*)',msg)
                    if offense_color_match:
                        self.offense_color.set("The " + offense_color_match.group(1) + " player is on the offense")
                        self.offense_color_canvas.configure(bg=offense_color_match.group(1))
                    else:
                        raise Exception("Failed to match offense update regex")
                if msg.find('[defense_update]') != -1: #Update the defense label
                    tag_found = True
                    defense_color_match = re.search('\[defense_update\] (.*)',msg)
                    if defense_color_match:
                        self.defense_color.set("The " + defense_color_match.group(1) + " player is on the defense")
                        self.defense_color_canvas.configure(bg=defense_color_match.group(1))
                    else:
                        raise Exception("Failed to match defense update regex")
                if msg.find('[alien_update]') != -1: #Update the player's alien
                    tag_found = True
                    tag = '[alien_update]'
                    alien_name = re.search('===== (.*) =====',msg)
                    assert alien_name, "Failed to parse alien name!"
                    if len(self.alien_name.get()) == 0:
                        self.alien_name.set('Your Alien is ' + alien_name.group(1))
                        self.alien_desc = msg[msg.find(tag)+len(tag):]
                        self.alien_label.grid(column=0,row=1)
                if msg.find('[alien_reveal]') != -1: #A player's alien was revealed, update the alien label
                    tag_found = True
                    alien_match = re.search('The (.*) player is (.*)!',msg)
                    assert alien_match, "Failed to parse alien name from reveal!"
                    player = alien_match.group(1)
                    alien_name = alien_match.group(2)
                    alien_desc = msg[msg.find('!\n')+2:]
                    #TODO: Make this information more prominent once it has been revealed?
                    self.player_aliens[player][0].set(player + ' player alien: ' + alien_name)
                    self.player_aliens[player] += (alien_desc,)
                    self.player_aliens[player][1].bind('<Enter>', lambda e: self.update_revealed_alien_info(self.player_aliens[player][2]))
                if msg.find('[cosmic_discard_update]') != -1: #Cards have moved to or have been removed from the cosmic discard pile
                    tag_found = True
                    lbrace = msg.find('{')
                    rbrace = msg.find('}')
                    cards = msg[lbrace+1:rbrace].split(',')
                    self.cosmic_discard_cards = []
                    for card in cards:
                        self.cosmic_discard_cards.append(card)
                    #Anytime we change the list, we need to update the StringVar wrapper
                    self.cosmic_discard_cards_wrapper.set(self.cosmic_discard_cards)
                    self.cosmic_discard_label.grid(column=0, row=0)
                    self.cosmic_discard_disp.grid(column=0,row=1)
                    self.cosmic_discard_scroll.grid(column=1,row=1, sticky=(N,S))
                if msg.find('[destiny_discard_update]') != -1: #Cards have been removed or added to the destiny discard pile
                    tag_found = True
                    lbrace = msg.find('{')
                    rbrace = msg.find('}')
                    cards = msg[lbrace+1:rbrace].split(',')
                    self.destiny_discard_cards = []
                    for card in cards:
                        self.destiny_discard_cards.append(card)
                    #Anytime we change the list, we need to update the StringVar wrapper
                    self.destiny_discard_cards_wrapper.set(self.destiny_discard_cards)
                    self.destiny_discard_label.grid(column=0, row=0)
                    self.destiny_discard_disp.grid(column=0,row=1)
                    self.destiny_discard_scroll.grid(column=1,row=1, sticky=(N,S))
                if not tag_found:
                    #Update the server log
                    self.text['state'] = 'normal'
                    self.text.insert('end',str(msg)+'\n')
                    self.text['state'] = 'disabled'
                    self.text.see('end') #Focus on the end of the text dump after updating

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

