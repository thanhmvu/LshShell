#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#define DEFAULT_TIME 5

int main(int argc, char* argv[]) {
	// get the number of seconds to sleep from command-line argument
	int time_sleep = (argc >= 2 ? atoi(argv[1]) : DEFAULT_TIME);

	sleep(time_sleep);
	printf("ending now \n");
	return 0;
}
