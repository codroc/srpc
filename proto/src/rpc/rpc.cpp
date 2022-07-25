#include "rpc/rpc.h"
#include "rpc/serialize.h"
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
{}

RPCPackage::RPCPackage(Type type, SerializationType se_type)
    : _header()
    , _opt()
{
    _header.type = type;
    _header.serialization = se_type;
}

void RPCPackage::set_option(const char *opt, uint8_t len) {
    _header.opt = true;
    _opt.setNewOpt(opt, len);
}

std::string RPCPackage::to_string() const {
    std::string header(reinterpret_cast<const char*>(this), sizeof(RPCHeader));
    std::string opt;
    if (has_opt()) {
        opt = std::string(_opt.data, RPC_OPTIONSIZE);
    }
    return header + opt + _rpc_method.to_string() + _msg->to_string();
}

RPCPackage RPCPackage::from_string(const std::string& str) {
    RPCPackage package;

    // set header
    int header_len = sizeof(RPCHeader);
    std::string_view sv = str;
    std::string_view sv_header = sv.substr(0, header_len);
    sv.remove_prefix(header_len);

    RPCHeader header;
    ::memcpy(&header, sv_header.data(), sv_header.size());
    package.set_header(header);

    // set option
    if (package.has_opt()) {
        std::string_view sv_opt = sv.substr(0, RPC_OPTIONSIZE);
        sv.remove_prefix(RPC_OPTIONSIZE);
        package.set_option(sv_opt.data(), sv_opt.size());
    }

    // set rpc method
    package.set_rpc_method(RPCMethod::from_string(sv));
    // std::cout << package.get_rpc_method() << std::endl;

    // set message
    package.set_message_from_string_with_rpc_method(sv);
    
    return package;
}

void RPCPackage::set_message_from_string_with_rpc_method(const std::string& rpc_method_then_message) {
    Serialize de(Serialize::DESERIALIZER, rpc_method_then_message);
    // skip service name and method name
    de.readString();
    de.readString();

    std::string args_type = de.readString();
    std::string reply_type = de.readString();

    std::string remain = de.peek_all();
    if (is_heartbeat()) {
    } else if (is_request()) {
        auto tmp = MessageFactory(args_type);
        _msg = tmp->from_string(remain);
    } else if (is_response()) {
        auto tmp = MessageFactory(reply_type);
        _msg = tmp->from_string(remain);
    }
}

void RPCPackage::set_message_from_string_with_rpc_method(std::string_view rpc_method_then_message) {
    set_message_from_string_with_rpc_method(std::string(rpc_method_then_message.data(), rpc_method_then_message.size()));
}

}
}

std::ostream& operator<<(std::ostream& os, const srpc::rpc::RPCMethod& rpc_method) {
    os << "service name: " << rpc_method.service_name << "\n"
       << "method name : " << rpc_method.method_name << "\n"
       << "args type   : " << rpc_method.args_type << "\n"
       << "reply type  : " << rpc_method.reply_type << "\n";
    return os;
}
