#include <gtest/gtest.h>
#include "address.h"

namespace {

TEST(Address, HostService) {
    {
        Address addr("www.bing.com", "http");
        EXPECT_EQ(addr.port(), 80);
    }

    {
        // host name and numeric port
        Address addr("www.bing.com", 80);
        EXPECT_EQ(addr.port(), 80);
    }

    {
        // host name and string port
        Address addr("www.bing.com", "80");
        EXPECT_EQ(addr.port(), 80);
    }

    {
        Address addr("www.bing.com", "https");
        EXPECT_EQ(addr.port(), 443);
    }

    {
        // dotted-quad ip and string port
        Address addr("202.89.233.100", "443");
        EXPECT_EQ(addr.port(), 443);
    }

    {
        // dotted-quad ip and numeric port
        Address addr("202.89.233.100", 443);
        EXPECT_EQ(addr.port(), 443);
    }

    {
        // dotted-quad ip and service
        Address addr("202.89.233.100", "https");
        EXPECT_EQ(addr.port(), 443);
    }
}

} // namespace
