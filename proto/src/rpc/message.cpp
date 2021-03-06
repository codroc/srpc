#include "rpc/message.h"

namespace srpc {
namespace rpc {
    
std::map<std::string, srpc::rpc::BaseMessage::ptr>& GetInstanceMap() {
    static std::map<std::string, srpc::rpc::BaseMessage::ptr> g_instance_map;
    return g_instance_map;
}

BaseMessage::ptr MessageFactory(const std::string& type_name) {
    return GetInstanceMap()[type_name]->new_instance();
}

}
}
