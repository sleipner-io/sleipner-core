sleipner-core
--------------------
Sleipner-io aims to provide an easy-to-use, cross-platform communications library for C++ and Python. The **core** of the library provides wrappers for various transport protocols, including *TCP*, *USB* and *Bluetooth*.

Scope
--------------------
This library is implemented in C++ and will have added python bindings. It will work on **Windows** and **POSIX** compliant machines. It provides a *socket*-like interface for IO operations, and may in the future add tools for encryption/security.

Requirements
--------------------
1) C++17 compiler
2) CMake 3.12+

Roadmap
--------------------
1) Implement **BluetoothSocket** and **UsbSocket**
2) Implement **Python** library
3) Implement all on **Linux**
