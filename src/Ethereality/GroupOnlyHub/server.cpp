#include "tcpserver.h"

int main(int argc, char* argv[]) {
	return startup("0.0.0.0", 5379);
}