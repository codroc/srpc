#include "rpc.h"
#include "flog.h"

namespace srpc {
namespace rpc {

RPCHeader::RPCHeader()
    : magic(get_magic_number())
    , type(RPCPackage::HeartBeat)
    , opt(false)
    , serialization(RPCPackage::Local)
    , status(RPCPackage::Ok)
    , data_length(0)
    , checksum(0)
{}

uint16_t RPCHeader::get_magic_number() {
    return 0xabcd;
}

void RPCOption::set(const char* opt, uint8_t length) {
    if (length > RPC_OPTIONSIZE) {
        LOG_ERROR << "Set fail: RPC_OPTIONSIZE = " << RPC_OPTIONSIZE << ", but length = " << length << "\n";
        return;
    }
    data = (char*)::malloc(RPC_OPTIONSIZE);
    ::memcpy(data, opt, length);
}

RPCPackage::RPCPackage()
    : _header()
    , _opt()
    , _bytes()
{}

RPCPackage::RPCPackage(Type type, SerializationType se_type)
    : _header()
    , _opt()
    , _bytes()
{
    _header.type = type;
    _header.serialization = se_type;
}

void RPCPackage::set_option(const char *opt, uint8_t len) {
    _header.opt = true;
    _opt.setNewOpt(opt, len);
}

void RPCPackage::set_body(const char* data, int len) {
    _bytes = std::string(data, len);
    _header.data_length = _bytes.size();
}

std::string RPCPackage::to_string() const {
    std::string header(reinterpret_cast<const char*>(this), sizeof(RPCHeader));
    std::string opt;
    if (has_opt()) {
        opt = std::string(_opt.data, RPC_OPTIONSIZE);
    }
    return header + opt + _bytes;
}
RPCPackage RPCPackage::from_string(const std::string& str) {
    RPCPackage package;
    int header_len = sizeof(RPCHeader);
    std::string_view sv = str;
    std::string_view sv_header = sv.substr(0, header_len);
    sv.remove_prefix(header_len);

    RPCHeader header;
    ::memcpy(&header, sv_header.data(), sv_header.size());
    package.set_header(header);
    if (package.has_opt()) {
        std::string_view sv_opt = sv.substr(0, RPC_OPTIONSIZE);
        sv.remove_prefix(RPC_OPTIONSIZE);
        package.set_option(sv_opt.data(), sv_opt.size());
    }
    package.set_body(sv.data(), sv.size());
    
    return package;
}

}
}
