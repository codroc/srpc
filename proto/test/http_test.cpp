#include "http.h"
#include <gtest/gtest.h>

namespace {

TEST(HTTP, HttpRequest) {
    {
        // test: HttpRequest::to_string
        HttpHeader::MapType headers = {
            {"Host", "www.example.com"},
        };

        std::string str = 
            "GET / HTTP/1.1\r\n"
            "Host: www.example.com\r\n"
            "\r\n";

        HttpRequest request(HttpMethod::GET, "/", headers);
        EXPECT_EQ(str, request.to_string());
    }

    {
        // test: HttpRequest::from_string without body
        HttpHeader::MapType headers = {
            {"Host", "www.example.com"},
        };
        HttpRequest request(HttpMethod::GET, "/", headers);

        std::string str = 
            "GET / HTTP/1.1\r\n"
            "Host: www.example.com\r\n"
            "\r\n";
        // fix me:
        EXPECT_EQ(HttpRequest::from_string(str).to_string(), request.to_string());
    }

    {
        // test: HttpHeader::operator[]
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
