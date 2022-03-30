#ifndef SRPC_NET_FILE_DESCRIPTOR_H
#define SRPC_NET_FILE_DESCRIPTOR_H

#include <memory>
#include <string>

// brief: 
class FDescriptor {
public:
    using ptr = std::shared_ptr<FDescriptor>;

    // brief: only accept fd returned by kernel
    // good case: FDescriptor fd(::open("/tmp/xxx", O_APPEND));
    //            dosomething(fd);
    //
    // bad case: int kfd = ::open("/tmp/xxx", O_APPEND);
    //           dosomething(kfd);
    //           FDescriptor fd(kfd);
    explicit FDescriptor(const int fd);
    ~FDescriptor() = default;

    FDescriptor(const FDescriptor& other) = default;
    FDescriptor& operator=(const FDescriptor& other) = default;
    FDescriptor(FDescriptor&& other) = default;
    FDescriptor& operator=(FDescriptor&& other) = default;

    // brief: increase FDWrapper ref count
    FDescriptor duplicate() const;

    // brief: read limited bytes
    // return: string
    std::string read(size_t limited = std::numeric_limits<size_t>::max());

    // brief: write string type to fd
    // param msg: 
    ssize_t write(const std::string& msg);

    // brief: write c str type to fd
    // param msg: must terminate with '\0'
    ssize_t write(const char* msg);

    // brief: use counts of fd returned by kernel
    size_t use_count() const { return _internal_fd.use_count(); }
private:
    // brief: handler of fd returned by kernel.
    struct FDWrapper {
        using ptr = std::shared_ptr<FDWrapper>;
        FDWrapper(const int fd);

        // brief: call [close(2)] on FDWrapper::fd by deconstructor
        ~FDWrapper();

        // brief: call [close(2)] on FDWrapper::fd
        void close();

        int fd{};
        bool eof{};
        bool closed{};

        // brief: 因为 FDWrapper 封装了 fd，而 fd 本身是一种 os 资源，且对应着打开文件资源，
        // 因此需要删除 copy functions
        FDWrapper(const FDWrapper&) = delete;
        FDWrapper& operator=(const FDWrapper&) = delete;
        // brief: 不需要被移动，如果被移动了，指向 FDWrapper 的 ptr 将会失效
        FDWrapper(FDWrapper&&) = delete;
        FDWrapper& operator=(FDWrapper&&) = delete;
    };

    // brief: way to increase the ref count of FDWrapper, called by FDescriptor::duplicate
    explicit FDescriptor(FDWrapper::ptr other);
    FDWrapper::ptr _internal_fd;
};

#endif
