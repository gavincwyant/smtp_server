#client code
from socket import *
server_name = 'localhost'
server_port = 8089


#connection socket
client_socket = socket(AF_INET, SOCK_STREAM)
client_socket.connect((server_name, server_port))

path = input("Enter pathname:\t")

print(path)
#ask for something
req = f"GET /{path} HTTP/1.1"
client_socket.send(req.encode())

#get something back
header = client_socket.recv(1024)
#carriage return carriage return
while b"\r\n\r\n" not in header:
    header += client_socket.recv(1024)

#parse header for content length
header_data, body_start = header.split(b"\r\n\r\n", 1)
headers = header_data.decode()
print("Headers:\n", headers)

# Find Content-Length
length = 0
for line in headers.split("\r\n"):
    if line.lower().startswith("content-length:"):
        length = int(line.split(":")[1].strip())

# Already got some of the body in body_start
body = body_start
while len(body) < length:
    body += client_socket.recv(1024)

print("Body:\n", body.decode())

#do something with what we got
