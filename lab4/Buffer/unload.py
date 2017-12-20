#! /usr/bin/python3
import os
modname = "scullBuffer"
filename = "./scullBuffer.ko"
noDevices = 1

if (os.system("/sbin/rmmod " + filename) != 0):
    print("Cant remove module\n")
for i in range(0, noDevices):
    if(os.system("/bin/rm " + "/dev/" + modname + i.__str__()) != 0):
        print("Problem removing device file: " +
              "/dev/" + modname + i.__str__())
print("Removed module: " + modname +
      " and related device files: " + "/dev/" + modname + "*\n")
