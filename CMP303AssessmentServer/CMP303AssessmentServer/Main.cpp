#include "Network.h"

int main() {
	Network network;
	network.init();
	//network.initUDP();
	while (1) {
		network.run();
	}
}