#ifndef SRPC_PROTO_PACKAGE_EXTRACTOR
#define SRPC_PROTO_PACKAGE_EXTRACTOR

#include "http.h"

#include <string>
#include <string_view>
#include <vector>
#include <cassert>

template <class P>
class PackageExtractor {
public:
    // brief: 判断一个消息是否存在边界
    virtual std::pair<bool, std::string::size_type> has_bound(std::string_view msg, std::string_view boundary) {
        return {{}, {}};
    };

    // brief: 从消息中提取出所有的 package
    virtual std::vector<P> extract(const std::string& msg) {
        return {};
    };
};

class HttpPackageExtractor : public PackageExtractor<HttpRequest> {
public:
    // brief: 判断是否至少存在一个完整的 HttpHeader
    virtual std::pair<bool, std::string::size_type> has_bound(std::string_view msg, std::string_view boundary) override;

    // brief: 从消息中提取出所有的 Http package
    virtual std::vector<HttpRequest> extract(const std::string& msg) override;
};

#endif

std::pair<bool, std::string::size_type> HttpPackageExtractor::has_bound(std::string_view msg, std::string_view boundary) {
    return {msg.find(boundary) != std::string::npos, msg.find(boundary) + boundary.size()};
}

std::vector<HttpRequest> HttpPackageExtractor::extract(const std::string& msg) {
    // 对于 HTTP package 来说，HttpHeader 可以按
    // "\r\n\r\n" 来分辨；但是 HttpBody 就需要分情况了:
    // 1. 固定长度。根据 content-length 来确定。
    // 2. 特定格式 + 头部添加长度字段。即 transfer-encoding: chunked
    // 3. 特殊字符串表示边界。即 content-type: multipart/form-data
    // 4. GET 方法可以不存在 content-length

    std::string_view view(msg); // 可以不可直接通过 string 构造？
    std::vector<HttpRequest> ret;
    auto p = has_bound(view, "\r\n\r\n");
    while (p.first) {
        // 找到了一个完整的 header，以 "\r\n\r\n" 结尾
        HttpRequest req = HttpRequest::from_string(std::string{view.substr(0, p.second)});
        view.remove_prefix(p.second);
        if (!req.get_header()["content-length"].empty()) {
            if (req.get_header()["content-type"].find("multipart/form-data") != std::string::npos and 
                    req.get_status_line().method == HttpMethod::POST) {
                // situation 3
                std::string_view value = req.get_header()["content-type"];
                std::string boundary("boundary=");
                assert(value.find(boundary) != std::string::npos);
                req.boundary = value.substr(value.find(boundary) + boundary.size());

                // 找到 --${boundary}--
                auto res = has_bound(view, "--" + req.boundary + "--");
                if (res.first) { // 具有完整表单的 HTTP Request
                    req.construct_form_data_from_string(view.substr(0, res.second));
                    view.remove_prefix(res.second);
                    ret.push_back(req);
                } else break; // 默认如果包不完整，就不读取
            } else {
                // situation 1
                size_t len = std::atoll(req.get_header()["content-length"].data());
                if (len <= view.size()) {
                    // fix me: use string_view
                    std::string body(view.substr(0, len));
                    req.set_body(body);
                    ret.push_back(req);
                } else break;
            }
        } else if (!req.get_header()["transfer-encoding"].empty()) {
            // situation 2
        } else if (req.get_status_line().method == HttpMethod::GET) {
            // situation 4
            ret.push_back(req);
        }
        p = has_bound(view, "\r\n\r\n");
    }
    return ret;
}
