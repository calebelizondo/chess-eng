#include "http_service.h"
#include <pthread.h>


pthread_t http_thread;

int main() {

    pthread_create(&http_thread, NULL, http_service, NULL); 
    pthread_join(http_thread, NULL);

    return 0;
}