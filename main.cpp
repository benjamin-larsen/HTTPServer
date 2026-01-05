#include "./TCPServer.cpp"

int main() {
    TCP::StartServer("127.0.0.1", 6000);

    return 0;
}