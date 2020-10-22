#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <thread>

using namespace std;

void usage() {
	cout << "syntax: us [-an][-e] <port>\n";
	cout << "  -an: auto newline\n";
	cout << "  -e : echo\n";
	cout << "sample: us 1234\n";
}

struct Param {
	bool autoNewline{false};
	bool echo{false};
	uint16_t port{0};

	bool parse(int argc, char* argv[]) {
		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "-an") == 0) {
				autoNewline = true;
				continue;
			}
			if (strcmp(argv[i], "-e") == 0) {
				echo = true;
				continue;
			}
			port = stoi(argv[i]);
		}
		return port != 0;
	}
} param;

void recvThread(int sd) {
	static const int BUFSIZE = 65536;
	char buf[BUFSIZE];
	while (true) {
		struct sockaddr_in addr;
		socklen_t len = sizeof(addr);
		memset(&addr, 0, sizeof(addr));
		ssize_t res = recvfrom(sd, buf, BUFSIZE - 1, 0, (struct sockaddr*)&addr, &len);
		if (res == 0 || res == -1) {
			cerr << "recvfrom return " << res << endl;
			perror("recv");
			break;
		}
		buf[res] = '\0';
		if (param.autoNewline)
			cout << buf << endl;
		else
			cout << buf;
		if (param.echo) {
			res = sendto(sd, buf, res, 0, (struct sockaddr*)&addr, sizeof(addr));
			if (res == 0 || res == -1) {
				cerr << "sendto return " << res << endl;
				perror("sendto");
				break;
			}
		}
	}
    close(sd);
}

int main(int argc, char* argv[]) {
	if (!param.parse(argc, argv)) {
		usage();
		return -1;
	}

	int sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd == -1) {
		perror("socket");
		return -1;
	}

	int optval = 1;
	int res = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	if (res == -1) {
		perror("setsockopt");
		return -1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(param.port);
	addr.sin_addr.s_addr = INADDR_ANY;
	memset(&addr.sin_zero, 0, sizeof(addr.sin_zero));

	res = ::bind(sd, (struct sockaddr*)&addr, sizeof(addr));
	if (res == -1) {
		perror("bind");
		return -1;
	}

	thread t(recvThread, sd);
	t.join();
	close(sd);
}
