#include "sleipner/sys/error.hpp"

#include <string>
#include <iostream>

int main(int argc, char* argv[]) {
    if ( argc < 2 )
        throw std::runtime_error("Input a numeric error code to check the value for!");

    int code = std::stoi(argv[1]);

    std::cout << sleipner::sys::error_message(code) << std::endl;
}
