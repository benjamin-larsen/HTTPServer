#ifndef HTTP_TCPLIB
#define HTTP_TCPLIB

#include <boost/asio.hpp>
#include <boost/asio/ip/address.hpp>

namespace TCP {
    using namespace boost::asio::ip;

    void StartServer(const char *addrStr, int port) {
        auto addr = make_address(addrStr);
        tcp::endpoint endpoint(addr, port);

        boost::asio::io_context context;
        tcp::acceptor acceptor(context, endpoint);

        for (;;) {
            tcp::socket socket(context);
            acceptor.accept(socket);
        }

        context.run();
    }
}

#endif