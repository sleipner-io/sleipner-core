#include <iostream>
#include <string>

#include "sleipner/net/ip.hpp"
#include "sleipner/transport/tcpclient.hpp"

#include <Windows.h>

int main(int argc, char* argv[]) {
    if ( argc < 2 )
        throw std::runtime_error("Please input a valid hostname, such as www.example.com, to GET!");

    std::string hostname = argv[1];

    std::cout << "Making GET request to: " << hostname << " :80" << std::endl;

    // All resources are automatically cleaned up after this (main) code-block
    sleipner::transport::TcpClient client;

    // 80 is the default un-secure HTTP port
    client.connect( sleipner::net::resolve_ip(hostname, 80) );

    std::cout << "Sending..." << std::endl;

    std::string packet = "GET / HTTP/1.1\r\nHost: " + hostname + "\r\nUser-Agent: cpp-sleipner/2.29.0\r\nAccept: */*\r\nConnection: keep-alive\r\n\r\n";
    client.send(packet);

    std::string res;

    do {
        Sleep(500);
        std::cout << "Receiving more data..." << std::endl;
        res = client.receive(1024, 5000);
    } while ( client.bytes_available() > 0 );

    std::cout << "--- Response: ---" << std::endl << res << std::endl;
}
