// cpputils
#include "flog.h"
#include "config.h"

// srpc
#include "socket.h"
#include "ssl_socket.h"
#include "address.h"
#include "http.h"

// urldecoder
#include "urldecoder.h"

// rapidjson
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/istreamwrapper.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <functional>
#include <csignal>
#include <filesystem>

namespace fs = std::filesystem;
using namespace rapidjson;

class Router {
public:
    using FuncType = std::function<void (TCPSocket*)>;
    using FuncTypeInternal = std::function<void (TCPSocket*, const std::string&)>;
    using MapType = std::unordered_map<std::string, FuncType>;
    using MapTypeInternal = std::unordered_map<std::string, FuncTypeInternal>;
    Router() = default;

    void parse(const HttpRequest& req, TCPSocket* sock);

    static void RegistMethod(const std::string& uri, FuncType method);
    static void RegistMethod(const std::string& uri, FuncTypeInternal method);
private:
    static std::string GetPath(const std::string& uri) {
        // /v1/list/xxx
        if (uri.substr(0, 9) == "/v1/list/") {
            // 判断是否为目录
            if (fs::status(uri.substr(4)).type() == fs::file_type::directory)
                return "/"; // 发送 hello.html
            else if (fs::status(uri.substr(4)).type() == fs::file_type::regular) {
                // 如果是文件
                return uri.substr(3);
            }
        }
        return uri;
    }
private:
    static MapType _mp;
    static MapTypeInternal _mp_internal;
};

Router::MapType Router::_mp;
Router::MapTypeInternal Router::_mp_internal;

void Router::parse(const HttpRequest& req, TCPSocket* sock) {
    std::string uri = req.get_status_line().uri;
    std::string path = GetPath(uri);
    // 如果是上传
    if (req.get_status_line().method == HttpMethod::POST and 
            uri == "/upload") {
        auto method = _mp_internal["/upload"];
        method(sock, req.get_body().as_string());
        return;
    }

    // 下载或获取书单
    if (_mp.find(path) != _mp.end()) {
        auto method = _mp[path];
        method(sock);
        return;
    }

    // 展现书单或下载书籍
    path = path.substr(1); // /list => list or /list/c++ => list/c++
    if (fs::status(path).type() == fs::file_type::directory) {
        _mp_internal["/internal/send_book_list_json"](sock, path);
    } else if(fs::status(path).type() == fs::file_type::regular) {
        _mp_internal["/internal/download"](sock, path);
    }
    
}

void Router::RegistMethod(const std::string& path, Router::FuncType method) {
    _mp[path] = method;
}

void Router::RegistMethod(const std::string& path, Router::FuncTypeInternal method) {
    _mp_internal[path] = method;
}

void SigChildHandler(int signum) {
    int ret;
    while ((ret = ::waitpid(0, 0, WNOHANG)) > 0);
}

void init() {
    Logger::setLevel(Logger::LEVEL::WARN);
    Logger::SetBufferLevel(Logger::kLineBuffer);
    std::signal(SIGCHLD, SigChildHandler);

    // assert(fs::status(path) == fs::file_type::regular);
    auto router_send_book = [](TCPSocket* sock, const std::string& path){
        HttpResponse res(HttpStatus::OK);
        res.get_header().insert({"Content-Type", "text/html;charset=utf-8"});

        std::stringstream ss;
        std::ifstream input;
        input.open(path);
        ss << input.rdbuf();
        std::string content = ss.str();
        res.set_body(content);
        res.get_header().insert({"Content-Length", std::to_string(content.size())});

        sock->send(res.to_string());
    };

    auto router_send_book_list_json = [](TCPSocket* sock, const std::string& path){
        HttpResponse res(HttpStatus::OK);
        res.get_header().insert({"Content-Type", "application/json;charset=utf-8"});

        Document d;
        d.SetObject();
        Value value_str(kStringType);
        Value value_str_array(kArrayType);

        for (auto it = fs::directory_iterator(path); it != fs::directory_iterator(); ++it) {
            fs::path pp = *it;
            std::string filename = pp.filename().c_str();
            value_str.SetString(filename.c_str(), filename.size(), d.GetAllocator()); 
            value_str_array.PushBack(value_str, d.GetAllocator());
        }
        d.AddMember("list", value_str_array, d.GetAllocator());

        StringBuffer sb;
        Writer<StringBuffer> writer(sb);
        d.Accept(writer);
        std::string json_data = sb.GetString();
        res.set_body(json_data);
        res.get_header().insert({"Content-Length", std::to_string(json_data.size())});

        LOG_INFO << "json_data: " << json_data << "\n";
        sock->send(res.to_string());
    };

    auto router_send_book_list = [](){

    };

    Router::RegistMethod("/v1/list", Router::FuncType(std::bind(router_send_book, std::placeholders::_1, "list.js")));
    Router::RegistMethod("/", Router::FuncType(std::bind(router_send_book, std::placeholders::_1, "hello.html")));
    Router::RegistMethod("/internal/send_book_list_json", router_send_book_list_json);

    auto router_download_book = [](TCPSocket* sock, const std::string& bookname) {
        HttpResponse res(HttpStatus::OK);
        res.get_header().insert({"Content-Type", "application/x-download"});
        std::stringstream ss;
        std::ifstream input;
        input.open(bookname);
        ss << input.rdbuf();
        std::string content = ss.str();
        res.set_body(content);
        res.get_header().insert({"Content-Length", std::to_string(content.size())});

        std::string msg = res.to_string();
        size_t written = 0;
        while (written < msg.size()) {
            written += sock->send(msg.substr(written));
        }
    };
    Router::RegistMethod("/internal/download", std::bind(router_download_book, std::placeholders::_1, std::placeholders::_2));

    auto router_api = [](TCPSocket* sock) {
        HttpResponse res(HttpStatus::OK);
        res.get_header().insert({"Content-Type", "application/json;charset=utf-8"});

        Document d;
        d.SetObject();
        d.AddMember("书单分类", "/", d.GetAllocator());
        d.AddMember("上传书籍", "/v1/upload", d.GetAllocator());

        StringBuffer sb;
        Writer<StringBuffer> writer(sb);
        d.Accept(writer);
        std::string json_data = sb.GetString();
        res.set_body(json_data);
        res.get_header().insert({"Content-Length", std::to_string(json_data.size())});

        // LOG_INFO << "json_data: " << json_data << "\n";
        sock->send(res.to_string());
        
    };
    Router::RegistMethod("/v1/api", Router::FuncType(router_api));

    Router::RegistMethod("/v1/upload", Router::FuncType(std::bind(router_send_book, std::placeholders::_1, "upload.html")));

    auto router_upload = [](TCPSocket* sock, const std::string& content) {
        // 1. 解析内容，它是 multipart/form-data 格式的
        // 2. 把内容保存到文件里
        // 服务器仅支持 pdf 上传

        HttpResponse res(HttpStatus::OK);
        res.get_header().insert({"Content-Type", "text/plain;charset=utf-8"});
        res.set_body("okokok\n");
        res.get_header().insert({"Content-Length", "7"});

        sock->send(res.to_string());
    };
    Router::RegistMethod("/upload", router_upload);
}

