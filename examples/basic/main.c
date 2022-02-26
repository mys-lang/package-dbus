#define _GNU_SOURCE

#include "dbg.h"
#include <unistd.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdint.h>
#include <assert.h>

static int setup(const char *bus_path_p)
{
    uint8_t buf[4096];
    struct msghdr msg;
    struct sockaddr_un addr;
    struct iovec io;
    struct cmsghdr *cmsg;
    struct ucred red;
    union {
        char buf[CMSG_SPACE(sizeof(red))];
        struct cmsghdr align;
    } u;

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);

    if (fd < 0) {
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(&addr.sun_path[0], bus_path_p);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        return -2;
    }

    memset(&msg, 0, sizeof(msg));
    io.iov_base = "";
    io.iov_len = 1;
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    red.pid = getpid();
    red.uid = getuid();
    red.gid = getgid();
    msg.msg_control = u.buf;
    msg.msg_controllen = sizeof(u.buf);
    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_CREDENTIALS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(red));
    memcpy(CMSG_DATA(cmsg), &red, sizeof(red));

    if (sendmsg(fd, &msg, MSG_NOSIGNAL) != 1) {
        return -3;
    }

    if (sendto(fd, "AUTH\r\n", 6, MSG_NOSIGNAL, NULL, 0) != 6) {
        return -4;
    }

    if (recvfrom(fd, &buf[0], sizeof(buf), 0, NULL, NULL) != 19) {
        return -5;
    }

    if (sendto(fd, "AUTH EXTERNAL 31303030\r\n", 24, MSG_NOSIGNAL, NULL, 0) != 24) {
        return -6;
    }

    if (recvfrom(fd, &buf[0], sizeof(buf), 0, NULL, NULL) != 37) {
        return -7;
    }

    if (sendto(fd, "NEGOTIATE_UNIX_FD\r\n", 19, MSG_NOSIGNAL, NULL, 0) != 19) {
        return -8;
    }

    if (recvfrom(fd, &buf[0], 4096, 0, NULL, NULL) != 15) {
        return -9;
    }

    if (sendto(fd, "BEGIN\r\n", 7, MSG_NOSIGNAL, NULL, 0) != 7) {
        return -10;
    }

    printf("Setup completed successfully!\n");

    return fd;
}

static int read_fixed_header(int fd,
                             uint8_t *type_p,
                             uint8_t *flags_p,
                             uint8_t *major_p,
                             uint32_t *body_size_p,
                             uint32_t *serial_p,
                             uint32_t *fields_size_p)
{
    uint8_t buf[16];
    int array_size;

    if (read(fd, &buf, 16) != 16) {
        return -1;
    }

    if (buf[0] != 'l') {
        return -2;
    }

    *type_p = buf[1];
    *flags_p = buf[2];
    *major_p = buf[3];
    *body_size_p = ((buf[4] << 0)
                    | (buf[5] << 8)
                    | (buf[6] << 16)
                    | (buf[7] << 24));
    *serial_p = ((buf[8] << 0)
                 | (buf[9] << 8)
                 | (buf[10] << 16)
                 | (buf[11] << 24));

    *fields_size_p = ((buf[12] << 0)
                      | (buf[13] << 8)
                      | (buf[14] << 16)
                      | (buf[15] << 24));

    printf("Header:\n"
           "  type = %u\n"
           "  flags = %u\n"
           "  major = %u\n"
           "  body_length = %u\n"
           "  serial = %u\n"
           "  fields_size = %u\n",
           *type_p,
           *flags_p,
           *major_p,
           *body_size_p,
           *serial_p,
           *fields_size_p);

    return 0;
}

static int read_header_fields(int fd, int size)
{
    uint8_t buf[4096];
    int rest;
    int read_size;

    read_size = size;
    rest = (size % 8);

    if (rest > 0) {
        read_size += (8 - rest);
    }

    if (read(fd, &buf, read_size) != read_size) {
        return -1;
    }

    printf("Header fields:\n");
    dbgh(&buf, size);

    return 0;
}

int main()
{
    uint8_t buf[4096];
    struct msghdr msg;
    struct sockaddr_un addr;
    int fd;
    uint8_t type;
    uint8_t flags;
    uint8_t major;
    uint32_t body_size;
    uint32_t serial;
    uint32_t fields_size;
    int res;

    fd= setup("/var/run/dbus/system_bus_socket");

    if (fd < 0) {
        return -fd;
    }

    if (send(fd,
             "l\1\0\1\0\0\0\0\1\2\3\4n\0\0\0"
             "\1\1o\0\25\0\0\0/org/freedesktop/DBus\0"
             "\0\0\2\1s\0\24\0\0\0org.freedesktop.DBus\0"
             "\0\0\0\6\1s\0\24\0\0\0org.freedesktop.DBus\0"
             "\0\0\0\3\1s\0\5\0\0\0Hello\0\0\0",
             128,
             MSG_NOSIGNAL) != 128) {
        return 15;
    }

    while (1) {
        if (read_fixed_header(fd,
                              &type,
                              &flags,
                              &major,
                              &body_size,
                              &serial,
                              &fields_size) != 0) {
            return 10;
        }

        if (read_header_fields(fd, fields_size) != 0) {
            return 11;
        }

        printf("body:\n");

        if (read(fd, &buf, body_size) != body_size) {
            return 12;
        }

        dbgh(&buf, body_size);
    }

    return 0;
}
