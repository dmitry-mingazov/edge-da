//  Connects SUB socket to tcp://localhost:5556

#include "zhelpers.h"
#include "jsmn.h"

int main (int argc, char *argv [])
{

    char *string = "{ \"t\": 100, \"time\": \"Thu sadfa\", \
		\"ref\": \"jzp://edv#211b.0000\", \
		\"uuid\": \"afkadjsga\", \
		\"type\": \"Environmental\", \
		\"m\": [ \
				{ \
				\"k\": \"environmental_temperature\", \
				\"t\": 4223, \
				\"tz\": \"dagkd\", \
				\"v\": 42.4, \
				\"u\": \"°C\", \
				\"ref\": \"jpz://edv#211b.0000/environmental_temperature\" \
				}, \
				{ \
				\"k\": \"relative_humidity\", \
				\"t\": 342, \
				\"tz\": \"%s\", \
				\"v\": 3, \
				\"u\": \"%%\", \
				\"ref\": \"jpz://edv#211b.0000/relative_humidity\" \
				}, \
				{ \
				\"k\": \"battery_level\", \
				\"t\": 3234, \
				\"tz\": \"%s\", \
				\"v\": 342, \
				\"u\": \"%%\", \
				\"ref\": \"jpz://edv#211b.0000/battery_level\" \
				}, \
			   ], \
		}";
    
    jsmn_parser parser;
    jsmntok_t tokens[128];
    int n_tokens;
    jsmn_init(&parser);
    n_tokens = jsmn_parse(&parser, string, strlen(string), tokens, 128);
    jsmntok_t curr_tok; 

    int key = 1;
    printf("%d\n", n_tokens);
    // return 0;

    for(int i = 0; i < n_tokens; i++) {
        curr_tok = tokens[i];
        if (curr_tok.type == JSMN_STRING && key == 1) {
            // printf("%d - %d\n", curr_tok.start, curr_tok.end);
            printf("%s\n", strndup(string + curr_tok.start, curr_tok.end - curr_tok.start));
            key = 0;
        } else {
            key = 1;
        }
    }

    return 0;
    //  Socket to talk to server
    void *context = zmq_ctx_new ();
    void *subscriber = zmq_socket (context, ZMQ_SUB);
    int rc = zmq_connect (subscriber, "tcp://localhost:5556");
    assert (rc == 0);

    // int *publisher = zmq_socket (context, ZMQ_PUB);
    // int pc = zmq_bind(publisher, "tcp://*:5557");
    // assert (pc == 0);

    rc = zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE,
                         "", 0);
    assert (rc == 0);

    //  Process 100 updates
    int update_nbr;
    for (update_nbr = 0; update_nbr < 100000; update_nbr++) {
        char *string = s_recv (subscriber);

        // it should crop the data and send it to subscriber
        
        // int temperature, relhumidity;
        // sscanf (string, "%d %d",
            // &temperature, &relhumidity);
        // sscanf (string, "%d %d",
        //     &temperature, &relhumidity);
        // printf("temp: %d - humidity: %d \n", temperature, relhumidity);


        jsmn_parser parser;
        jsmntok_t tokens[128];
        int n_tokens;
        jsmn_init(&parser);
        n_tokens = jsmn_parse(&parser, string, strlen(string), tokens, 128);
        jsmntok_t curr_tok; 

        int key = 1;

        for(int i = 0; i < n_tokens; i++) {
            curr_tok = tokens[i];
            if (curr_tok.type == JSMN_STRING && key == 1) {
                printf("%.*s\n", string+curr_tok.start);
                key = 0;
            } else {
                key = 1;
            }
        }

        // printf("start: %d - end: %d\n",tokens->start, tokens->end);


        printf("Message: %s", string);
        
        // char update[20];
        // sprintf(update, "%d", temperature);
        // s_send(publisher, update);
        // printf("temp: %d - hum: %d\n", temperature, relhumidity);
        // printf("n. %d\n", temperature);
        free (string);
    }

    zmq_close (subscriber);
    zmq_ctx_destroy (context);
    return 0;
}

