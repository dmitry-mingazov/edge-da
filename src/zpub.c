//  Binds PUB socket to tcp://*:5556

#include "zhelpers.h"

int main (void)
{
    //  Prepare our context and publisher
    void *context = zmq_ctx_new ();
    void *publisher = zmq_socket (context, ZMQ_PUB);
    int rc = zmq_bind (publisher, "tcp://*:5556");
    assert (rc == 0);

    //  Initialize random number generator
    srandom ((unsigned) time (NULL));
    // send 10000 random data
    int update_nbr;
    for(update_nbr = 0; update_nbr < 10000; update_nbr++) {
        int temperature, relhumidity;
        
        temperature = randof (215) - 80;
        relhumidity = randof (50) + 10;
	printf("%d %d\n", update_nbr, relhumidity);
        usleep(1);
        //  Send message to all subscribers
        char update [20];
        sprintf (update, "%d %d", update_nbr, relhumidity);
        // sprintf (update, "%d", update_nbr);
        s_send (publisher, update);
    }
    zmq_close (publisher);
    zmq_ctx_destroy (context);
    return 0;
}
