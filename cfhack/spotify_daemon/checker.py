import sqlite3
import sys
import os


# arg: motion stopped, motion started

args = sys.argv

if len(args) <= 1:
	print "GIVE ME an action"
	exit()

mode = args[1] 

if mode == 'motion':
	# kill current jukebox 
	print ">>> Stopping the controller"
	
	os.system( "/home/nyan/SpotiPi/cfhack/spotify_daemon/src/jukebox/jukebox_controller stop" )

	# check if database says playback enabled
	#SQL SHIT
	print ">>> GRabbing data to make decision"

	qry = "SELECT name, password, enabled, default_pl from users";
	con = sqlite.connect("/home/nyan/SpotiPi/cfhack/web/examplesite/example.db")
	cur = con.cursor()
	result = cur.execute(qry)
	
	do_playback = True
	username = "voxyn"
	password = "bastard11"
	playlist_id = 3

	# call jukebox functionm
	if do_playback:
		print ">>> doing playback"
	
		#os.chdir()i
		os.chdir("/home/nyan/SpotiPi/cfhack/spotify_daemon/src/")
		opts = "-u %s -p %s -l %d" % (username, password, playlist_id)
		cmd = """/home/nyan/SpotiPi/cfhack/spotify_daemon/src/jukebox/jukebox_controller start %s """ %( opts)
		print cmd
		os.system( cmd )


elif mode == 'nomotion':
	# kill jukebox
	
	os.system( "/home/nyan/SpotiPi/cfhack/spotify_daemon/src/jukebox/jukebox_controller stop" )


else:
	print "Action not recognised"
	exit()


print "fin"



# get playlist iD







