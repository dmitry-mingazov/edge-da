//  Connects SUB socket to tcp://localhost:5556

#include "zhelpers.h"

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

    //  Process 100 updates
    int update_nbr;
    for (update_nbr = 0; update_nbr < 100000; update_nbr++) {
        char *string = s_recv (subscriber);

        // it should crop the data and send it to subscriber
        
        int temperature, relhumidity;
        // sscanf (string, "%d %d",
            // &temperature, &relhumidity);
        sscanf (string, "%d %d",
            &temperature, &relhumidity);
        printf("temp: %d - humidity: %d \n", temperature, relhumidity);
        
        char update[20];
        sprintf(update, "%d", temperature);
        s_send(publisher, update);
        // printf("temp: %d - hum: %d\n", temperature, relhumidity);
        // printf("n. %d\n", temperature);
        free (string);
    }

    zmq_close (subscriber);
    zmq_ctx_destroy (context);
    return 0;
}