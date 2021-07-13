//  Connects SUB socket to tcp://localhost:5556

#include "zhelpers.h"
#include "jsmn.h"
#include <string.h>

typedef struct s_payload {
    char *ref;
    char* k;
    double v;
    char* u;
    char* tz;
} payload;

void str_append(char** dest, char* src) {
    char* tmp;
    int size = (strlen(*dest) + strlen(src) + 1) * sizeof(char);
    // printf("%d - %d - %d\n", size, strlen(*dest), strlen(src));

    if (!(tmp = realloc(*dest, size))) {
        fprintf(stderr, "Out of memory!");
        exit(1);
    }
    *dest = tmp;
    // printf("%s\n", *dest);
    strcat(*dest, src);
}

char* get_token_str(char* json, jsmntok_t curr_tok) {
    char* res;
    int curr_tok_len = curr_tok.end - curr_tok.start;
    // char* curr_tok_str = malloc(curr_tok_len * sizeof(char));
    switch(curr_tok.type) {
        case JSMN_STRING:
            res = malloc((curr_tok_len + 2) * sizeof(char));
            res = strndup(json + curr_tok.start - 1, curr_tok_len+2);
            break;
        default:
            res = malloc(curr_tok_len * sizeof(char));
            res = strndup(json + curr_tok.start, curr_tok_len);
    }

    return res;
}

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
    int m_array = 0;
    char* valuables[] = {
        "\"ref\"",
        "\"k\"",
        "\"v\"",
        "\"tz\"",
        "\"u\"",
    };

    int mark[] = {
        01,
        02,
        04,
        010,
        020
    };
    int n_valuables = (sizeof(valuables) / sizeof(const char*));
    int MARK_TOT = 0;
    int curr_mark;
    for(int i = 0; i < n_valuables; i++) {
        MARK_TOT |= mark[i];
    }
    int save_value = 0;
    char* curr_key;
    int curr_key_len;
    char* start_json = "{\n";
    char* mid_json = ",\n";
    char* end_json = "\n}";

    // printf("%d\n", n_tokens);
    printf("%d\n", MARK_TOT);
    // return 0;

    char* curr_json = malloc(1);
    // *curr_json = "";
    // char* curr_json = "{\n";

    for(int i = 0; i < n_tokens; i++) {
        curr_tok = tokens[i];
        if (curr_tok.type == JSMN_STRING && key == 1) {
            // curr_key_len = curr_tok.end - curr_tok.start;
            // curr_key = malloc(curr_key_len * sizeof(char));
            // curr_key = strndup(string + curr_tok.start, curr_key_len);
            curr_key = get_token_str(string, curr_tok);
            if (m_array) {
                if (curr_mark == 0) {
                    str_append(&curr_json, start_json);
                } 
                for (int j = 0; j < n_valuables; j++) {
                    if (strcmp(curr_key, valuables[j]) == 0) {
                        save_value = 1;
                        curr_mark |= mark[j];
                        break;
                    }
                }
                if (!save_value) {
                    continue;
                }
                // int size = (strlen(curr_json)+ curr_key_len + 1) * sizeof(char);
                // char* tmp;
                // if (!(tmp = realloc(curr_json, size))) {
                //     fprintf(stderr, "Out of memory!");
                //     exit(1);
                // }
                // curr_json = tmp;
                // strcat(curr_json, curr_key);
                str_append(&curr_json, curr_key);
                str_append(&curr_json, ": ");
                // printf("%s\n",curr_key);
            } else if (strcmp(curr_key, "\"m\"") == 0) {
                m_array = 1;
                curr_mark = 0;
            }
            // printf("%d - %d\n", curr_tok.start, curr_tok.end);
            // printf("%s\n", strndup(string + curr_tok.start, curr_tok.end - curr_tok.start));
            key = 0;
        } else {
            if (save_value) {
                save_value = 0;
                char* curr_value = get_token_str(string, curr_tok);
                str_append(&curr_json, curr_value);
                if (curr_mark == MARK_TOT) {
                    str_append(&curr_json, end_json);
                    printf("%s\n", curr_json);
                    curr_mark = 0;
                    curr_json = malloc(1);
                } else {
                    str_append(&curr_json, mid_json);
                }
            }
            key = 1;
        }
    }

    // printf("%s\n", curr_json);

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

