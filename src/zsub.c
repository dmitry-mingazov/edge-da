//  Connects SUB socket to tcp://zpi:5557

#include "zhelpers.h"
#include "sqlite3.h"
#include "jsmn.h"
#include <string.h>
#include <stdlib.h>

typedef struct zsub
{
  char* ref;
  char* k;
  double v;
  char* u;
  char* tz;
} payload;

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

static payload* buildpayload(const char *json, jsmntok_t *tok, size_t r){
  int i;
  payload* pl = malloc(sizeof(struct zsub));
  for (i = 1; i < r; i++) {
    if (jsoneq(json, &tok[i], "ref") == 0) {
      /* We may use strndup() to fetch string value */
      pl->ref=strndup(json + tok[i + 1].start,tok[i + 1].end - tok[i + 1].start);
      i++;
    } else if (jsoneq(json, &tok[i], "k") == 0) {
      /* We may additionally check if the value is either "true" or "false" */
      pl->k=strndup(json + tok[i + 1].start,tok[i + 1].end - tok[i + 1].start);
      i++;
    } else if (jsoneq(json, &tok[i], "v") == 0) {
      /* We may want to do strtol() here to get numeric value */
      char *tmp = strndup(json + tok[i + 1].start,tok[i + 1].end - tok[i + 1].start);
      pl->v=strtod(tmp, NULL);
      free(tmp);
      i++;
    } else if (jsoneq(json, &tok[i], "tz") == 0) {
      /* We may want to do strtol() here to get numeric value */
      pl->tz=strndup(json + tok[i + 1].start,tok[i + 1].end - tok[i + 1].start);
      i++;
    } else if (jsoneq(json, &tok[i], "u") == 0) {
      /* We may want to do strtol() here to get numeric value */
      pl->u=strndup(json + tok[i + 1].start,tok[i + 1].end - tok[i + 1].start);
      i++;
    } else {
      printf("Unexpected key: %.*s\n", tok[i].end - tok[i].start,
             json + tok[i].start);
    }
  }
  return pl;
}

static int setuptable(sqlite3 *db){

  char *err_msg = 0;
  int rc;
  char *sql = "DROP TABLE IF EXISTS Data;" 
                "CREATE TABLE Data(ref TEXT, k TEXT, v REAL, u TEXT, tz TEXT);";
  //char *sql = "CREATE TABLE Data(ref TEXT, k TEXT, v REAL, u TEXT, tz TEXT);";
  rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        
        sqlite3_free(err_msg);        
        sqlite3_close(db);
        
        return 1;
    } 
  return 0;
}

static int uploadpayload(sqlite3 *db, payload *pl){
    char *err_msg = 0;
  int rc;
  
  char *sql;
  size_t len;
  char * format = "INSERT INTO Data VALUES('%s','%s',%lf,'%s','%s');";
  len = (size_t)snprintf(NULL, 0, format, pl->ref,pl->k,pl->v,pl->u,pl->tz);
  sql = malloc(len);
  snprintf(sql, len, format, pl->ref,pl->k,pl->v,pl->u,pl->tz);
  rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        
        sqlite3_free(err_msg);        
        sqlite3_close(db);
        
        return 1;
    } 
  free(sql);
  return 0;
}
// static char* gettok(const char *json, jsmntok_t *tok){
//   size_t bufsize = tok->end - tok->start;
//   char* buf = malloc(sizeof(char)*(bufsize + 1));
//   memcpy(buf, json + tok->start, bufsize);
//   buf[bufsize] = '\0';
//   return buf;
// }

static void freepayload(payload *pl){
  free(pl->ref);
  free(pl->tz);
  free(pl->k);
  free(pl->u);
  free(pl);
}

int main (int argc, char *argv [])
{   
    sqlite3 *db;
    
    int sqlrc = sqlite3_open("test.db", &db);
    
    if (sqlrc != SQLITE_OK) {
        
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return 1;
    }
    int setupfail;
    setupfail =setuptable(db);
    if(setupfail){
        fprintf(stderr, "Cannot create table: %s\n", sqlite3_errmsg(db));
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

    //  Process updates
    for(;;) {
        char *string = s_recv (subscriber);
        // it should save the data received to local SQL lite server

// char* string = "{\"ref\":\"device\",\"k\":\""
//    "k1\",\"tz\":101,\"v\":1,\"u\":\""
//    "u1\"}";
        int r;
        jsmn_parser p;
        jsmntok_t *tok;
        size_t tokcount = 0;

        /* Prepare parser */
        jsmn_init(&p);
        r = jsmn_parse(&p, string, strlen(string), NULL, 9999);
        tok = malloc(sizeof(*tok) * r);
        tokcount = r;
        jsmn_init(&p);
        r = jsmn_parse(&p, string, strlen(string), tok, tokcount);

        payload *pl = buildpayload(string,tok,tokcount);
        uploadpayload(db,pl);
        freepayload(pl);
        free(tok);


        free (string);
    }
    sqlite3_close(db);
    zmq_close (subscriber);
    zmq_ctx_destroy (context);
    return 0;
}