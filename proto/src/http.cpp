#include "http.h"
#include "flog.h"

#include <sstream>

static constexpr std::string_view methods[] = {
#define XX(num, name, string) #name,
    HTTP_METHOD_MAP(XX)
#undef XX
    "None",
};

static const std::string_view& GetMethodName(HttpMethod method) {
    return methods[static_cast<int>(method)];
}
static HttpMethod GetMethodEnum(const std::string& method_name) {
    int n = sizeof(methods) / sizeof(std::string_view);
    // LOG_INFO << "n = " << n << "\n";
    for (int i = 0; i < n; ++i) {
        if (std::string(methods[i]) == method_name)
            return static_cast<HttpMethod>(i);
    }
    return HttpMethod::None;
}

static std::string strip(const std::string& str, char c) {
    std::string::size_type l = 0, r = str.size() - 1;
    while (l < str.size() && str[l] == c) ++l;
    while (r >= l && str[r] == c) --r;
    return str.substr(l, r - l + 1);
}

std::pair<std::string, std::string> HttpHeader::get_pair(const std::string& line) {
    // Host: www.exmaple.com
    //   Host   :   www.example.com   
    std::string::size_type n = line.find(':');
    if (n == std::string::npos) {
        LOG_ERROR << "Bad header!\t" 
                  << "header line = " << line << "\n";
        return {{}, {}};
    }

    std::string key = line.substr(0, n);
    std::string value = line.substr(n + 1);
    key = strip(key, ' ');
    value = strip(value, ' ');
    return {key, value};
}

HttpHeader HttpHeader::get_header_from_string(const std::string& str) {
    // LOG_INFO << "header = " << str << "\n";
    if (str.empty()) {
        return {};
    }
    HttpHeader header;
    // n 总是定位到 "\r\n"
    std::string::size_type n = 0, last = 0;
    while (n < str.size()) {
        last = n;
        n = str.find("\r\n", last);
        if (n == std::string::npos)
            break;
        std::string line = str.substr(last, n - last);
        header.insert(HttpHeader::get_pair(line));
        n += 2;
    }
    header.insert(HttpHeader::get_pair(str.substr(last)));
    return header;
}

std::string_view HttpBody::parse(const std::string& cnt_type) {
    if (cnt_type.empty()) {
        LOG_ERROR << "Content-Type is empty!\n";    
        return {};
    }
    return {};
}

std::string HttpRequest::to_string() {
    std::stringstream ss;
    // status line
    ss << GetMethodName(get_status_line().method) << ' ' 
       << get_status_line().uri << ' ' 
       << get_status_line().version << "\r\n";

    // request headers
    for (auto p : get_header()) {
        ss << p.first << ": " 
           << p.second << "\r\n";
    }
    ss << "\r\n";

    if (get_body().content_length() > 0) {
        ss << get_body().parse(std::string(get_header()["Content-Type"])) << "\r\n";
    }

    return ss.str();
}
HttpRequest HttpRequest::from_string(const std::string& str) {
    std::string::size_type end_pos_of_status_line = str.find("\r\n");
    if (end_pos_of_status_line == std::string::npos) {
        LOG_ERROR << "Bad Http Request! Status line!\n";
        return HttpRequest(HttpMethod::None, "");
    }
    std::string s_status_line = str.substr(0, end_pos_of_status_line);
    // status line
    std::string::size_type n1, n2;
    n1 = s_status_line.find(' ');
    HttpMethod method = GetMethodEnum(s_status_line.substr(0, n1));
    n2 = s_status_line.find(' ', n1 + 1);
    std::string uri = s_status_line.substr(n1 + 1, n2 - n1 - 1);
    std::string version = s_status_line.substr(n2 + 1);
    if (version != "HTTP/1.1") {
        LOG_ERROR << "The protocal is not HTTP/1.1!\n"
                  << "version = " << version << "\n";
        return HttpRequest(HttpMethod::None, "");
    }

    std::string::size_type end_pos_of_headers = str.find("\r\n\r\n", 
            end_pos_of_status_line);
    if (end_pos_of_headers == std::string::npos) {
        LOG_ERROR << "Bad Http Request!\n";
        return HttpRequest(HttpMethod::None, "");
    } else if (end_pos_of_status_line == end_pos_of_headers) {
        return HttpRequest(method, uri);
    }
    std::string s_headers = str.substr(end_pos_of_status_line + 2, 
            end_pos_of_headers - end_pos_of_status_line - 2);

    std::string::size_type start_pos_of_body = end_pos_of_headers + 2 + 2;
    std::string s_body = str.substr(start_pos_of_body);


    // deal with headers
    HttpHeader headers = HttpHeader::get_header_from_string(s_headers);
    
    if (s_body.empty()) {
        return HttpRequest(method, uri, headers);
    }

    // deal with body
    return HttpRequest(method, uri, headers, s_body);
}