from gi.repository import Notify

keysFileName = "keys"
codesToKeys = {}
keysFile = open(keysFileName, 'r')
for line in keysFile:
    key, code = line.split()
    codesToKeys[int(code)+8] = key
keysFile.close()

Notify.init ("Profile Notification")
profileChange=Notify.Notification.new ("Profile change", "","dialog-information")
while True:
    i = raw_input()
    msgCode, profName, key1, key2 = i.split()
    if msgCode == 'A':
        profileChange.update("Ready", "Mouse connected", "dialog-information")
        profileChange.show()
    elif msgCode == 'C':
        profileChange.update( "Profile changed to " + profName,
                              "Button 1: " + codesToKeys[int(key1)+8] + " Button 2: " + codesToKeys[int(key2)+8] , "dialog-information")
        profileChange.show()
