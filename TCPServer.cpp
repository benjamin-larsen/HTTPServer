#ifndef HTTP_TCPServerLIB
#define HTTP_TCPServerLIB

#include <thread>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ip/address.hpp>
#include "./TCPSocket.cpp"

namespace TCP {
    using namespace boost::asio::ip;

    void StartWorker(int id, boost::asio::io_context* context) {
        printf("Starting worker: %i\n", id);
        context->run();
        printf("Quitting worker: %i\n", id);
    }

    class Server {
    private:
        boost::asio::io_context context;
        decltype(boost::asio::make_work_guard(context)) workGuard;
        tcp::acceptor acceptor;
        tcp::endpoint endpoint;

        void cleanup() {
            boost::system::error_code ec;
            acceptor.close(ec);
            workGuard.reset();
        }
    public:
        Server(const char *addrStr, const int port) : workGuard(boost::asio::make_work_guard(context)), acceptor(context) {
            const auto addr = make_address(addrStr);
            endpoint = tcp::endpoint(addr, port);

            // future note: add threads to destructor
            // not necessary right now as the lifetime of Server is expected to be the Program Lifetime
            for (auto i = 0; i < std::thread::hardware_concurrency(); i++) {
                std::thread t(StartWorker, i, &context);
                t.detach();
            }
        }

        void ListenAndServe() {
            acceptor.open(endpoint.protocol());
            acceptor.set_option(tcp::acceptor::reuse_address(false));
            acceptor.bind(endpoint);
            acceptor.listen();

            for (;;) {
                try {
                    tcp::socket socket(context);
                    acceptor.accept(socket);
                    std::cout << "Socket Connected 2" << std::endl;
                } catch (...) {
                    std::cout << "Socket Error" << std::endl;
                }
            }

            cleanup();
        }
    };

    void StartServer(const char *addrStr, const int port) {
        auto server = new Server(addrStr, port);
        server->ListenAndServe();
    }
}

#endif