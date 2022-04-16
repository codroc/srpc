#ifndef SRPC_PROTO_PARSER_H
#define SRPC_PROTO_PARSER_H

#include <string>

class Parser {
public:
    bool parse(const std::string& content_type, const std::string& buf);
};

// template <class T>
// T parse(const std::string& content_type, const std::string& buf);

// text
// text/html
using HTMLObject = ...;
HTMLObject parse_html(HTMLType, const std::string& buf);

// text/plain
std::string parse_html(const std::string& buf);

// text/json or application json
using JSONObject = ...;
JSONObject parse_json(JSONType, const std::string& buf);

// text/xml or application/xml
using XMLObject = ...;
XMLObject parse_xml(XMLType, const std::string& buf);

// image
// image/gif
using GIFObject = ...;
GIFObject parse_gif(GIFType, const std::string& buf);

// image/jpeg
using JPEGObject = ...;
JPEGObject parse_jpeg(JPEGType, const std::string& buf);

// image/png
using PNGObject = ...;
PNGObject parse_jpeg(PNGType, const std::string& buf);

// application
// application/xml
// application/json

#endif

