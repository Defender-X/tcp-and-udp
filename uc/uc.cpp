#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <thread>

using namespace std;

void usage() {
	cout << "syntax: uc <ip> <port>\n";
	cout << "sample: uc 127.0.0.1 1234\n";
}

struct Param {
	struct in_addr ip{0};
	uint16_t port{0};

	bool parse(int argc, char* argv[]) {
		if (argc != 3) return false;
		int res = inet_pton(AF_INET, argv[1], &ip);
		switch (res) {
			case 1: break;
			case 0: cerr << "not a valid network address\n"; return false;
			case -1: perror("inet_pton"); return false;
		}
		port = stoi(argv[2]);
		return true;
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
			perror("recvfrom");
			break;
		}
		buf[res] = '\0';
		cout << buf << endl;
	}
    close(sd);
	exit(0);
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

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(param.port);
	addr.sin_addr = param.ip;
	memset(&addr.sin_zero, 0, sizeof(addr.sin_zero));

	thread t(recvThread, sd);
	t.detach();
	while (true) {
		string s;
		getline(cin, s);
		ssize_t res = sendto(sd, s.c_str(), s.size(), 0, (struct sockaddr*)&addr, sizeof(addr));
		if (res == 0 || res == -1) {
			cerr << "sendto return " << res << endl;
			perror("sendto");
			break;
		}
	}
	close(sd);
}
