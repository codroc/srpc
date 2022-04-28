#include "http.h"
#include <gtest/gtest.h>

namespace {

TEST(HTTP, HttpRequest) {
    {
        // test: HttpRequest::to_string 
        // Note: without headers
        HttpHeader::MapType headers = {
        };

        std::string str = 
            "GET / HTTP/1.1\r\n"
            "\r\n";

        HttpRequest request(HttpMethod::GET, "/", headers);
        EXPECT_EQ(str, request.to_string());
    }
}

TEST(HTTP, HttpRequestWithHeader) {
    
    {
        // test: HttpRequest::to_string 
        // Note: with headers but without body
        HttpHeader::MapType headers = {
            {"Host", "www.example.com"},
        };

        std::string str = 
            "GET / HTTP/1.1\r\n"
            "host: www.example.com\r\n"
            "\r\n";

        HttpRequest request(HttpMethod::GET, "/", headers);
        EXPECT_EQ(str, request.to_string());
    }
}

TEST(HTTP, HttpRequestWithoutBody) {
    {
        // test: HttpRequest::from_string 
        // Note: without body
        HttpHeader::MapType headers = {
            {"Host", "www.example.com"},
            {"Content-Length", "5"}, 
        };
        HttpRequest request(HttpMethod::GET, "/", headers);

        std::string str = 
            "GET / HTTP/1.1\r\n"
            "Host: www.example.com\r\n"
            "Content-Length: 5\r\n"
            "\r\n";
        HttpRequest req = HttpRequest::from_string(str);

        EXPECT_EQ(req.get_status_line().method, HttpMethod::GET);
        EXPECT_EQ(req.get_status_line().uri, "/");
        EXPECT_EQ(req.get_status_line().version, "HTTP/1.1");

        EXPECT_EQ(req.get_header()["Host"], headers["Host"]);
        EXPECT_EQ(req.get_header()["Content-Length"], headers["Content-Length"]);
        EXPECT_EQ(req.get_body().as_string(), "");
    }
}

TEST(HTTP, HttpRequestOperator) {
    {
        // test: HttpHeader::operator[]
        // Note: without body
        HttpHeader::MapType headers = {
            {"Host", "www.example.com"},
            {"Content-Type", "text/html"},
        };

        std::string str = 
            "GET / HTTP/1.1\r\n"
            "Host: www.example.com\r\n"
            "Content-Type: text/html\r\n"
            "\r\n";

        HttpRequest request(HttpMethod::GET, "/", headers);
        EXPECT_EQ("www.example.com", request.get_header()["Host"]);
        EXPECT_EQ("text/html", request.get_header()["Content-Type"]);
    }
}

} // namespace
