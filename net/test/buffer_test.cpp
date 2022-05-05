#include <gtest/gtest.h>
#include <string>
#include "buffer.h"
using namespace std;

namespace {

TEST(Buffer, Normal) {
    string str(100, 'c');
    Buffer buf;
    EXPECT_TRUE(buf.empty());

    buf.append(str);
    EXPECT_EQ(buf.readable(), str.size());

    EXPECT_EQ(buf.peek(10), string(10, 'c'));

    buf.retrieve(10);
    EXPECT_EQ(buf.readable(), str.size() - 10);

    EXPECT_EQ(buf.retrieve_all(), string(100 - 10, 'c'));

    EXPECT_TRUE(buf.empty());
}

}
