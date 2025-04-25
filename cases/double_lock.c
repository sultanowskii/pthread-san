#include <pthread.h>

int main() {
    pthread_mutex_t mu;

    pthread_mutex_init(&mu, NULL);

    pthread_mutex_lock(&mu);
    pthread_mutex_lock(&mu);

    pthread_mutex_destroy(&mu);
    return 0;
}
