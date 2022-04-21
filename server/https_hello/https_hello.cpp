#include "flog.h"
#include "http.h"
#include "socket.h"
#include "config.h"

#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <streambuf>

// brief: a simple server just provide a hello.html file to client.

// brief: 用来识别一个完整的 HTTP 包
bool IsComplete(const std::string& msg) {
    // 出现 \r\n\r\n 了，就表示 HTTP 头结束了，接下来的是 HTTP body，
    // body 的长度可以根据 header 中 content-length 字段值来确定；如果
    // 在传输时不能确定长度，应该使用 transfer-encoding: chunked
    std::string::size_type n = msg.find("\r\n\r\n");
    if (n == std::string::npos) return false;
    HttpRequest req = HttpRequest::from_string(msg.substr(0, n + 2 + 2));
    std::string len(req.get_header()["content-length"]);
    if (len.empty()) {
        LOG_ERROR << "No content-length field in Http header.\n";
        ::exit(1);
    }
    int l = std::atoi(len.c_str());
    if (msg.size() - req.to_string().size() > l) {
        LOG_ERROR << "Received more than one package!\n";
        ::exit(1);
    } else if (msg.size() - req.to_string().size() < l) return false;
    return true;
}

std::string data;
bool CacheFileData(const std::string& file) {
    std::filesystem::path path(file);
    std::filesystem::directory_entry entry(path);
    if (!entry.exists()) {
        LOG_INFO << "hello.html is not exists!\n";
        return false;
    }
    
    std::ifstream instream;
    instream.open("hello.html");

    std::stringstream ss;
    ss << instream.rdbuf();
    data = ss.str();
    return true;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "usage: ./https_hello config.yaml\n";
        return 0;
    }
    Config::loadFromYaml(argv[1]);

    SocketOptions opts;
    opts.blocking = true;
    opts.use_ssl = true;
    TCPSocket serv(opts);

    serv.bind("127.0.0.1", 443);
    serv.listen();
    
    if (!CacheFileData("hello.html"))
        return 1;
    while (1) {
        // 阻塞 accept
        TCPSocket sock = serv.accept();
        
        pid_t pid;
        if ((pid = fork()) == 0) {
            // 子进程，因为只发送一个 html 文件，因此不管对面什么要求
            // 只提供 http 短连接】
            std::string msg = sock.recv();
            
            // 具体要做到的是：分出一个完整的HTTP包，不要多也不要少；
            // 因为本服务仅仅发送 hello.html，不会给你建立长连接的机会的。
            // 这里 IsComplete 其实就是对一个完整的 HTTP 包的识别
            while (!IsComplete(msg)) { 
                msg += sock.recv();
            }
            HttpRequest  req = HttpRequest::from_string(msg);
            if (req.get_status_line().version != "HTTP/1.1") {
                LOG_ERROR << "We just support HTTP/1.1" << 
                    " but " << req.get_status_line().version <<
                    " received!\n";
                ::exit(1);
            } else if (req.get_status_line().method != HttpMethod::GET) {
                LOG_ERROR << "We just support GET method" << 
                    " but " << std::string(HttpStatusLine::get_method_name(req.get_status_line().method)) <<
                    " received!\n";
                ::exit(1);
            } else if (req.get_status_line().uri != "/") {
                LOG_ERROR << "We just support hello.html or /" << 
                    " but " << req.get_status_line().uri <<
                    " received!\n";
                ::exit(1);
            }

            HttpResponse res(HttpStatus::OK);
            res.set_body(data);

            sock.send(res.to_string());

            return 0; // 析构 sock，调用 ssl_shutdown, ssl_free, close
        }
        sock = std::move(TCPSocket());
    }
    return 0;
}
