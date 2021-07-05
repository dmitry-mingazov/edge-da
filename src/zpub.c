//  Binds PUB socket to tcp://*:5556

#include "zhelpers.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <getopt.h>
#include <uuid/uuid.h>

// Generate random int between lower and upper bounds
int getRandomInt(int lower, int upper) {
    int num = (rand() %
        (upper - lower + 1)) + lower;
  return num;
}

// Generate random float between lower and upper bounds
float getRandomFloat(float lower, float upper) {
    float random = ((float) rand()) / (float) RAND_MAX;
    float diff = upper - lower;
    float r = random * diff;
    return lower + r;
}

int main(int argc, char **argv)
{
    //  Prepare context and publisher
    void *context = zmq_ctx_new ();
    void *publisher = zmq_socket(context, ZMQ_PUB);
    int rc = zmq_bind (publisher, "tcp://*:5556");
    assert (rc == 0);
	
	// Prepare to create and send messages
	srand(time(0));
	int is_quantity_opt_available = 0;
	int is_frequency_opt_available = 0;
	int option; 
	int quantity = 100;
	int frequency = 100 * 1000; 

	// Check optional commands
	while((option = getopt(argc, argv, "q:f:")) != -1) {
		switch(option) {
			case 'q':
				quantity = atoi(optarg); 
				printf("Quantity inserted: %d messages.\n", quantity);  
				is_quantity_opt_available = 1;
				break;
			case 'f':
				frequency = atoi(optarg) * 1000; 
				printf("Frequency entered: %dms.\n", frequency/1000);
				is_frequency_opt_available = 1;
				break;
			default: 
				printf("Error: wrong command entered.\n");
		}
	}
  
	if(is_quantity_opt_available == 0 && is_frequency_opt_available == 0) {
		printf("Default values will be used: 100 total messages at a frequency of 100ms per message.\n");
	} else if(is_quantity_opt_available == 0) {
		printf("Default value for quantity will be used: 100 total messages.\n");
	} else if(is_frequency_opt_available == 0) {
		printf("Default value for frequency will be used: 100ms per message.\n");
	}
	printf("\n");
  
	char msg[800];
	int count = 0;

	// Create and send json strings
	for (quantity; quantity > 0; quantity--) {
	
		// Generate time stamp
		time_t rawtime;
		struct tm * timeinfo;
		time(&rawtime);
		timeinfo = localtime(&rawtime);
		char* time = asctime(timeinfo); 
		int length = strlen(time);
		time[length - 1] = '\0';

		// Generate UUID code
		uuid_t binuuid;
		uuid_generate_random(binuuid);
		char *uuid = malloc(37);
		uuid_unparse_lower(binuuid, uuid);

		// Generate unique code t
		int t = getRandomInt(100000000, 999999999);

		sprintf(msg, "{ \"t\": %d, \"time\": \"%s\", \
		\"ref\": \"jzp://edv#211b.0000\", \
		\"uuid\": \"%s\", \
		\"type\": \"Environmental\", \
		\"m\": [ \
				{ \
				\"k\": \"environmental_temperature\", \
				\"t\": %d, \
				\"tz\": \"%s\", \
				\"v\": %f, \
				\"u\": \"°C\", \
				\"ref\": \"jpz://edv#211b.0000/environmental_temperature\" \
				}, \
				{ \
				\"k\": \"relative_humidity\", \
				\"t\": %d, \
				\"tz\": \"%s\", \
				\"v\": %d, \
				\"u\": \"%%\", \
				\"ref\": \"jpz://edv#211b.0000/relative_humidity\" \
				}, \
				{ \
				\"k\": \"battery_level\", \
				\"t\": %d, \
				\"tz\": \"%s\", \
				\"v\": %d, \
				\"u\": \"%%\", \
				\"ref\": \"jpz://edv#211b.0000/battery_level\" \
				}, \
			   ], \
		}", t, time, uuid, 
		t, time, getRandomFloat(-5.0, 35.0), 
		t, time, getRandomInt(5, 95),
		t, time, getRandomInt(0, 100));
		
		count++;

		printf("IoT device message #%d:\n---\n%s\n---\n\n", count, msg);
		// Send message
		s_send (publisher, msg);
		// Wait before sending another message
		usleep(frequency);
	}

    zmq_close (publisher);
    zmq_ctx_destroy (context);
    return 0;
}
