import bluetooth
import sys

server_sock=bluetooth.BluetoothSocket( bluetooth.RFCOMM )

port = 0
server_sock.bind(("",port))
server_sock.listen(1)

uuid = "00000000-0000-0000-0000-00000000ABCD"
bluetooth.advertise_service( server_sock, "FooBar Service", uuid )

client_sock,address = server_sock.accept()

while True:
    data = client_sock.recv(1024)
    print "%s" % data
    sys.stdout.flush()

client_sock.close()
server_sock.close()
