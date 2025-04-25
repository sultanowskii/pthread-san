#include <pthread.h>

pthread_mutex_t mu;

void *f(void *arg) {
    (void)arg;

    pthread_mutex_lock(&mu);
    pthread_mutex_unlock(&mu);
    return NULL;
}

int main() {
    pthread_mutex_init(&mu, NULL);

    pthread_t thread1;
    pthread_t thread2;
    pthread_create(&thread1, NULL, f, NULL);
    pthread_create(&thread2, NULL, f, NULL);

    pthread_mutex_lock(&mu);
    pthread_mutex_unlock(&mu);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&mu);
    return 0;
}