bool IsComplete(const std::string& msg) {
    // 出现 \r\n\r\n 了，就表示 HTTP 头结束了，接下来的是 HTTP body，
    // body 的长度可以根据 header 中 content-length 字段值来确定；如果
    // 在传输时不能确定长度，应该使用 transfer-encoding: chunked
    // 如果传输的是表单，就用 content-type: multipart/form-data
    std::string::size_type n = msg.find("\r\n\r\n");
    if (n == std::string::npos) return false;
    HttpRequest req = HttpRequest::from_string(msg.substr(0, n + 2 + 2));
    std::string len(req.get_header()["content-length"]);
    if (len.empty()) {
        HttpMethod m = req.get_status_line().method;
        if (m == HttpMethod::GET) return true;
        else {
            if (m == HttpMethod::POST) {
                if (req.get_header()["Content-Type"] == "multipart/form-data"){
                    LOG_ERROR << "Not support form-data now.\n";
                    ::exit(1);
                } else if (req.get_header()["Transfer-Encoding"] == "chunked") {
                    LOG_ERROR << "Not support chunked now.\n";
                    ::exit(1);
                } else {
                    LOG_ERROR << "Unknow!\n";
                    ::exit(1);
                }
            } else {
                LOG_ERROR << "Not support " << static_cast<int>(m) << " now.\n";
                ::exit(1);
            }
        }
    }
    
    int l = std::atoi(len.c_str());
    if (msg.size() - req.to_string().size() > l) {
        LOG_ERROR << "Received more than one package!\n";
        ::exit(1);
    } else if (msg.size() - req.to_string().size() < l) return false;
    return true;
}

int main(int argc, char** argv) {
    init();
    if (argc < 2) {
        std::cout << "usage: ./https_ft http\n";
        std::cout << "usage: ./https_ft https config.yaml\n";
        return 1;
    }
    if (argc == 3)
        Config::loadFromYaml(argv[2]);
    std::string protocal(argv[1]);

    SocketOptions opts;
    opts.reuseaddr = true;
    opts.blocking = true;
    opts.use_ssl = protocal == "https";

    TCPSocket serv(opts);
    uint16_t port = protocal == "https" ? 443 : 80;
    serv.bind("0.0.0.0", port);

    serv.listen();

    while (1) {
        TCPSocket client = serv.accept();
        pid_t pid = ::fork();
        if (pid == 0) {
            // 目前仅支持客户端从服务器下载一个文件，不支持上传一个文件
            // 客户端发起一个短连接，发送完一个 HTTP package 后即主动断开连接
            // tcp 收到的 byte stream
            std::string byte_stream;
            while (!IsComplete(byte_stream)) {
                byte_stream += client.recv();
                // std::cout << byte_stream;
            }
            HttpRequest req = HttpRequest::from_string(byte_stream);
            // 当前服务器仅支持 GET method
            HttpMethod m = req.get_status_line().method;
            if (m == HttpMethod::GET) {
                // uri 中包含需要调用的方法以及传入的参数
                Router r;
                r.parse(req, &client);
                return 0;
            } else if (m == HttpMethod::POST) {
                Router r;
                r.parse(req, &client);
                return 0;
            } else {
                LOG_INFO << "Invaild http request: \n" << 
                    req.to_string();
                    return 1;
            }
        }
        client = std::move(TCPSocket());
    }
    
    return 0;
}
