#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __linux__
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif // __linux__
#include <iostream>
#include <thread>

#ifdef WIN32
void myerror(const char* msg) { fprintf(stderr, "%s %lu\n", msg, GetLastError()); }
#else
void myerror(const char* msg) { fprintf(stderr, "%s %s %d\n", msg, strerror(errno), errno); }
#endif

void usage() {
    printf("syntax: echo-client <ip> <port>\n");
    printf("sample: echo-client 127.0.0.1 1234\n");
}

struct Param {
    char* ip{nullptr};
    char* port{nullptr};

    bool parse(int argc, char* argv[]) {
        if (argc < 3) return false;
        ip = argv[1];
        port = argv[2];
        return (ip != nullptr) && (port != nullptr);
    }
} param;

void recvThread(int sd) {
    printf("Connected to server\n");
    fflush(stdout);
    static const int BUFSIZE = 65536;
    char buf[BUFSIZE];
    while (true) {
        ssize_t res = ::recv(sd, buf, BUFSIZE - 1, 0);
        if (res == 0 || res == -1) {
            fprintf(stderr, "recv return %zd\n", res);
            myerror(" ");
            break;
        }
        buf[res] = '\0';
        printf("Received: %s", buf);
        fflush(stdout);
    }
    printf("Disconnected from server\n");
    fflush(stdout);
    ::close(sd);
    exit(0);
}

int main(int argc, char* argv[]) {
    if (!param.parse(argc, argv)) {
        usage();
        return -1;
    }

#ifdef WIN32
    WSAData wsaData;
    WSAStartup(0x0202, &wsaData);
#endif // WIN32

    struct addrinfo aiInput, *aiOutput, *ai;
    memset(&aiInput, 0, sizeof(aiInput));
    aiInput.ai_family = AF_INET;
    aiInput.ai_socktype = SOCK_STREAM;
    aiInput.ai_flags = 0;
    aiInput.ai_protocol = 0;

    int res = getaddrinfo(param.ip, param.port, &aiInput, &aiOutput);
    if (res != 0) {
#ifdef WIN32
        fprintf(stderr, "getaddrinfo: %S\n", gai_strerror(res));
#else
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
#endif
        return -1;
    }

    int sd;
    for (ai = aiOutput; ai != nullptr; ai = ai->ai_next) {
        sd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (sd != -1) break;
    }
    if (ai == nullptr) {
        fprintf(stderr, "cannot find socket for %s\n", param.ip);
        return -1;
    }

    res = ::connect(sd, ai->ai_addr, ai->ai_addrlen);
    if (res == -1) {
        myerror("connect");
        return -1;
    }

    freeaddrinfo(aiOutput);

    std::thread t(recvThread, sd);
    t.detach();

    while (true) {
        std::string s;
        std::getline(std::cin, s);
        s += "\r\n";
        ssize_t res = ::send(sd, s.data(), s.size(), 0);
        if (res == 0 || res == -1) {
            fprintf(stderr, "send return %zd\n", res);
            myerror(" ");
            break;
        }
    }
    ::close(sd);
    return 0;
}
