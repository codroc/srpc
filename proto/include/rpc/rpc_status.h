#ifndef SRPC_PROTO_RPC_STATUS_H
#define SRPC_PROTO_RPC_STATUS_H

namespace srpc {
namespace rpc {

class Status {
public:
    enum St {
        Ok = 1,
        Timeout = 2,
        Error = 3,
    };

    Status()
        : _status(Ok)
    {}
    Status(St s, const std::string& msg)
        : _status(s),
         _error_message(msg)
    {}
    ~Status() = default;

    bool ok() const { return _status == Ok; }
    std::string errstr() const { return _error_message; }
private:
    St _status;
    std::string _error_message;
};

}
}

#endif
