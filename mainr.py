import socket
from datetime import datetime
import logging

BUFFER_SIZE = 1024  

def start_server(server_ip, server_port):
    try:
        server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server_socket.bind((server_ip, server_port))
        server_socket.listen(5)

        connected_clients = {}

        while True:
            client_socket, addr = server_socket.accept()
            handle_client(client_socket, addr, connected_clients)

    except Exception as e:
        logging.error(f"Error in server: {e}")
    finally:
        server_socket.close()

def handle_client(client_socket, addr, connected_clients):
    client_ip = addr[0]

    if client_ip not in connected_clients:
        current_time = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"client_{client_ip}_{current_time}.txt"
        logging.info(f"New connection from {addr}, logging to {filename}")
        connected_clients[client_ip] = filename

    else:
        logging.info(f"Existing connection from {addr}, appending to {connected_clients[client_ip]}")
        filename = connected_clients[client_ip]

    with open(filename, 'a') as file:
        while True:
            data = client_socket.recv(BUFFER_SIZE)
            if not data:
                break
            file.write(data.decode())

    client_socket.close()

if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
    start_server("192.168.1.123", 5555)
