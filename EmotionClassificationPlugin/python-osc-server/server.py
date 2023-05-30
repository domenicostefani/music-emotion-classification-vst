#!/usr/bin/env python3
"""Small example OSC server

This program listens to several addresses, and prints some information about
received packets.
"""
import argparse
import os
import glob

from pythonosc.dispatcher import Dispatcher
from pythonosc.udp_client import SimpleUDPClient

from pythonosc import osc_server


# For Elk Board
if os.path.exists("/udata"):
    RECORDINGS_DIR = "/udata/emoAwSMIs-recordings/unnamed"
    RECORDINGS_BAK_DIR = "/udata/emoAwSMIs-recordings/bakup"
    RECORDINGS_RENAMED_DIR = "/udata/emoAwSMIs-recordings/RENAMED"
    RECORDINGS_DELETED_DIR = "/udata/emoAwSMIs-recordings/trashcan"
else:
    RECORDINGS_DIR = "./recordings"
    RECORDINGS_BAK_DIR = "./recordings_bak"
    RECORDINGS_RENAMED_DIR = "./renamed_recordings"
    RECORDINGS_DELETED_DIR = "./recordings_trashcan"

RECORDINGS_PATTERN = RECORDINGS_DIR + "/*.wav"
RECORDINGS_ANDTXT_PATTERN = RECORDINGS_DIR + "/*"

# def print_volume_handler(unused_addr, args, volume):
#   print("[{0}] ~ {1}".format(args[0], volume))

# def print_compute_handler(unused_addr, args, volume):
#   try:
#     print("[{0}] ~ {1}".format(args[0], args[1](volume)))
#   except ValueError: pass

# CLIENT = None
SERVER_IP = ""
RX_PORT = 6042
TX_PORT = 7042

BASH_START_PIANO = "sh ./run_piano.sh"
BASH_START_ACOUSTIC = "sh ./run_acoustic.sh"
BASH_START_ELECTRIC = "sh ./run_electric.sh"
BASH_STOP = "sh ./stop.sh"


def reply_handshake(addr, *args):
    global CLIENT
    print("received /pyshake " + " ".join([str(x) for x in args]))
    if len(args) == 2:
        MY_IP = args[0]
        CONTROLLER_IP = args[1]
        # print("Received pyshake from {}, telling me that my IP is {}".format(CONTROLLER_IP, MY_IP))
        if CLIENT== None:
            # print("Since it's the first time, I'll send a pyshake back to the controller (\"{}\") on port {}".format(CONTROLLER_IP,TX_PORT))
            CLIENT= SimpleUDPClient(CONTROLLER_IP, TX_PORT)
            print("Sending pyshake back to the controller (\"{}\") on port {}".format(CONTROLLER_IP,TX_PORT))
            CLIENT.send_message("/pyshake", "hello")
            assert CLIENT!= None
        else:
            print(
                "I already know my IP, so I won't send a pyshake back to the controller"
            )
            print(str(CLIENT))


def start_plugin(addr, *args):
    global CLIENT
    global ALREADY_RUNNING
    print("received /startPlugin " + " " + " ".join([str(x) for x in args]))

    if len(args) == 1 and CLIENT!= None and not ALREADY_RUNNING:
        ALREADY_RUNNING = True
        if args[0] == "piano":
            os.system(BASH_START_PIANO)
        elif args[0] == "acoustic":
            os.system(BASH_START_ACOUSTIC)
        elif args[0] == "electric":
            os.system(BASH_START_ELECTRIC)


def stop_plugin(addr, *args):
    print("received /stopPlugin " + " ".join([str(x) for x in args]))
    global CLIENT
    global ALREADY_RUNNING
    if len(args) == 0 and CLIENT!= None:
        os.system(BASH_STOP)
        ALREADY_RUNNING = False
        CLIENT.send_message("/stopped","1")


def disconnect(addr, *args):
    global CLIENT
    print("received /disconnect " + " ".join([str(x) for x in args]))
    CLIENT = None

