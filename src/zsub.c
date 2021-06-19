//  Connects SUB socket to tcp://zpi:5557

#include "zhelpers.h"
#include "sqlite3.h"

int main (int argc, char *argv [])
{   
    sqlite3 *db;
    sqlite3_stmt *res;
    
    int sqlrc = sqlite3_open(":memory:", &db);
    
    if (sqlrc != SQLITE_OK) {
        
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return 1;
    }

    //  Socket to talk to server
    void *context = zmq_ctx_new ();
    void *subscriber = zmq_socket (context, ZMQ_SUB);
    int rc = zmq_connect (subscriber, "tcp://zpi:5557");
    assert (rc == 0);

    //  Subscribe to zipcode, default is NYC, 10001
    rc = zmq_setsockopt (subscriber, ZMQ_SUBSCRIBE,
                         "", 0);
    assert (rc == 0);

    //  Process 1000 updates
    int update_nbr;
    for (update_nbr = 0; update_nbr < 100000; update_nbr++) {
        char *string = s_recv (subscriber);

        // it should save the data received to local SQL lite server

        int temperature;
        sscanf (string, "%d",
            &temperature);
        printf("temp: %d\n", temperature);

        free (string);
    }

    zmq_close (subscriber);
    zmq_ctx_destroy (context);
    return 0;
}
