#include "http.h"
#include "flog.h"
#include "socket.h"

#include <unistd.h>

#include <string>
#include <iostream>
#include <unordered_map>

std::pair<std::string, std::string> GetHostAndUri(const std::string& url) {
    std::string host_name;
    std::string uri;

    std::string::size_type n = url.find(':');
    if (n != std::string::npos) {
        // 1. 能够找到 ':' 符号，类似于 https://www.bing.com/?.*
        if (url.size() < n + 3) {
            LOG_ERROR << "Bad url [" << url << "]!\n";
            ::exit(1);
        } else if (url[n + 1] != '/' || url[n + 2] != '/') {
            LOG_ERROR << "Bad url [" << url << "]!\n";
            ::exit(1);
        }
        n += 3;
        if (url.find('/', n) == std::string::npos) {
            host_name = url.substr(n);
            uri = "/";
        }
        else {
            std::string::size_type pos = url.find('/', n); // position of '/' after host, before uri
            host_name = url.substr(n, pos - n);
            uri = url.substr(pos);
        }

    } else {
        // 2. 没有 ':' 符号，直接是 www.bing.com/?.*
        if (url.find('/') == std::string::npos) {
            host_name = url;
            uri = "/";
        }
        else {
            std::string::size_type pos = url.find('/');
            host_name = url.substr(0, pos);
            uri = url.substr(pos);
        }
    }
    return {host_name, uri};
}

int main(int argc, char** argv) {
    // usage: ./wget http://www.bing.com/
    if (argc < 2) {
        LOG_INFO << "usage: ./wget http://www.bing.com/\n";
        return 0;
    }
    std::string url = argv[1];
    std::string::size_type n = url.find(':');
    std::string protocal = "http"; // default protocal is http/1.1
    if (n != std::string::npos) {
         protocal = url.substr(0, n);
    }

    if (protocal == "http") {
        auto p = GetHostAndUri(url);
        SocketOptions ops;
        ops.blocking = true;

        // 阻塞 socket
        TCPSocket sock(ops);
        sock.connect(p.first, "http");

        HttpHeader::MapType header = {
            {"host", p.first},
            {"user-agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/100.0.4896.127 Safari/537.36"},
            {"connection", "close"},
        };
        HttpRequest req(HttpMethod::GET, p.second, header);
        sock.send(req.to_string());
        std::string recv_msg;
        std::string msg = sock.recv();
        while (!msg.empty()) {
            recv_msg += msg;
            msg = sock.recv();
        }
        std::cout << "HttpRequest:  -----------------------------------------------------------------------\n";
        std::cout << req.to_string();
        std::cout << "HttpResponse: -----------------------------------------------------------------------\n";
        std::cout << recv_msg;
        std::cout << "-----------------------------------------------------------------------\n";
    } else if (protocal == "https") {
        LOG_INFO << "Don't support https now.\n";
        return 0;
    } else {
        LOG_INFO << "Unknow protocal [" << protocal << "]!\n";
        return 1;
    }
    return 0;
}