def renameRecording(addr, *args):
    global CLIENT
    print("received " + addr + " " + " ".join([str(x) for x in args]))
    os.system('mkdir -p '+RECORDINGS_RENAMED_DIR)

    if len(args) == 1 and CLIENT!= None:
        new_filename = os.path.splitext(args[0])[0]
        list_of_files = glob.glob(RECORDINGS_PATTERN) # * means all if need specific format then *.csv
        
        # Check that there is no file with new_filename.* in RECORDINGS_RENAMED_DIR
        potential_samename = glob.glob(os.path.join(RECORDINGS_RENAMED_DIR,new_filename+".*"))
        if len(potential_samename) > 0:
            print('Rename error: file with same name already exists')
            CLIENT.send_message("/renamed", "errorOverwrite")
        elif len(list_of_files) > 1:
            print('Rename error: too many files found')
            CLIENT.send_message("/renamed", "errorTooManyFiles")
        elif len(list_of_files) == 0:
            print('Rename error: no file found')
            CLIENT.send_message("/renamed", "errorNoFile")
        else:
            # Include labels
            list_of_files = glob.glob(os.path.splitext(list_of_files[0])[0]+"*")
            print('list_of_files',list_of_files)
            os.system('mkdir -p '+RECORDINGS_RENAMED_DIR)
            os.system('mkdir -p '+RECORDINGS_BAK_DIR)

            for file in list_of_files:
                os.system('cp '+file+' '+RECORDINGS_BAK_DIR)
                if os.path.splitext(file)[1] == ".wav":
                    new_fullpath = os.path.join(RECORDINGS_RENAMED_DIR,new_filename+'.wav')
                    os.system('mv '+file+' '+new_fullpath)
                    # os.system('echo "Renamed '+file+' to '+new_fullpath+' > '+RECORDINGS_RENAMED_DIR+'/copylog'+new_filename+'.txt')
                    # Redo previous line with python access to file to avoid bash
                    with open(os.path.join(RECORDINGS_RENAMED_DIR,'copylog'+new_filename+'.txt'), 'w') as f:
                        f.write('Renamed '+file+' to '+new_fullpath+'\n')
                    
                else:
                    os.system('mv '+file+' '+os.path.join(RECORDINGS_RENAMED_DIR,new_filename+os.path.basename(file)))
            CLIENT.send_message("/renamed", "ok")


def deleteUnnamed(addr, *args):
    global CLIENT
    if len(args) == 0 and CLIENT!= None:
        print("received " + addr + " ".join([str(x) for x in args]))
        os.system('mkdir -p '+RECORDINGS_DELETED_DIR)

        list_of_files = glob.glob(RECORDINGS_ANDTXT_PATTERN) # * means all if need specific format then *.csv
        for file in list_of_files:
            os.system('mv '+file+' '+RECORDINGS_DELETED_DIR)


def deleteAllButLast(addr, *args):
    global CLIENT
    if len(args) == 0 and CLIENT!= None:
        print("received " + addr)
        os.system('mkdir -p '+RECORDINGS_DELETED_DIR)

        list_of_files = glob.glob(RECORDINGS_PATTERN) # * means all if need specific format then *.csv
        # Sort by date
        list_of_files.sort(key=os.path.getmtime)
        print('list_of_files',list_of_files)
        # Keep only the last one
        list_of_files = list_of_files[:-1]
        print('list_of_files except for latest',list_of_files)
        # Append all txt files that begin with the same name
        toappend = []
        for file in list_of_files:
            toappend.extend(glob.glob(os.path.splitext(file)[0]+"*.txt"))
        list_of_files.extend(toappend)
        list_of_files = list(set(list_of_files)) # Remove duplicates
        print('list_of_files with txt',list_of_files)

        for file in list_of_files:
            os.system('mv '+file+' '+RECORDINGS_DELETED_DIR)
            print('Deleted '+file)

def setDate(addr, *args):
    global CLIENT
    month_num2str = {1:'Jan',2:'Feb',3:'Mar',4:'Apr',5:'May',6:'Jun',
                     7:'Jul',8:'Aug',9:'Sep',10:'Oct',11:'Nov',12:'Dec'}
    print("received " + addr + " " + " ".join([str(x) for x in args]))
    
    if len(args) == 6 and CLIENT!= None:
        command = 'sudo date -s "'+str(args[2])+ ' ' + month_num2str[args[1]].upper()+' '+str(args[0])+' '+str(args[3])+':'+str(args[4])+':'+str(args[5])+'"'
        print("command:",command)
        os.system(command)
            

if __name__ == "__main__":
    global CLIENT
    global ALREADY_RUNNING
    CLIENT = None
    ALREADY_RUNNING = False
    parser = argparse.ArgumentParser()
    parser.add_argument("--ip", default=SERVER_IP, help="The ip to listen on")
    parser.add_argument("--port", type=int, default=6042, help="The port to listen on")
    args = parser.parse_args()

    dispatcher = Dispatcher()
    dispatcher.map("/pyshake", reply_handshake)
    dispatcher.map("/startPlugin", start_plugin)
    dispatcher.map("/stopPlugin", stop_plugin)
    dispatcher.map("/disconnect", disconnect)
    dispatcher.map("/rename", renameRecording)
    dispatcher.map("/deleteUnnamed", deleteUnnamed)
    dispatcher.map("/deleteAllButLast", deleteAllButLast)
    dispatcher.map("/setDate", setDate)
    #   dispatcher.map("/volume", print_volume_handler, "Volume")
    #   dispatcher.map("/logvolume", print_compute_handler, "Log volume", math.log)

    server = osc_server.ThreadingOSCUDPServer((args.ip, args.port), dispatcher)
    print("Serving on {}".format(server.server_address))
    server.serve_forever()

# if __name__ == "__main__":
#     CLIENT = SimpleUDPClient('localhost', TX_PORT)
#     CLIENT.send_message("/pyshake", "hello")
