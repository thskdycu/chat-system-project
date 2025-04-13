#include <SFML/Network.hpp>
#include <iostream>
#include <string>
#include <unistd.h>

int main()
{
    sf::TcpSocket socket;
    if (socket.connect("127.0.0.1", 53000) != sf::Socket::Done)
    {
        std::cerr << "[Client] Connection failed.\n";
        return 1;
    }

    socket.setBlocking(false);
    std::cout << "[Client] Connected.\n";

    while (true)
    {
        // Input from user
        fd_set inputSet;
        FD_ZERO(&inputSet);
        FD_SET(STDIN_FILENO, &inputSet);
        struct timeval timeout = {0, 0};
        if (select(STDIN_FILENO + 1, &inputSet, NULL, NULL, &timeout) > 0)
        {
            std::string userInput;
            std::getline(std::cin, userInput);
            if (userInput == "exit") break;

            sf::Packet packet;
            packet << "[Client] " + userInput;
            socket.send(packet);
        }

        // Receive messages
        sf::Packet packet;
        if (socket.receive(packet) == sf::Socket::Done)
        {
            std::string msg;
            packet >> msg;
            std::cout << msg << "\n";
        }

        sf::sleep(sf::milliseconds(100));
    }

    std::cout << "[Client] Disconnected.\n";
    return 0;
}
