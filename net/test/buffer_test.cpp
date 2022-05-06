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

TEST(Buffer, Number) {
    int32_t val = 108;
    Buffer buf;
    buf.append_int32(val); // 108
    buf.append_int32(val + 1); // 108109

    EXPECT_EQ(buf.peek_int32(), val);

    buf.retrieve(sizeof val); // 108
    EXPECT_EQ(buf.peek_int32(), val + 1);
    buf.retrieve(sizeof val); //

    EXPECT_TRUE(buf.empty());

    buf.append_int32(-1);
    EXPECT_EQ(buf.peek_int32(), -1);
}

TEST(Buffer, Expansion) {
    string str(100, 'c');
    int capacity = 500;
    Buffer buf(capacity);
    EXPECT_TRUE(buf.empty());
    
    buf.append(str);
    buf.retrieve(20);
    buf.retrieve(80);

    EXPECT_EQ(buf.remain_capacity(), capacity);

    string str1(88, '8');
    buf.append(str1);
    EXPECT_EQ(str1, buf.peek_all());
    
    string str2(420, '9');
    buf.append(str2);
    EXPECT_EQ(str1 + str2, buf.retrieve_all());
    EXPECT_TRUE(buf.empty());
}

}
