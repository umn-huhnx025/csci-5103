#! /usr/bin/python3
import os
import sys
device = "/proc/devices"
modname = "scullBuffer"
filename = "./scullBuffer.ko"
mode = "777"
noDevices = 1

# first install the module
if os.system("/sbin/insmod " + filename + " " + " ".join(sys.argv[1:])) != 0:
    print("Cant install module\n")

# now we need to create the device files
file = open(device)
lines = file.readlines()
for line in lines:
    if(line.find(modname) >= 0):
        major = line.split()[0]
        break
# now since we have the major number, we can create the files
for i in range(0, noDevices):
    if os.system("/bin/mknod " + "/dev/" + modname + i.__str__() + " c " + major +
                 " " + i.__str__()) != 0:
        print("Cant make node..\n")
    # time to change the mode of the file
    if os.system("/bin/chmod " + mode + " " + "/dev/" + modname + i.__str__()) != 0:
        print("cant change mode of file..\n")

print("Installed module: " + modname + " and associated it with dev node: " +
      "/dev/" + modname + "*" + " with appropriate permissions\n")
