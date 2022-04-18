#include "parser.h"

namespace parser {

std::string parse_plain(const std::string& buf) { return buf; }

JSONObject parse_json(const std::string& buf) {
    rapidjson::Document d;
    d.Parse(buf.c_str());
    return d;
}

} // namespace parser
