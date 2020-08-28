#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <thread>

using namespace std;

void usage() {
	cout << "syntax: us [-e] <port>\n";
	cout << "sample: us 1234\n";
}

struct Param {
	bool echo{false};
	uint16_t port{0};

	bool parse(int argc, char* argv[]) {
		if (argc < 2) return false;
		bool portExist = false;
		for (int i = 1; i < argc; i++) {
			if (strcmp(argv[i], "-e") == 0) {
				echo = true;
				continue;
			}
			port = stoi(argv[i]);
			portExist = true;
		}
		return portExist;
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
			fprintf(stderr, "recvfrom return %ld\n", res);
			perror("recv");
			break;
		}
		buf[res] = '\0';
		cout << buf << endl;
		if (param.echo) {
			res = sendto(sd, buf, res, 0, (struct sockaddr*)&addr, sizeof(addr));
			if (res == 0 || res == -1) {
				fprintf(stderr, "sendto return %ld\n", res);
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

	res = bind(sd, (struct sockaddr*)&addr, sizeof(addr));
	if (res == -1) {
		perror("bind");
		return -1;
	}

	thread t(recvThread, sd);
	t.join();
	close(sd);
}
