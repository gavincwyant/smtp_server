from socket import *
import threading

def run():
    #connection socket
    server_port = 8089
    connection_socket = socket(AF_INET, SOCK_STREAM)
    connection_socket.bind(('', server_port))
    #listen
    connection_socket.listen(5)
    #server serves
    while(True):
        client_socket,addr = connection_socket.accept()
        thread = threading.Thread(target=handle_client, args=(client_socket, addr))
        thread.daemon = True   # dies when main program exits
        thread.start()

    #close connection socket
    connection_socket.close()




def handle_client(client_socket, addr):
    try:
        #client socket
        #client speaks first
        req = client_socket.recv(1024).decode()
        #get the correct webpage from request
        request_line = req.splitlines()[0]
        method, path, version = request_line.split()
        if method != "GET":
            response = "HTTP/1.1 405 Method Not Allowed\r\n\r\nMethod Not Allowed"
            client_socket.close()
        try:
            path = path[1:]
            with open(path, "r") as file:
                body = file.read()
                headers = (
                    "HTTP/1.1 200 OK\r\n"
                    f"Content-Length: {len(body)}\r\n"
                    f"Content-Type: text/html\r\n"
                    "Connection: close\r\n"
                    "\r\n"
                )
                #send header
                client_socket.send(headers.encode())
                #send file
                client_socket.send(body.encode())
        except FileNotFoundError:
            #if file does not exist send 404
            response = "HTTP/1.1 404 Not Found\r\n\r\nFile Not Found"
            client_socket.send(response.encode())

    finally:
        #close client connection
        client_socket.close()


if __name__ == "__main__":
    run()
