#include <libconfig.h>
#include "../include/ServiceDiscovery.hh"
#include <rude/config.h>
using namespace std;

int main() {
	ServiceDiscovery *ptr=ServiceDiscovery::getInstance();
	ptr->discoveryStart();
	//--------------------------<>

	return 0;
}
