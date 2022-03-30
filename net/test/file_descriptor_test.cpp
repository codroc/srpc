#include <gtest/gtest.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "file_descriptor.h"

#include <string>

namespace {

TEST(FDescriptor, ShareObj) {
    {
        // The argument flags must include one of the following access modes: O_RDONLY, O_WRONLY, or O_RDWR.
        // brief: just one FDescriptor
        // Note: O_TRUNC 标志表示，如果文件存在，则把里面的内容全部丢弃
        FDescriptor fd(::open("/tmp/fd_test.txt", O_CREAT | O_TRUNC | O_RDWR, 0666));
        std::string str{"hello world\n"};
        EXPECT_EQ(fd.write(str), str.size()); 
        EXPECT_EQ(fd.use_count(), 1);
    }

    {
        // brief: two FDescriptors share one FDWrapper
        // fd1 use to read from file "/tmp/fd_test.txt"
        // fd2 use to write to  file "/tmp/fd_test.txt"
        //
        // Note: 对于普通文件来说，读写是共享同一个 “文件偏移” 的，如果想既读又写，可以 open 两次
        // 当然也可以用 [lseek(2)] 时刻改变 文件偏移
        // 奇怪的是 socket 能同时 read write 同一个 fd 而不出错，为什么？
        FDescriptor fd1(::open("/tmp/fd_test.txt", O_RDONLY));
        FDescriptor fd2(::open("/tmp/fd_test.txt", O_WRONLY | O_APPEND));
        FDescriptor fd2dup = fd2.duplicate();
        EXPECT_EQ(fd2.use_count(), 2);

        std::string str{"hello world\n"};

        EXPECT_EQ(fd2.write(str), str.size());
        EXPECT_EQ(fd1.read(), str + str); 
    }

    {
        FDescriptor fd1(::open("/tmp/fd_test.txt", O_RDONLY));
        // copy constructor ref + 1
        FDescriptor fd2 = fd1;
        EXPECT_EQ(fd2.use_count(), 2);

        std::string str{"hello world\n"};
        EXPECT_EQ(fd2.read(), str + str); 

        // copy assignment ref + 1
        FDescriptor fd3 = fd2.duplicate();
        fd3 = fd2;

        // fd1, fd2, fd3
        EXPECT_EQ(fd1.use_count(), 3);

        // move construct
        FDescriptor fd4(std::move(fd1)); // +1 -1
        EXPECT_EQ(fd4.use_count(), 3);

        // move assignment
        FDescriptor fd5(fd4); // +1 -1
        fd5 = std::move(fd2);
        EXPECT_EQ(fd4.use_count(), 3);
    }
}

} // namespace 
