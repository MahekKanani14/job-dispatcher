#ifndef IPC_H
#define IPC_H

#include <unistd.h>
#include <cstddef>

// Shared low-level pipe I/O helpers used by both master and worker.
//
// A plain read()/write() on a pipe is allowed to return fewer bytes than
// requested (short read/write), so every message that crosses a pipe in
// this project must go through these "exact" wrappers rather than a bare
// read()/write() call.

// Reads exactly n bytes into buf. Returns false on EOF/error (the other
// end of the pipe closed, or something went wrong) before n bytes arrived.
inline bool read_exact(int fd, void* buf, size_t n) {
    char* p = static_cast<char*>(buf);
    size_t total = 0;
    while (total < n) {
        ssize_t r = read(fd, p + total, n - total);
        if (r <= 0) return false;
        total += static_cast<size_t>(r);
    }
    return true;
}

// Writes exactly n bytes from buf. Returns false if the write fails or the
// reading end has gone away.
inline bool write_exact(int fd, const void* buf, size_t n) {
    const char* p = static_cast<const char*>(buf);
    size_t total = 0;
    while (total < n) {
        ssize_t w = write(fd, p + total, n - total);
        if (w <= 0) return false;
        total += static_cast<size_t>(w);
    }
    return true;
}

// Convenience wrappers for sending/receiving a single POD struct (Job,
// JobResult, ...) as raw bytes over a pipe.
template <typename T>
inline bool send_message(int fd, const T& msg) {
    return write_exact(fd, &msg, sizeof(T));
}

template <typename T>
inline bool recv_message(int fd, T& msg) {
    return read_exact(fd, &msg, sizeof(T));
}

#endif // IPC_H
