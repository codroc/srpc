// cpputils
#include "flog.h"
#include "config.h"

// srpc
#include "socket.h"
#include "ssl_socket.h"
#include "address.h"
#include "http.h"
#include "package_extractor.h"
#include "encoder.h"

// rapidjson
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/istreamwrapper.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <fstream>
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
    using UploadFunc = std::function<void (TCPSocket*, const std::vector<HttpRequest::Form>&)>;
    Router() = default;

    void parse(const HttpRequest& req, TCPSocket* sock);

    static void RegistMethod(const std::string& uri, FuncType method);
    static void RegistMethod(const std::string& uri, FuncTypeInternal method);
    static void RegistMethod(const std::string& uri, UploadFunc method);
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
    static UploadFunc _upload_method;
};

Router::MapType Router::_mp;
Router::MapTypeInternal Router::_mp_internal;
Router::UploadFunc Router::_upload_method;

void Router::parse(const HttpRequest& req, TCPSocket* sock) {
    std::string uri = req.get_status_line().uri;
    uri = urlDecode(uri.c_str());
    std::string path = GetPath(uri);
    // 如果是上传
    if (req.get_status_line().method == HttpMethod::POST and 
            uri == "/upload") {
        _upload_method(sock, req.form_data);
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

void Router::RegistMethod(const std::string& uri, UploadFunc method) {
    _upload_method = method;
}

void SigChildHandler(int signum) {
    int ret;
    while ((ret = ::waitpid(0, 0, WNOHANG)) > 0);
}

void init() {
    Logger::getInstance()->setLevel(Logger::LEVEL::WARN);
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

        sock->send(res.to_string());
        
    };
    Router::RegistMethod("/v1/api", Router::FuncType(router_api));

    Router::RegistMethod("/v1/upload", Router::FuncType(std::bind(router_send_book, std::placeholders::_1, "upload.html")));

    auto router_upload = [](TCPSocket* sock, const std::vector<HttpRequest::Form>& form_data) {
        // 1. 解析内容，它是 multipart/form-data 格式的
        // 2. 把内容保存到文件里
        // 服务器仅支持 pdf 上传

        std::string content;
        // for (auto f : form_data) {
            auto f = form_data[0];
            std::string_view sv(f.header["content-disposition"]);
            std::string filename;
            // name="/v1/list"
            // filename="chrome.png"
            if (sv.find("name") != std::string::npos) {
                sv.remove_prefix(sv.find("name"));
                sv.remove_prefix(4 + 1 + 1 + 4);
                filename += std::string(sv.substr(0, sv.find("\""))) + "/";
            }
            if (sv.find("filename") != std::string::npos) {
                sv.remove_prefix(sv.find("filename"));
                sv.remove_prefix(8 + 1 + 1);
                filename += std::string(sv.substr(0, sv.find("\"")));
            }
            std::cout << "filename = " << filename << " value size: " << f.value.size() << "\n";
            std::ofstream output;
            output.open(filename, std::ofstream::binary);
            output << f.value;
        // }
        HttpResponse res(HttpStatus::OK);
        res.get_header().insert({"Content-Type", "text/plain;charset=utf-8"});
        res.set_body("okokok\n");
        res.get_header().insert({"Content-Length", "7"});

        sock->send(res.to_string());
    };
    Router::RegistMethod("/upload", router_upload);
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
            std::string msg = client.recv();
            std::string byte_stream = msg;
            HttpPackageExtractor pe;
            while (pe.extract(byte_stream).empty() && !msg.empty()) {
                // 这里浏览器有时候会莫名其妙建立 2 个连接，
                // 但只在一个连接上发送 HTTP Request，导致阻塞，见 srpc 开发日志
                //
                // fix me:
                // 这里存在严重的安全问题。如果对面只建立连接，不发数据，那么会产生大量阻塞进程，
                // 最终导致系统崩溃
                msg = client.recv();
                byte_stream += msg;
            }
            std::cout << "bytestream size = " << byte_stream.size() << "\tmsg size = " << msg.size() << "\n";
            auto vec = pe.extract(byte_stream);
            if (vec.empty()) {
                LOG_ERROR << "extract failed!\n";
                return 1;
            }
            HttpRequest req = vec[0]; // 短连接，只接受 一个 HttpRequest 包
            // 当前服务器仅支持 GET method
            HttpMethod m = req.get_status_line().method;
            if (m == HttpMethod::GET || m == HttpMethod::POST) {
                // uri 中包含需要调用的方法以及传入的参数
                Router r;
                r.parse(req, &client);
            } else {
                LOG_INFO << "Invaild http request: \n" << 
                    req.to_string();
            }
            return 0;
        }
        client = std::move(TCPSocket());
    }
    
    return 0;
}
