#include "parser.h"

#include "rapidjson/document.h"

JSONObject parse_json(JSONType, const std::string& buf) {
    Document d;
    d.Parse(buf.c_str());
    return d;
}
