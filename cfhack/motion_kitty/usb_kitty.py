import usb, time, os
from usb import core
#import sys

#####################################################
##          Parameters                             ##
#####################################################
interval = 0.02     # 0.01 Sample interval (seconds)
sample = 1.5        # 1.0 How long is the sample (block len) (seconds)
min_movement=1     # 2 Threshold for samples different to last second

#####################################################
##          Movement response                      ##
#####################################################
def fire():
    print "MOVEMENT!"

#####################################################
##          Find and Grab USB Device               ##
#####################################################

if os.path.exists("./pid"):
    os.remove("./pid")
f = open("./pid","w")
pid = os.getpid()
f.write(str(pid))
f.close()

device = usb.core.find(idVendor=6465, idProduct=32801)
if device.is_kernel_driver_active(0):
    try:
        device.detach_kernel_driver(0)
    except usb.core.USBError as e:
        sys.exit("Could not detatch kernel driver: %s" % str(e))
device.set_configuration()

try:
    device.set_configuration()
    device.reset()
except usb.core.USBError as e:
    sys.exit("Could not set configuration: %s" % str(e))

endpoint = device[0][(0,0)][0]

data = []

statuses=[0,4]      # High/low bits for movement input
laststatus=True        # Status of previous sample (default high)
changes=0           # Number of changes so far this sample (reset each sample)
elapsed_time = 0    # Elapsed time for current sample (reset each sample)

threshold = min_movement
false_threshold = (sample/interval)-min_movement
print false_threshold

while 1:
    try:
        data = device.read(endpoint.bEndpointAddress, endpoint.wMaxPacketSize)
        #print data
        status = statuses.index(data[1])        # reformat signal to high/low
        #print laststatus,status
        if not(laststatus==status):             # if status is different to previous sample
            changes+=1                          # count change
        time.sleep(interval)                    
        elapsed_time+=interval

        if laststatus:                              # If laststatus was no movement
            if elapsed_time>=sample or changes>=threshold:                # If end of sample period read, check if movement criteria met
                #print changes
                if changes>=threshold:
                        laststatus=not(laststatus)  # Flip laststatus
                        fire()                      # fire movement response
                changes=0
                elapsed_time=0
        else:                                       # if last status was movement
            if elapsed_time>=sample or changes>=false_threshold:                # If end of sample period read, check if movement criteria met
                #print changes
                if changes>=false_threshold:
                        laststatus=not(laststatus)  # Flip laststatus
                        fire()                      # fire movement response
                changes=0
                elapsed_time=0
	print "\t mode:", laststatus, "      block tot :", changes, "      ", time.clock()
            
    except usb.core.USBError as e:
        print "Oops! - "+e
