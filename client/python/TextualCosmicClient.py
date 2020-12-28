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

        #General game information
        self.num_players = None
        self.player_color = '' #Filled in with the client's color
        self.color_to_num = {}
        self.turn_phase = 0

        #Initial setup for the game window, hidden until we use it in set_up_main_window
        self.master = master
        #Give the window some arbitrary starting size so the user doesn't see a bunch of resizing at the start of the program
        #Saturate the window size at 1080p because the screen{width,height} functions accumulate the total space across all monitors
        window_width = min(self.master.winfo_screenwidth(),1920)
        window_height = min(self.master.winfo_screenheight(),1080)
        self.master.geometry(str(window_width-100)+'x'+str(window_height-100))
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
        self.server_log_frame = ttk.Labelframe(self.master_frame, text='Server log', padding="5 5 5 5")
        self.server_log_frame.grid(column=4, row=2, columnspan=2)
        self.text = Text(self.server_log_frame, state='disabled', width=50, height=24)
        self.text.grid(column=0,row=0)
        self.server_log_scroll = ttk.Scrollbar(self.server_log_frame, orient=VERTICAL, command=self.text.yview)
        self.text['yscrollcommand'] = self.server_log_scroll.set
        self.server_log_scroll.grid(column=1,row=0,sticky=(N,S))
        self.master.protocol("WM_DELETE_WINDOW", endCommand)

        #Stack info
        self.stack_info_frame = ttk.Labelframe(self.master_frame, text='Current Stack', padding="5 5 5 5")
        self.stack_info_frame.grid(column=0, row=1)
        #TODO: Consider making this a listbox where the user can click on each stack item for a more detailed description
        #      For instance, the plague item should tell users who was targeted by the plague, etc.
        self.stack_info_text = Text(self.stack_info_frame, state='disabled', width=50, height=6)
        self.stack_info_text.grid(column=0,row=0)

        #Player choices
        #TODO: Consider using a listbox instead if the number of options can ever be large. A listbox also fits in a specified area (possibly with a scrollbar)
        self.choice_frame = ttk.Frame(self.master_frame, padding="5 5 5 5") #Use a frame to group the options and confirmation button together as one widget in the main window
        self.choice_frame.grid(column=0,row=2)
        self.choice_list = []
        self.client_choice = StringVar()
        self.choice_label_var = StringVar()
        self.choice_label_var.set("Waiting on other players...")
        self.choice_label = Label(self.choice_frame, textvariable=self.choice_label_var)
        self.choice_label.grid(column=0, row=0)
        self.confirmation_button = ttk.Button(self.choice_frame, text='Confirm choice', command=self.hide_options)
        self.no_colony_option = ''
        self.no_colony_button = ttk.Button(self.choice_frame, text='Choose no additional ships', command= lambda: self.hide_options_colony('','',''))

        #Player/Turn info
        self.player_info_frame = ttk.Frame(self.master_frame, padding="5 5 5 5")
        self.player_info_frame.grid(column=0,row=3)
        self.player_color_label_var = StringVar()
        self.player_color_label = Label(self.player_info_frame, textvariable=self.player_color_label_var)
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
        self.current_offense_score = StringVar()
        self.current_offense_score_label = Label(self.player_info_frame, textvariable=self.current_offense_score)
        self.current_defense_score = StringVar()
        self.current_defense_score_label = Label(self.player_info_frame, textvariable=self.current_defense_score)

        #A box to describe portions of the board that the user is interacting with (cards, aliens, etc.)
        self.description_box_frame = ttk.Labelframe(self.master_frame, text='Card/Alien description', padding="5 5 5 5")
        self.description_box_frame.grid(column=4, row=3, columnspan=2)
        self.description_box = Text(self.description_box_frame, state='disabled', width=50, height=12)
        self.description_box.grid(column=0, row=0)
        #TODO: Should probably add a scrollbar in case there's some Alien with a really long description

        #Player hand
        self.hand_frame = ttk.Labelframe(self.master_frame, text='Player hand', padding="5 5 5 5") #Group the label, hand, and scrollbar together
        self.hand_cards = []
        self.hand_cards_wrapper = StringVar(value=self.hand_cards)
        self.hand_disp = Listbox(self.hand_frame, height=8, listvariable=self.hand_cards_wrapper, selectmode='browse') #Height here is the number of lines the box will display without scrolling; 'browse' indicates only one item can be selected at a time
        def combined_action(current_selection,hand_cards):
            self.update_hand_info(current_selection,hand_cards)
            self.update_hand_choice(current_selection)
        self.hand_disp.bind("<<ListboxSelect>>", lambda e: combined_action(self.hand_disp.curselection(),self.hand_cards))
        self.hand_disp_scroll = ttk.Scrollbar(self.hand_frame, orient=VERTICAL, command=self.hand_disp.yview)
        self.hand_disp['yscrollcommand'] = self.hand_disp_scroll.set
        self.hand_disp.grid(column=0,row=0)
        self.hand_disp_scroll.grid(column=1,row=0, sticky=(N,S))
        self.play_card_button = ttk.Button(self.hand_frame)
        self.play_alien_button = ttk.Button(self.hand_frame)
        self.next_turn_phase_button = ttk.Button(self.hand_frame)
        self.play_alien_option = ''
        self.hand_options = {} #Map a card in hand to its option num, if it can be used to make a play at this moment
        self.hand_frame.grid(column=1,row=3,columnspan=3)

        #Cosmic discard pile
        self.cosmic_discard_frame = ttk.Labelframe(self.master_frame, text='Cosmic discard pile', padding="5 5 5 5")
        self.cosmic_discard_cards = []
        self.cosmic_discard_cards_wrapper = StringVar(value=self.cosmic_discard_cards)
        self.cosmic_discard_disp = Listbox(self.cosmic_discard_frame, height=8, listvariable=self.cosmic_discard_cards_wrapper, selectmode='browse')
        self.cosmic_discard_disp.bind("<<ListboxSelect>>", lambda e: self.update_hand_info(self.cosmic_discard_disp.curselection(),self.cosmic_discard_cards))
        self.cosmic_discard_scroll = ttk.Scrollbar(self.cosmic_discard_frame, orient=VERTICAL, command=self.cosmic_discard_disp.yview)
        self.cosmic_discard_disp['yscrollcommand'] = self.cosmic_discard_scroll.set
        self.cosmic_discard_disp.grid(column=0,row=0)
        self.cosmic_discard_scroll.grid(column=1,row=0, sticky=(N,S))
        self.cosmic_discard_frame.grid(column=4,row=1)

        #Destiny discard pile
        self.destiny_discard_frame = ttk.Labelframe(self.master_frame, text='Destiny discard pile', padding="5 5 5 5")
        self.destiny_discard_cards = []
        self.destiny_discard_cards_wrapper = StringVar(value=self.destiny_discard_cards)
        self.destiny_discard_disp = Listbox(self.destiny_discard_frame, height=8, listvariable=self.destiny_discard_cards_wrapper, selectmode='browse')
        self.destiny_discard_disp.bind("<<ListboxSelect>>", lambda e: self.update_hand_info(self.destiny_discard_disp.curselection(),self.destiny_discard_cards))
        self.destiny_discard_scroll = ttk.Scrollbar(self.destiny_discard_frame, orient=VERTICAL, command=self.destiny_discard_disp.yview)
        self.destiny_discard_disp['yscrollcommand'] = self.destiny_discard_scroll.set
        self.destiny_discard_disp.grid(column=0,row=0)
        self.destiny_discard_scroll.grid(column=1,row=0, sticky=(N,S))
        self.destiny_discard_frame.grid(column=5,row=1)

        #Display the current turn phase
        self.turn_phase_frame = ttk.Frame(self.master_frame, padding="5 5 5 5")
        self.turn_phase_frame.grid(column=1, columnspan=3, row=0)
        self.turn_phase_labels = []
        self.turn_phase_labels.append(Label(self.turn_phase_frame, text="Start Turn"))
        self.turn_phase_labels.append(Label(self.turn_phase_frame, text="Regroup"))
        self.turn_phase_labels.append(Label(self.turn_phase_frame, text="Destiny"))
        self.turn_phase_labels.append(Label(self.turn_phase_frame, text="Launch"))
        self.turn_phase_labels.append(Label(self.turn_phase_frame, text="Alliance"))
        self.turn_phase_labels.append(Label(self.turn_phase_frame, text="Planning"))
        self.turn_phase_labels.append(Label(self.turn_phase_frame, text="Reveal"))
        self.turn_phase_labels.append(Label(self.turn_phase_frame, text="Resolution"))
        for i in range(len(self.turn_phase_labels)):
            self.turn_phase_labels[i].configure(padx=10)
            self.turn_phase_labels[i].grid(column=i,row=0)
        ttk.Separator(self.turn_phase_frame, orient='horizontal').grid(column=0, columnspan=8, row=1, sticky='ew')
        self.default_label_bg = self.turn_phase_labels[0].cget('bg')

        #Game board
        self.game_board_frame = ttk.Frame(self.master_frame, padding="5 5 5 5")
        self.game_board_frame.grid(column=1, row=1, columnspan=3, rowspan=2)
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
        self.player_hand_size_labels = {} #Dict that maps player to color to labels displaying that player's hand size

        #Treat player planets and the warp as a similar entity (both are essentially containers for ships)
        self.planet_canvases = []
        self.planets = []
        self.planet_labels = []
        self.planet_labels_written = False
        self.colony_bindings = []
        self.planet_bindings = []

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

        #Create the deal window but hide it until it's needed
        self.deal_window = Toplevel(self.master)
        self.deal_window.withdraw()
        self.deal_window.title("Textual Cosmic -- Deal brokering")

        self.offense_to_defense_frame = ttk.Frame(self.deal_window, padding="5 5 5 5")
        self.offense_colony_label = ttk.Label(self.offense_to_defense_frame, text='Potential colonies for the defense to establish')
        self.offense_colony_label.grid(row=0, column=0)
        self.offense_to_defense_colony = StringVar() #Which colony choice was made?
        self.offense_to_defense_colonies = [] #List of colony options
        self.offense_to_defense_frame.grid(row=0, column=0)

        self.defense_to_offense_frame = ttk.Frame(self.deal_window, padding="5 5 5 5")
        self.defense_colony_label = ttk.Label(self.defense_to_offense_frame, text='Potential colonies for the offense to establish')
        self.defense_colony_label.grid(row=0, column=0)
        self.defense_to_offense_colony = StringVar() #Which colony choice was made?
        self.defense_to_offense_colonies = [] #List of colony options
        self.defense_to_offense_frame.grid(row=0, column=1)

        self.deal_confirmation_button = ttk.Button(self.deal_window, text="Propose deal", command=self.propose_deal)
        self.deal_confirmation_button.grid(row=1,column=0,columnspan=2)

        #Another window for accepting another player's proposed deal
        self.deal_acceptance_window = Toplevel(self.master)
        self.deal_acceptance_window.withdraw()
        self.deal_acceptance_window.title("Textual Cosmic -- Deal proposal")
        self.deal_terms = StringVar()
        self.deal_terms_label = Label(self.deal_acceptance_window, textvariable=self.deal_terms)
        self.deal_terms_label.grid(column=0, row=0)
        self.deal_accept_button = ttk.Button(self.deal_acceptance_window, text='Accept', command=self.accept_deal)
        self.deal_accept_button.grid(column=0, row=1)
        self.deal_reject_button = ttk.Button(self.deal_acceptance_window, text='Reject', command=self.reject_deal)
        self.deal_reject_button.grid(column=0, row=2)

    def set_up_main_window(self,*args):
        self.s0.connect((self.server_ip.get(),int(self.server_port.get())))
        self.connected = True
        # Set up the GUI
        # Add more GUI stuff here depending on your specific needs
        self.conn.destroy()
        self.master.state('normal')
        self.master.title("Textual Cosmic")

    def propose_deal(self):
        if len(self.offense_to_defense_colony.get()) == 0  or len(self.defense_to_offense_colony.get()) == 0: #The user hasn't chosen an option on both sides yet
            return
        if self.defense_to_offense_colony.get() == 'None' and self.offense_to_defense_colony.get() == 'None': #The deal must have some sort of exchange
            return
        msg = '[needs_response]\n'
        msg += '[propose_deal]\n'
        msg += 'Offense receives colony: ' + self.defense_to_offense_colony.get() + '\n'
        msg += 'Defense receives colony: ' + self.offense_to_defense_colony.get() + '\n'
        self.send_message_to_server(msg)
        self.deal_window.withdraw()

    def accept_deal(self):
        msg = '[needs_response]\n'
        msg += '[accept_deal]\n'
        msg += self.deal_terms.get()
        self.send_message_to_server(msg)
        self.deal_acceptance_window.withdraw()
        self.deal_window.withdraw() #Remove the proposal window too now that a deal has been made

    def reject_deal(self):
        msg = '[needs_response]\n'
        msg += '[reject_deal]\n'
        self.send_message_to_server(msg)
        self.deal_acceptance_window.withdraw()

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
        def get_destiny_desc(color):
            return '[Destiny card] Have an encounter with the ' + color + ' player in his or her home system. However, if you are the ' + color + ' player, either: A) Have an encounter with any other player in your home system or B) Discard this card and draw again.'
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
        elif re.match('Red',card):
            msg += get_destiny_desc('Red')
        elif re.match('Blue',card):
            msg += get_destiny_desc('Blue')
        elif re.match('Purple',card):
            msg += get_destiny_desc('Purple')
        elif re.match('Yellow',card):
            msg += get_destiny_desc('Yellow')
        elif re.match('Green', card):
            msg += get_destiny_desc('Green')
        elif re.match('Special: Fewest ships in warp', card):
            msg += '[Destiny card] Have an encounter with the player who has the fewest ships in the warp. Break ties to your left.'
        elif re.match('Special: Most cards in hand', card):
            msg += '[Destiny card] Have an encounter with the player who has the most cards in hand. Break ties to your left.'
        elif re.match('Special: Most foreign colonies', card):
            msg += '[Destiny card] Have an encounter with the player who has the most foreign colonies. Break ties to your left.'
        elif re.match('Wild', card):
            msg += '[Destiny card] Have an encounter with a player of your choice in the chosen player\'s home system.'
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
        self.description_box.insert('end',desc+'\n')
        self.description_box['state'] = 'disabled'

    def update_stack_info(self, info):
        self.stack_info_text['state'] = 'normal'
        self.stack_info_text.delete(1.0,'end')
        self.stack_info_text.insert('end',info+'\n')
        self.stack_info_text['state'] = 'disabled'

    def on_colony_click(self, planet_color, planet_num, option_num):
        def hide_options_colony():
            return self.hide_options_colony(planet_color,planet_num,option_num)
        self.confirmation_button.configure(text='Confirm ' + planet_color + ' planet ' + str(planet_num), command=hide_options_colony)
        self.confirmation_button.grid(column=0, row=1)

    def on_planet_click(self, planet_color, planet_num, option_num):
        def hide_options_planet():
            return self.hide_options_planet(planet_color,planet_num,option_num)
        self.confirmation_button.configure(text='Confirm ' + planet_color + ' planet ' + str(planet_num), command=hide_options_planet)
        self.confirmation_button.grid(column=0, row=1)

    def update_hand_choice(self, current_selection):
        if len(current_selection) != 1: #The user should only be able to select one item at a time but this protects against the case where no item is selected
            return
        sel = self.hand_cards[current_selection[0]]
        self.play_card_button.configure(text='Play ' + sel)
        if sel in self.hand_options:
            #Update and display the button for the user to confirm this choice
            self.play_card_button.configure(command=lambda: self.hide_options_empty_stack(self.hand_options[sel]))
        else:
            self.play_card_button.configure(command=lambda: self.hide_options_empty_stack(None))

    def processIncoming(self):
        """ Handle all messages currently in the queue, if any. """
        while self.queue.qsize():
            try:
                msg = self.queue.get(0)
                if ord(msg[0]) == 0: #For some reason the messages tend to start with a null character? Might be a server-side issue, but it's easy enough to deal with here
                    msg = msg[1:]
                print('From queue:')
                print(msg)
                #Process options if there are any
                #TODO: Create a separate window for making deals?
                tag_found = False
                #TODO: Do something neat with the [tick_tock_win_condition] tag
                if msg.find('[needs_response]') != -1:
                    tag_found = True
                    if msg.find('[colony_response]') != -1: #The player needs to choose one of their colonies
                        #TODO: Consider extracting the prompt here as well? There are different reasons to choose a colony, of course. Regroup, plague, mobius tubes, force field, end of a turn for allies, etc.
                        self.choice_label_var.set("Please choose one of your colonies.")
                        #TODO: We could have the user click and drag the planet from the source to the colony; this would require the server to send over the source
                        #Each set of ships (e.g. a canvas oval) is tagged with it's color and planet number. Search through planet canvas to find these tags and when the tag is found, create a binding for when the ship is clicked
                        for line in msg.splitlines():
                            option_match = re.match('([0-9]*): (.*)',line)
                            if line.find('[needs_response]') != -1: #This line is delivered after the options
                                break
                            if option_match:
                                if line.find('None') != -1:
                                    self.no_colony_button.grid(column=0, row=2)
                                    self.no_colony_option = option_match.group(1)
                                else:
                                    colony = option_match.group(2) #Of the form: Blue Planet 2
                                    colony_match = re.match('(.*) Planet ([0-9])',colony)
                                    assert colony_match, "Expected format for colony option!"
                                    planet_color = colony_match.group(1)
                                    planet_num = colony_match.group(2)
                                    #Search the planet canvases to find this planet and then look for the ovals corresponding to the player's color. Can find ovals with the planet_color and planet_num tags and then verify that they're fill color matches self.player_color?
                                    num_planets = 5
                                    for i in range(self.num_players):
                                        for j in range(num_planets):
                                            ship_tag = self.player_color + ' _ships_' + planet_color + '_planet_' + str(planet_num) #Example: Blue_ships_Red_planet_4 means a Blue colony on Red planet 4
                                            if len(self.planet_canvases[(num_planets*i)+j].find_withtag(ship_tag)) != 0:
                                                assert len(self.planet_canvases[(num_planets*i)+j].find_withtag(ship_tag)) == 1, "Found multiple colonies for a single player on a single planet!"
                                                #Add a bind to this object; note the default values in the lambda are used to capture the current values of planet color/num; otherwise the latest value in the interpreter would be used whenever the user clicks
                                                #TODO: Consider highlighting colonies that can be chosen this way...
                                                self.planet_canvases[(num_planets*i)+j].tag_bind(ship_tag, '<ButtonPress-1>', lambda e,planet_color=planet_color,planet_num=planet_num,option_num=option_match.group(1): self.on_colony_click(planet_color, planet_num, option_num))
                                                self.planet_canvases[(num_planets*i)+j].tag_bind(ship_tag, '<Enter>', lambda e, num_planets=num_planets, i=i, j=j: self.planet_canvases[(num_planets*i)+j].configure(cursor='hand2'))
                                                self.planet_canvases[(num_planets*i)+j].tag_bind(ship_tag, '<Leave>', lambda e, num_planets=num_planets, i=i, j=j: self.planet_canvases[(num_planets*i)+j].configure(cursor=''))
                                                self.colony_bindings.append((i,j,ship_tag)) #Bookkeeping for bindings to make it easier to remove them all
                    elif msg.find('[planet_response]') != -1:
                        for line in msg.splitlines():
                            option_match = re.match('([0-9]*): (.*)',line)
                            if line.find('[needs_response]') != -1: #This line is delivered after the options
                                break
                            elif line.find('[planet_response]') != -1:
                                continue
                            else:
                                assert option_match, "Expected an option but didn't find one!"
                                planet = option_match.group(2) #Of the form: Blue Planet 2
                                planet_match = re.match('(.*) Planet ([0-9])',planet)
                                planet_color = planet_match.group(1)
                                planet_num = int(planet_match.group(2))
                                player_id = self.color_to_num[planet_color]
                                num_planets = 5
                                self.planet_canvases[(num_planets*player_id)+planet_num].bind('<ButtonPress-1>', lambda e,planet_color=planet_color,planet_num=planet_num,option_num=option_match.group(1): self.on_planet_click(planet_color, planet_num, option_num))
                                self.planet_canvases[(num_planets*player_id)+planet_num].bind('<Enter>', lambda e, num_planets=num_planets,player_id=player_id,planet_num=planet_num: self.planet_canvases[(num_planets*player_id)+planet_num].configure(cursor='hand2'))
                                self.planet_canvases[(num_planets*player_id)+planet_num].bind('<Leave>', lambda e, num_planets=num_planets,player_id=player_id,planet_num=planet_num: self.planet_canvases[(num_planets*player_id)+planet_num].configure(cursor=''))
                                self.planet_bindings.append((player_id,planet_num))
                        #FIXME: We have two similar but not identical contexts here: 1) choosing a planet to attack and 2) choosing where to spawn a new colony upon a successful deal. For the first case we can use planet_color to clarify the label. For the second case we would need an additional tag and the color of the other player in the deal
                        self.choice_label_var.set("Please choose a planet.")
                    elif msg.find('[stack_response]') != -1:
                        #The client can either pass the turn, play a card from his or her hand, or use his or her alien power
                        #Have one button with the option to proceed to the next turn, have a second for the Alien power, if it's available, and a third to choose the selected card in hand
                        #TODO: Should we highlight the cards that the player can use here?
                        self.choice_label_var.set("It's your action. You may play a card from your hand,\nuse your alien power, or proceed to the next turn phase.")
                        #For each option, find the corresponding card and add a binding such that clicking on the card sets a button that casts the card upon confirmation
                        #If one of the options is an alien power, provide another button for that use case
                        for line in msg.splitlines():
                            option_match = re.match('([0-9]*): (.*)',line)
                            if line.find('[needs_response]') != -1: #This line is delivered after the options
                                break
                            elif line.find('[stack_response]') != -1:
                                continue
                            elif option_match: #There are other diagnostic lines here, should we spit them out to the server log in an else clause?
                                play = option_match.group(2)
                                #Checking explicitly that this string occurs at the beginning of the option such that we don't match something like 'None (Resolve Alien Power)'
                                if re.match('Alien Power',play):
                                    mandatory = (play.find('mandatory') != -1)
                                    self.play_alien_option = option_match.group(1)
                                    config_text = 'Use alien power'
                                    if mandatory:
                                        config_text += ' (mandatory)'
                                    #FIXME: A separate variable for play_alien_option might not be necessary here after all
                                    self.play_alien_button.configure(text=config_text, command= lambda: self.hide_options_empty_stack(self.play_alien_option))
                                    self.play_alien_button.grid(column=0, row=2)
                                elif play.find('None') != -1:
                                    self.next_turn_phase_button.configure(text='Continue to ' + self.get_next_turn_phase(), command= lambda option_num=option_match.group(1): self.hide_options_empty_stack(option_num))
                                    #TODO: Consider not displaying this button if we find a mandatory alien power?
                                    self.next_turn_phase_button.grid(column=0, row=3)
                                else: #Should correspond to a card in hand
                                    found_corresponding_card = False
                                    for card in self.hand_cards:
                                        if card == play:
                                            found_corresponding_card = True
                                            self.hand_options[card] = option_match.group(1)
                                    assert found_corresponding_card, "Failed to find card " + play + " in the following option list: " + msg
                                    current_selection = self.hand_disp.curselection()
                                    if len(current_selection) == 1:
                                        card = self.hand_cards[current_selection[0]]
                                    else: #Nothing yet selected, select the first item
                                        card = self.hand_cards[0]
                                    play_card_button_text = 'Play ' + card
                                    if card in self.hand_options: #Check if the already selected card is a valid play and update the button accordingly
                                        self.play_card_button.configure(command=lambda: self.hide_options_empty_stack(self.hand_options[card]))
                                    else:
                                        self.play_card_button.configure(command=lambda: self.hide_options_empty_stack(None))
                                    self.play_card_button.configure(text=play_card_button_text)
                                    self.play_card_button.grid(column=0, row=1)
                    elif msg.find('[deal_setup]') != -1:
                        offense_color = None
                        defense_color = None
                        valid_offense_colonies_matched = False
                        valid_offense_colonies = []
                        valid_defense_colonies_matched = False
                        valid_defense_colonies = []
                        for line in msg.splitlines():
                            offense_match = re.search('Offense = (.*)', line)
                            defense_match = re.search('Defense = (.*)', line)
                            valid_offense_colonies_match = re.search('Valid offense colonies', line)
                            valid_defense_colonies_match = re.search('Valid defense colonies', line)
                            if line.find('[needs_response]') != -1:
                                continue
                            elif line.find('[deal_setup]') != -1:
                                continue
                            elif offense_match:
                                offense_color = offense_match.group(1)
                            elif defense_match:
                                defense_color = defense_match.group(1)
                            elif valid_offense_colonies_match:
                                valid_offense_colonies_matched = True
                            elif valid_defense_colonies_match:
                                valid_offense_colonies_matched = False
                                valid_defense_colonies_matched = True
                            elif valid_offense_colonies_matched:
                                valid_offense_colonies.append(line)
                            elif valid_defense_colonies_matched:
                                valid_defense_colonies.append(line)
                            else:
                                raise Exception('Unexpected line when parsing deal setup: ' + line)
                        assert offense_color is not None, "Error parsing deal setup"
                        assert defense_color is not None, "Error parsing deal setup"
                        print('Deal setup:\nOffense = ' + offense_color + '\nDefense = ' + defense_color + '\nOffense colonies w/o defensive ships: ' + str(valid_offense_colonies) + '\nDefense colonies w/o offensive ships: ' + str(valid_defense_colonies))
                        #TODO: Create the deal window given the above parameters and the client's hand
                        option_num = 0
                        for colony_choice in valid_offense_colonies:
                            self.offense_to_defense_colonies.append(ttk.Radiobutton(self.offense_to_defense_frame, text=colony_choice, variable=self.offense_to_defense_colony, value=colony_choice))
                            self.offense_to_defense_colonies[-1].grid(column=0, row=1+option_num)
                            option_num += 1
                        self.offense_to_defense_colonies.append(ttk.Radiobutton(self.offense_to_defense_frame, text='None', variable=self.offense_to_defense_colony, value='None'))
                        self.offense_to_defense_colonies[-1].grid(column=0, row=1+option_num)
                        option_num = 0
                        for colony_choice in valid_defense_colonies:
                            self.defense_to_offense_colonies.append(ttk.Radiobutton(self.defense_to_offense_frame, text=colony_choice, variable=self.defense_to_offense_colony, value=colony_choice))
                            self.defense_to_offense_colonies[-1].grid(column=0, row=1+option_num)
                            option_num += 1
                        self.defense_to_offense_colonies.append(ttk.Radiobutton(self.defense_to_offense_frame, text='None', variable=self.defense_to_offense_colony, value='None'))
                        self.defense_to_offense_colonies[-1].grid(column=0, row=1+option_num)
                        self.deal_window.state('normal') #Display the window
                    elif msg.find('[propose_deal]') != -1:
                        offense_receives_match = re.search('Offense receives colony: (.*)\n', msg)
                        defense_receives_match = re.search('Defense receives colony: (.*)\n', msg)
                        assert offense_receives_match, "Error parsing propsed deal"
                        assert defense_receives_match, "Error parsing propsed deal"
                        terms = ''
                        if offense_receives_match.group(1) != 'None':
                            terms += 'The offense will establish a colony on ' + offense_receives_match.group(1) + '.\n'
                        if defense_receives_match.group(1) != 'None':
                            terms += 'The defense will establish a colony on ' + defense_receives_match.group(1) + '.\n'
                        self.deal_terms.set(terms)
                        self.deal_acceptance_window.state('normal') #Display the window
                    elif msg.find('[reject_deal]') != -1:
                        #The other player rejected our deal, bring up the deal window in case we want to make a subsequent offer
                        self.deal_window.state('normal')
                    elif msg.find('[accept_deal]') != -1:
                        #Our deal has been accepted, send an ack to the server since it's still looping on a response from us
                        self.send_message_to_server('[ack]')
                        self.deal_acceptance_window.withdraw() #If we were deciding on a different offer we can get rid of that window now; the deal_window should already be removed from the deal we proposed that has now been accepted
                    else:
                        option_num = None
                        prompt = ''
                        for line in msg.splitlines():
                            option_match = re.match('([0-9]*): (.*)',line)
                            if line.find('[needs_response]') != -1: #This line is delivered after the options
                                break
                            if option_match:
                                if option_num == None: #If this is the first option we've found we now have the entire prompt
                                    self.choice_label_var.set(prompt)
                                option_num = option_match.group(1)
                                prompt = option_match.group(2)
                                self.choice_list.append(ttk.Radiobutton(self.choice_frame, text=prompt, variable=self.client_choice, value=option_num))
                                option_row = int(option_num)+1
                                self.choice_list[int(option_num)].grid(column=0,row=option_row)
                            elif option_num == None: #We haven't found an option yet, so we're still reading the prompt
                                if len(prompt) != 0:
                                    prompt += '\n'
                                prompt += line
                        if option_num == None:
                            print('ERROR:\n' + msg)
                            raise Exception('A response is required but we failed to find any options!')
                        confirmation_row = int(option_num)+2
                        self.confirmation_button.configure(text='Confirm choice', command=self.hide_options)
                        self.confirmation_button.grid(column=0, row=confirmation_row)
                if msg.find('[player_hand]') != -1: #Update the player's hand
                    tag_found = True
                    if len(self.player_color_label_var.get()) == 0:
                        player_color_match = re.search('Hand for the (.*) player',msg)
                        if player_color_match:
                            assert player_color_match.group(1) == 'Red' or player_color_match.group(1) == 'Blue' or player_color_match.group(1) == 'Purple' or player_color_match.group(1) == 'Green' or player_color_match.group(1) == 'Yellow', "Found an unexpected player color!"
                            self.player_color = player_color_match.group(1)
                            self.player_color_label_var.set("You are the " + player_color_match.group(1) + " player")
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
                    self.turn_phase = phase_index
                    for i in range(len(self.turn_phase_labels)):
                        if i == phase_index:
                            self.turn_phase_labels[i].config(bg="Orange")
                        else:
                            self.turn_phase_labels[i].config(bg=self.default_label_bg)
                    if phase_index == 0: #Reset the highlighted planet for the next encounter
                        for planet_label in self.planet_labels:
                            planet_label.configure(background=self.default_label_bg)
                        self.current_offense_score_label.grid_forget()
                        self.current_defense_score_label.grid_forget()
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
                        self.num_players = len(players)
                        #TODO: We can probably do a better job of organizing this data, but this is a good start
                        for i in range(len(players)):
                            for j in range(num_planets): #Create a row for each planet
                                self.planet_canvases.append(Canvas(self.game_board_frame, width=self.warp_width, height=self.warp_height, background="black")) #TODO: What background to use here? Perhaps an image of stars like space? #TODO: Adjust the width of the canvas to the number of colonies on each planet? Hmm
                                self.planet_canvases[(num_planets*i)+j].grid(column=i,row=4+(2*j)+1)
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
                                self.planet_labels.append(Label(self.game_board_frame,text=labeltext))
                                self.planet_labels[-1].grid(column=i,row=4+2*j)
                                #Add a label for the player's Alien
                                if j == 0:
                                    self.player_aliens[player] = []
                                    self.player_aliens[player].append(StringVar())
                                    self.player_aliens[player].append(Label(self.game_board_frame, textvariable=self.player_aliens[player][0]))
                                    self.player_aliens[player][0].set(player + ' player alien: ???') #Keep the client's Alien in an unrevealed state to let them know that thier alien is still unknown to others (they can see their own alien info elsewhere)
                                    self.player_aliens[player][1].grid(column=i,row=2)
                                    self.color_to_num[player] = i
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
                                ship_tag = color + ' _ships_' + player + '_planet_' + str(planet_id) #Example: Blue_ships_Red_planet_4 means a Blue colony on Red planet 4
                                self.planets.append(self.planet_canvases[(num_planets*i)+planet_id].create_oval(left,top,right,bottom,fill=color,outline='black',tags=(ship_tag,))) #Tag each stack of ships with the color planet and planet number they're on
                                self.planets.append(self.planet_canvases[(num_planets*i)+planet_id].create_text(center_hor,center_ver,text=str(num),fill='white')) #Is white easier to see here? Can we make the text larger or bold it to make it more prominent?
                                bbox = self.planet_canvases[(num_planets*i)+planet_id].bbox(self.planets[-1]) #Get a bounding box for the newly created text object
                                self.planets.append(self.planet_canvases[(num_planets*i)+planet_id].create_rectangle(bbox, fill="black")) #Add a black background to the bounding box
                                self.planet_canvases[(num_planets*i)+planet_id].tag_raise(self.planets[-2],self.planets[-1]) #Bring the text in front of the background
                                colorcount += 1
                            planet_id += 1
                #TODO: Handle PlayerColors::Invalid for offense and defense and clear this info from the GUI when relevant? Not a huge deal because it's obvious when these are valid via turn phase, but this is a "nice to have"
                #       We can clear this when the turn phase changes to 'Start Turn'
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
                    self.player_aliens[player].append(alien_desc)
                    self.player_aliens[player][1].bind('<Enter>', lambda e: self.update_revealed_alien_info(self.player_aliens[player][2]))
                if msg.find('[alien_aux_update]') != -1: #Some other alien information was received from the server
                    tick_tock_aux_match = re.search('\[alien_aux_update\]\n(.*) tokens remaining: (.*)',msg)
                    assert tick_tock_aux_match, "Failed to parse alien aux update!"
                    for alien, alien_info in self.player_aliens.items():
                        if alien_info[0].get().find(tick_tock_aux_match.group(1)) != -1: #If this alien is indeed Tick-Tock
                            if alien_info[2].find('Tokens remaining') != -1: #Replace the existing aux data
                                alien_info[2] = re.sub('Tokens remaining: (.*)','Tokens remaining: ' + tick_tock_aux_match.group(2), alien_info[2])
                            else:
                                alien_info[2] += 'Tokens remaining: ' + tick_tock_aux_match.group(2)
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
                if msg.find('[player_hand_size]') != -1: #Update the number of cards in the given player's hand
                    tag_found = True
                    player_hand_match = re.search('\[player_hand_size\] (.*): (.*)',msg)
                    assert player_hand_match, "Failed to parse player hand size message"
                    player = player_hand_match.group(1)
                    if player not in self.player_hand_size_labels:
                        self.player_hand_size_labels[player] = Label(self.game_board_frame, text='Hand size: ' + player_hand_match.group(2))
                        #NOTE: This column choice is exploiting how the player hands are dealt in order; ideally we should probably have a map from color to indices for better organization
                        self.player_hand_size_labels[player].grid(column=len(self.player_hand_size_labels)-1,row=3)
                    else:
                        self.player_hand_size_labels[player].configure(text='Hand size: ' + player_hand_match.group(2))
                if msg.find('[targeted_planet]') != -1:
                    tag_found = True
                    target_planet_match = re.search('player on (.*) Planet (.*)',msg)
                    assert target_planet_match, "Failed to parse targeted planet!"
                    for planet_label in self.planet_labels:
                        if planet_label and planet_label.cget('text').find(target_planet_match.group(1)) != -1 and planet_label.cget('text').find(target_planet_match.group(2)) != -1:
                            planet_label.config(background='Orange')
                            break
                if msg.find('[stack_update]') != -1:
                    tag_found = True
                    #Put the stack info in its own box
                    stack_info = ''
                    if msg.find('{') != -1: #If the stack isn't empty
                        stack_info = msg[msg.find('{'):]
                    self.update_stack_info(stack_info)
                if msg.find('[score_update]') != -1:
                    tag_found = True
                    scores_match = re.search('Offense = (.*); Defense = (.*)\n',msg)
                    offense_score = scores_match.group(1)
                    defense_score = scores_match.group(2)
                    self.current_offense_score.set('Current offense score: ' + offense_score)
                    self.current_offense_score_label.grid(column=0, row=4)
                    self.current_defense_score.set('Current defense score: ' + defense_score)
                    self.current_defense_score_label.grid(column=0, row=5)
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

    def hide_options_colony(self,planet_color,planet_num,option_num):
        #Once a choice is confirmed we need to remove the colony on click bindings, remove the button, and send the response to the server
        #We'll also need to reconfigure the confirmation button to its standard state
        #There's a decent chance this scheme to undo the bindings isn't even necessary because the canvas objects will be deleted and redrawn anyway
        num_planets = 5
        for binding in self.colony_bindings:
            i = binding[0]
            j = binding[1]
            ship_tag = binding[2]
            self.planet_canvases[(num_planets*i)+j].tag_unbind(ship_tag, '<ButtonPress-1>')
            self.planet_canvases[(num_planets*i)+j].tag_unbind(ship_tag, '<Enter>')
            self.planet_canvases[(num_planets*i)+j].tag_unbind(ship_tag, '<Leave>')
        self.no_colony_button.grid_forget()
        self.confirmation_button.grid_forget()
        self.confirmation_button.configure(text='Confirm choice', command=self.hide_options)
        self.choice_label_var.set("Waiting on other players...")
        if planet_color == '' and planet_num == '' and option_num == '':
            option_num = self.no_colony_option
        self.send_message_to_server(option_num)

    def hide_options_planet(self,planet_color,planet_num,option_num):
        num_planets = 5
        for binding in self.planet_bindings:
            i = binding[0]
            j = binding[1]
            self.planet_canvases[(num_planets*i)+j].unbind('<ButtonPress-1>')
            self.planet_canvases[(num_planets*i)+j].unbind('<Enter>')
            self.planet_canvases[(num_planets*i)+j].unbind('<Leave>')
        self.confirmation_button.grid_forget()
        self.confirmation_button.configure(text='Confirm choice', command=self.hide_options)
        self.choice_label_var.set("Waiting on other players...")
        self.send_message_to_server(option_num)

    def get_next_turn_phase(self):
        if self.turn_phase == 0:
            return "Regroup"
        elif self.turn_phase == 1:
            return "Destiny"
        elif self.turn_phase == 2:
            return "Launch"
        elif self.turn_phase == 3:
            return "Alliance"
        elif self.turn_phase == 4:
            return "Planning"
        elif self.turn_phase == 5:
            return "Reveal"
        elif self.turn_phase == 6:
            return "Resolution"
        elif self.turn_phase == 7:
            return "Start Turn"
        else:
            assert False, "Unexpected turn phase!"

    def hide_options_empty_stack(self, option_num):
        #if the selected option isn't valid, don't do anything
        if not option_num:
            return
        self.next_turn_phase_button.grid_forget()
        self.play_card_button.grid_forget()
        self.play_alien_button.grid_forget()
        self.alien_play_option = ''
        self.hand_options = {}
        self.choice_label_var.set("Waiting on other players...")
        self.send_message_to_server(option_num)

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

