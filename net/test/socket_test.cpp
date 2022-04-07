#include <gtest/gtest.h>
#include "address.h"
#include "socket.h"
namespace {

TEST(Socket, UDPSocket) {
    {
        // brief: test udp socket bind to an address
        UDPSocket sock;
        Address addr("127.0.0.1", 7890);
        sock.bind(addr);
    }

    {
        // brief: test call bind by (ip, port)
        UDPSocket sock;
        sock.bind("127.0.0.1", 7890);

        // sock.bind("127.0.0.1", "7890");
    }

    {
        // brief: test get local address
        UDPSocket sock;
        sock.bind("127.0.0.1", 7777);
        Address addr = sock.get_local_address();
        EXPECT_EQ("127.0.0.1", addr.ip());
        EXPECT_EQ(7777, addr.port());
        EXPECT_EQ("127.0.0.1:7777", addr.to_string());
    }
    
    {
        // brief: test connect
        UDPSocket sock_bind;
        UDPSocket sock_connect;
        sock_bind.bind("127.0.0.1", 8888);
        sock_connect.connect("127.0.0.1", 8888);

        // brief: test get peer address
        EXPECT_EQ(sock_bind.get_local_address().to_string(), 
                sock_connect.get_peer_address().to_string());
        
        // brief: test send and recv, not specify address in syscall.
        std::string sended_msg = "hi, 8888";
        sock_connect.send(sended_msg);
        auto p = sock_bind.recvfrom(8);
        EXPECT_EQ(p.first, sended_msg);
        
        sended_msg = "hello, i am 8888";
        sock_bind.sendto(sended_msg, p.second);
        EXPECT_EQ(sock_connect.recv(), sended_msg);
    }

    {
        // brief: test sendto and recvfrom
        UDPSocket serv;
        serv.bind("127.0.0.1", 6666);
        
        UDPSocket client;
        // client send to serv
        std::string sended_msg = "hello serv!\n";
        client.sendto(sended_msg, {"127.0.0.1", 6666});

        auto p = serv.recvfrom();
        EXPECT_EQ(p.first, sended_msg);
        EXPECT_EQ(client.get_local_address().port(), 
                p.second.port());

        // serv send to client
        sended_msg = "hi client!\n";
        serv.sendto(sended_msg, p.second);
        p = client.recvfrom();
        EXPECT_EQ(p.first, sended_msg);
    }

    {
        UDPSocket serv;
        Address serv_addr("127.0.0.1", 4444);
        serv.bind(serv_addr);

        UDPSocket client;
        client.connect(serv_addr);
        
        // big msg1
        std::string big_msg(6555, 'c');
        client.send(big_msg);
        EXPECT_EQ(serv.recv(), std::string(6555, 'c'));
        
        // big msg2
        std::string big_msg2(65507, 'c');
        client.send(big_msg2);
        EXPECT_EQ(serv.recv(), std::string(65507, 'c'));

        // big msg3
        // The max size of UDP datagram is 65535 - 28 = 65507
        // std::string big_msg3(65508, 'c');
        // client.send(big_msg3);
        // EXPECT_EQ(serv.recv(), std::string(65508, 'c'));
    }
}

TEST(Socket, TCPSocket) {
    {
        // brief: test commu
        TCPSocket serv;
        serv.bind("127.0.0.1", 7777);
        serv.listen();
        // TCPSocket sock = serv.accept();
        // 
        // std::string msg = sock.recv();
        // while (!msg.empty()) {
        //     // echo
        //     sock.send("echo: " + msg);
        //     msg = sock.recv();
        // }
    }
}

TEST(Socket, UnixSocket) {
    {
        UnixSocket sock;
        sock.bind("/tmp/cwp.socket");
        sock.listen();
    }
}

} // namespace 
