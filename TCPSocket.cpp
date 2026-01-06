#ifndef HTTP_TCPClientLIB
#define HTTP_TCPClientLIB

#include <memory>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

namespace TCP {
    using namespace boost::asio::ip;

    struct RequestInfo {
        std::string method;
        std::string path;
        std::string version;
    };

    struct RequestHeader {
        std::string key;
        std::string value;
    };

    RequestInfo DecodeRequestInfo(const std::string& line) {
        RequestInfo info;
        std::stringstream ss(line);

        std::getline(ss, info.method, ' ');
        std::getline(ss, info.path, ' ');
        std::getline(ss, info.version, ' ');

        return info;
    }

    RequestHeader NullHeader{};

    RequestHeader DecodeRequestHeader(const std::string& line) {
        RequestHeader header{};
        auto delimPos = line.find(':');

        if (delimPos == std::string::npos) return NullHeader;

        header.key = line.substr(0, delimPos);
        boost::algorithm::to_lower(header.key);

        auto vv = std::string_view(line).substr(delimPos + 1);

        auto leadingTrim = vv.find_first_not_of(" \t");
        auto trailingTrim = vv.find_last_not_of(" \t");

        if (leadingTrim == std::string::npos || trailingTrim == std::string::npos)
            return NullHeader;

        // This shouldn't happen, but just incase.
        if (trailingTrim < leadingTrim) return NullHeader;

        vv = vv.substr(leadingTrim, trailingTrim - leadingTrim + 1);
        header.value = std::string(vv);

        return header;
    }

    void TrimSuffixChar(std::string& str, const char c) {
        if (!str.empty() && str.back() == c) str.pop_back();
    }

    class Socket {
    private:
        boost::asio::streambuf buf;
        tcp::socket socket;
    public:
        Socket(tcp::socket socket) : socket(std::move(socket)) {
        }

        boost::asio::awaitable<std::string> ReadLine() {
            co_await async_read_until(socket, buf, '\n', boost::asio::use_awaitable);

            std::string line;
            std::istream is(&buf);

            std::getline(is, line);

            // Should be unlikely, but incase getline() for some reason included the newline
            TrimSuffixChar(line, '\n');

            // Trim carriage return
            TrimSuffixChar(line, '\r');

            co_return line;
        }


        boost::asio::awaitable<bool> ReadRequestHeaders() {
            auto reqInfo = DecodeRequestInfo(co_await ReadLine());

            printf("Method: %s\nPath: %s\nVersion: %s\n", reqInfo.method.c_str(), reqInfo.path.c_str(), reqInfo.version.c_str());

            if (reqInfo.path.empty() || reqInfo.path.front() != '/') co_return true;
            if (reqInfo.version != "HTTP/1.0" && reqInfo.version != "HTTP/1.1") co_return true;

            for (;;) {
                auto line = co_await ReadLine();

                if (line.empty()) break;

                auto [key, value] = DecodeRequestHeader(line);

                if (key.empty() || value.empty()) co_return true;

                printf("Key: %s\nValue: %s\n", key.c_str(), value.c_str());
            }

            printf("Request Done\n");

            co_return false;
        }
    };

    boost::asio::awaitable<void> SpawnSocket(tcp::socket tcpSocket) {
        auto socket = Socket(std::move(tcpSocket));

        try {
            for (;;) {
                auto shouldClose = co_await socket.ReadRequestHeaders();

                if (shouldClose) {
                    break;
                }
            }
        }
        catch (...) {}

        std::cout << "Socket Disconnected" << std::endl;

        co_return;
    }
}

#endif