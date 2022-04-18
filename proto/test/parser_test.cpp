#include "parser.h"
#include <gtest/gtest.h>

#include <iostream>

namespace {

TEST(Parser, Json) {
    const char* json = "{\"key\":\"value\", \"hello\":\"world\"}";
    auto obj = parser::parse_json(json);
    ASSERT_STREQ("value", obj["key"].GetString());
    ASSERT_STREQ("world", obj["hello"].GetString());
}

} // namespace
