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

int main()
{
    uint8_t buf[4096];
    struct msghdr msg;
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(&addr.sun_path[0], "/var/run/dbus/system_bus_socket");
    assert(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == 0);

    memset(&msg, 0, sizeof(msg));
    struct iovec io;
    io.iov_base = "";
    io.iov_len = 1;
    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    struct cmsghdr *cmsg;
    struct ucred red;
    red.pid = getpid();
    red.uid = getuid();
    red.gid = getgid();
    union {
        char buf[CMSG_SPACE(sizeof(red))];
        struct cmsghdr align;
    } u;
    msg.msg_control = u.buf;
    msg.msg_controllen = sizeof(u.buf);
    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_CREDENTIALS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(red));
    memcpy(CMSG_DATA(cmsg), &red, sizeof(red));
    assert(sendmsg(fd, &msg, MSG_NOSIGNAL) == 1);

    assert(sendto(fd, "AUTH\r\n", 6, MSG_NOSIGNAL, NULL, 0) == 6);
    assert(recvfrom(fd, &buf[0], sizeof(buf), 0, NULL, NULL) == 19);
    assert(sendto(fd, "AUTH EXTERNAL 31303030\r\n", 24, MSG_NOSIGNAL, NULL, 0) == 24);
    assert(recvfrom(fd, &buf[0], sizeof(buf), 0, NULL, NULL) == 37);
    assert(sendto(fd, "NEGOTIATE_UNIX_FD\r\n", 19, MSG_NOSIGNAL, NULL, 0) == 19);
    assert(recvfrom(fd, &buf[0], 4096, 0, NULL, NULL) == 15);
    assert(sendto(fd, "BEGIN\r\n", 7, MSG_NOSIGNAL, NULL, 0) == 7);
    assert(send(fd, "l\1\0\1\0\0\0\0\1\2\3\4n\0\0\0\1\1o\0\25\0\0\0/org/freedesktop/DBus\0\0\0\2\1s\0\24\0\0\0org.freedesktop.DBus\0\0\0\0\6\1s\0\24\0\0\0org.freedesktop.DBus\0\0\0\0\3\1s\0\5\0\0\0Hello\0\0\0", 128, MSG_NOSIGNAL) == 128);
    assert(read(fd, &buf, 16) == 16);
    fprintf(stderr, "Fixed header:\n");
    dbgh(&buf, 16);
    fprintf(stderr, "Fields size: %d\n", buf[12]);
    fprintf(stderr, "Body size: %d\n", buf[4]);
    int res = recv(fd, &buf, 64, MSG_NOSIGNAL);
    fprintf(stderr, "Header fields:\n");
    dbgh(&buf, 61);
    fprintf(stderr, "body:\n");
    res = recv(fd, &buf, 11, MSG_NOSIGNAL);
    dbgh(&buf, res);
    res = recv(fd, &buf, sizeof(buf), MSG_NOSIGNAL);
    dbgh(&buf, res);
    
    return 0;
}
