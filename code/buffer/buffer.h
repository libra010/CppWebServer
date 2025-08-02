#ifndef BUFFER_H
#define BUFFER_H

#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/uio.h>
#include <vector>
#include <atomic>
#include <assert.h>

class Buffer
{
public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    size_t WritableBytes() const;
    size_t ReadableBytes() const;
    size_t PrependableBytes() const;

    const char *Peek() const;
    void EnsureWriteable(size_t len);
    void HasWritten(size_t len);

    void Retrieve(size_t len);
    void RetrieveUntil(const char *end);

    void RetrieveAll();
    std::string RetrieveAllToStr();

    const char *BeginWriteConst() const;
    char *BeginWrite();

    void Append(const std::string &str);
    void Append(const char *str, size_t len);
    void Append(const void *data, size_t len);
    void Append(const Buffer &buff);

    // 读接口 fd socket中的文件描述符
    ssize_t ReadFd(int fd, int *Errno);
    // 写接口
    ssize_t WriteFd(int fd, int *Errno);

private:
    // buffer开头
    char *BeginPtr_();
    const char *BeginPtr_() const;
    // 扩展空间
    void MakeSpace_(size_t len);

    // 存储实体
    std::vector<char> buffer_;
    // 读位置下标
    std::atomic<std::size_t> readPos_;
    // 写位置下标
    std::atomic<std::size_t> writePos_;
};

#endif //BUFFER_H