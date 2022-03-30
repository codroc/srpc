#include "file_descriptor.h"
#include "flog.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>


FDescriptor::FDWrapper::FDWrapper(const int fd)
    : fd(fd)
{
    if (fd < 0) {
        LOG_ERROR << "fd < 0\n";
    }
}

FDescriptor::FDWrapper::~FDWrapper() {
    if (closed) return;
    close();
}

void FDescriptor::FDWrapper::close() {
    if (closed) return;
    ::close(fd);
    eof = true;
    closed = true;
}

FDescriptor::FDescriptor(FDWrapper::ptr other)
    : _internal_fd(other)
{}

FDescriptor::FDescriptor(const int fd) 
    : _internal_fd(std::make_shared<FDWrapper>(fd))
{}

FDescriptor FDescriptor::duplicate() const {
    return FDescriptor(_internal_fd);
}

std::string FDescriptor::read(size_t limited) {
    // ssize_t read(int fd, void *buf, size_t count);
    static size_t kBufferSize = 1024*1024;
    size_t cap = std::min(limited, kBufferSize);
    char buf[cap]{};
    ssize_t readed = ::read(_internal_fd->fd, buf, cap);
    if (readed < 0) {
        LOG_ERROR << "FDescriptor::read ::read return < 0\n";
        return {};
    } else if (limited > 0 && readed == 0) _internal_fd->eof = true;
    return buf;
}

ssize_t FDescriptor::write(const std::string& msg) {
    ssize_t ret = ::write(_internal_fd->fd, msg.data(), msg.size());
    if (ret < 0)
        LOG_ERROR << "write return -1, errstr: " << ::strerror(errno)
            << ". file descriptor: " << _internal_fd->fd << "\n";
    return ret;
}

ssize_t FDescriptor::write(const char* msg) {
    return write(msg);
}
