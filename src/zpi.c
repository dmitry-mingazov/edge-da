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


char* init_curr_json();
void str_append(char** dest, char* src);
char* get_token_str(char* json, jsmntok_t curr_tok);

int main (int argc, char *argv [])
{
    //  Socket to talk to server
    void *context = zmq_ctx_new ();
    void *subscriber = zmq_socket (context, ZMQ_SUB);
    int rc = zmq_connect (subscriber, "tcp://localhost:5556");
    assert (rc == 0);

    int *publisher = zmq_socket (context, ZMQ_PUB);
    int pc = zmq_bind(publisher, "tcp://*:5557");
    assert (pc == 0);

    rc = zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE,
                         "", 0);
    assert (rc == 0);

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
    char* mid_json = ",\n";
    char* end_json = "\n}";
    int n_valuables = (sizeof(valuables) / sizeof(const char*));
    int MARK_TOT = 0;
    for(int i = 0; i < n_valuables; i++) {
        MARK_TOT |= mark[i];
    }
    for (;;) {
        char* buffer = malloc(1024 * sizeof(char));
        zmq_recv(subscriber, buffer, 800, 0);
        size_t buf_size = strlen(buffer);
        char* in_json = malloc(buf_size * sizeof(char));
        strcpy(in_json, buffer);

        // it should crop the data and send it to subscriber
        jsmn_parser parser;
        jsmntok_t tokens[128];
        int n_tokens;
        jsmn_init(&parser);
        n_tokens = jsmn_parse(&parser, in_json, strlen(in_json), tokens, 128);
        jsmntok_t curr_tok; 

        int save_value = 0;
        int key = 1;
        int m_array = 0;
        char* curr_json = init_curr_json();
        int curr_mark = 0;

        for(int i = 0; i < n_tokens; i++) {
            curr_tok = tokens[i];
            if (curr_tok.type == JSMN_STRING && key == 1) {
                char* curr_key = get_token_str(in_json, curr_tok);
                if (m_array) {
                    // if (curr_mark == 0) {
                    //     str_append(&curr_json, start_json);
                    // } 
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
            
                    str_append(&curr_json, curr_key);
                    str_append(&curr_json, ": ");
                } else if (strcmp(curr_key, "\"m\"") == 0) {
                    m_array = 1;
                }
                // printf("%d - %d\n", curr_tok.start, curr_tok.end);
                // printf("%s\n", strndup(string + curr_tok.start, curr_tok.end - curr_tok.start));
                key = 0;
            } else {
                if (save_value) {
                    save_value = 0;
                    char* curr_value = get_token_str(in_json, curr_tok);
                    str_append(&curr_json, curr_value);
                    if (curr_mark == MARK_TOT) {
                        str_append(&curr_json, end_json);
                        printf("%s\n", curr_json);
                        s_send(publisher, curr_json);
                        free(curr_json);
                        curr_mark = 0;
                        curr_json = init_curr_json();
                    } else {
                        str_append(&curr_json, mid_json);
                    }
                }
                key = 1;
            }
        }
        free (in_json);
    }

    zmq_close (subscriber);
    zmq_ctx_destroy (context);
    return 0;
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

char* init_curr_json() {
    char* start_json = "{\n";
    char* curr_json = malloc((strlen(start_json) + 1) * sizeof(char));
    strcpy(curr_json, start_json);
    return curr_json;
}