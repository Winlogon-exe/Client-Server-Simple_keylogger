import socket
from datetime import datetime

def start_server():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(('your_ip', your_port))
    server_socket.listen(5)

    connected_clients = {}  # Словарь для отслеживания подключившихся IP-адресов

    while True:
        client_socket, addr = server_socket.accept()
        client_ip = addr[0] 

        if client_ip not in connected_clients:
            current_time = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"client_{client_ip}_{current_time}.txt"
            print(f"New connection from {addr}, logging to {filename}")
            connected_clients[client_ip] = filename

            with open(filename, 'w') as file:
                while True:
                    data = client_socket.recv(1024)
                    if not data:
                        break
                    print("Received:", data.decode())
                    file.write(data.decode())
        else:
            print(f"Existing connection from {addr}, appending to {connected_clients[client_ip]}")
            with open(connected_clients[client_ip], 'a') as file:
                while True:
                    data = client_socket.recv(1024)
                    if not data:
                        break
                    #print("Received:", data.decode())
                    file.write(data.decode())
        
        client_socket.close()

start_server()
