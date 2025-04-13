#include <SFML/Network.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>

int main(){
    sf::TcpListener listener;
    listener.setBlocking(false);
    if (listener.listen(53000) != sf::Socket::Done){
        std::cerr << "Error: Couldn't bind to port 53000\n";
        return 1;
    }

    std::vector<sf::TcpSocket*> clients;
    sf::SocketSelector selector;
    selector.add(listener);

    std::cout << "A server has been created on port 53000\n";

    while (true){
        if (selector.wait(sf::milliseconds(100))){
            if (selector.isReady(listener)){
                sf::TcpSocket* client = new sf::TcpSocket;
                if (listener.accept(*client) == sf::Socket::Done){
                    client->setBlocking(false);
                    clients.push_back(client);
                    selector.add(*client);
                    std::cout << "[Server] New client connected: " << client->getRemoteAddress() << "\n";
                }
                else delete client;
            }

            for (auto it = clients.begin(); it != clients.end();){
                sf::TcpSocket* client = *it;
                if (selector.isReady(*client)){
                    sf::Packet packet;
                    if (client->receive(packet) != sf::Socket::Done)
                    {
                        std::cout << "[Server] Client disconnected.\n";
                        selector.remove(*client);
                        delete client;
                        it = clients.erase(it);
                        continue;
                    }

                    std::string msg;
                    packet >> msg;
                    std::cout << msg << "\n";

                    // Broadcast to all
                    for (auto& other : clients)
                    {
                        sf::Packet out;
                        out << msg;
                        other->send(out);
                    }
                }
                ++it;
            }
        }

        // Server typing
        fd_set inputSet;
        FD_ZERO(&inputSet);
        FD_SET(STDIN_FILENO, &inputSet);
        struct timeval timeout = {0, 0};
        if (select(STDIN_FILENO + 1, &inputSet, NULL, NULL, &timeout) > 0)
        {
            std::string input;
            std::getline(std::cin, input);
            std::string msg = "[Server] " + input;
            std::cout << msg << "\n";

            sf::Packet packet;
            packet << msg;
            for (auto& client : clients)
                client->send(packet);
        }
    }

    for (auto client : clients)
        delete client;
    return 0;
}
