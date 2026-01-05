#include "./TCP.cpp"

int main() {
    TCP::StartServer("127.0.0.1", 5000);

    return 0;
}