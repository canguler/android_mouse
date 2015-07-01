import bluetooth

target_name = "Can (Galaxy S4)"
target_address = None

nearby_devices = bluetooth.discover_devices()

for bdaddr in nearby_devices:
    bname = bluetooth.lookup_name( bdaddr )
    print bname
    if target_name == bname:
        target_address = bdaddr
        break

if target_address is not None:
    print "found target bluetooth device with address ", target_address
else:
    print "could not find target bluetooth device nearby"
