import os

f = open("./pid",'r')
pid = int(f.readlines()[0])
try:
    os.kill(pid,1)
except OSError as e:
    print "Error killing pid %d: %s" %(pid,e)
os.remove("./pid")
