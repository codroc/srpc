#include "rpc/stub_generator.h"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <bits/stdc++.h>
using namespace std;
using namespace rapidjson;

void test_message_generator(const std::string& filename, Document& d) {
    srpc::rpc::StubGenerator generator(filename, d);
}

void test_macro(const std::string& filename, Document& d) {
    srpc::rpc::StubGenerator generator(filename, d);
    cout << generator.generateMacro();
}

void test_service_define(const std::string& filename, Document& d) {
    srpc::rpc::StubGenerator generator(filename, d);
    cout << generator.generateServiceDefine();
}

void test_client_stub(const std::string& filename, Document& d) {
    srpc::rpc::StubGenerator generator(filename, d);
    cout << generator.generateClientStubDefine();
}

void test_server_stub(const std::string& filename, Document& d) {
    srpc::rpc::StubGenerator generator(filename, d);
    cout << generator.generateServerStubDefine();
}

void test_generateMessageClass(const std::string& filename, Document& d) {
    srpc::rpc::StubGenerator generator(filename, d);
    cout << generator.generateMessageClass();
}

void test_generateServiceClass(const std::string& filename, Document& d) {
    srpc::rpc::StubGenerator generator(filename, d);
    cout << generator.generateServiceClass();
}

void test_generate_header_file(const std::string& filename, Document& d) {
    srpc::rpc::StubGenerator generator(filename, d);
    cout << generator.generateHeaderFile(filename);
}

void test_generate_source_file(const std::string& filename, Document& d) {
    srpc::rpc::StubGenerator generator(filename, d);
    cout << generator.generateSourceFile(filename);
}

class WriteableFile {
public:
    explicit WriteableFile(const std::string& filename)
        : filename_(filename)
    {
        fd_ = ::open(filename.c_str(), O_WRONLY|O_CREAT, 0666);
    }

    void Write(const std::string& content) {
        ::write(fd_, content.c_str(), content.size());
    }
private:
    std::string filename_;
    int fd_;
};

void final_test(const std::string& filename, Document& d) {
    srpc::rpc::StubGenerator generator(filename, d);
    std::string source_file_name = filename + ".srpc.cpp";
    std::string header_file_name = filename + ".srpc.h";

    WriteableFile source(source_file_name);
    source.Write(generator.generateSourceFile(filename));

    WriteableFile header(header_file_name);
    header.Write(generator.generateHeaderFile(filename));
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("usage: %s [idl.json] [output file name]\n", argv[0]);
        exit(1);
    }
    string filename(argv[1]);
    string output_filename(argv[2]);

    FILE* fp = fopen(filename.c_str(), "rb");
    char readBuffer[65536];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));

    Document d;
    d.ParseStream(is);

    // test_message_generator(output_filename, d);
    // test_macro(output_filename, d);
    // test_service_define(output_filename, d);

    // test_client_stub(output_filename, d);
    // test_server_stub(output_filename, d);

    // test_generateMessageClass(output_filename, d);
    // test_generateServiceClass(output_filename, d);
    //

    // test_generate_header_file(output_filename, d);
    // test_generate_source_file(output_filename, d);

    final_test(output_filename, d);
    return 0;
}
