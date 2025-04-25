#include <pthread.h>
#include <stdio.h>

pthread_mutex_t mu1;
pthread_mutex_t mu2;
pthread_mutex_t mu3;
pthread_mutex_t mu4;

void *f1(void *arg) {
    (void)arg;

    pthread_mutex_lock(&mu1);
    pthread_mutex_lock(&mu2);
    pthread_mutex_lock(&mu3);
    pthread_mutex_lock(&mu4);

    puts("f1 says: hi");

    pthread_mutex_unlock(&mu4);
    pthread_mutex_unlock(&mu3);
    pthread_mutex_unlock(&mu2);
    pthread_mutex_unlock(&mu1);
}

void *f2(void *arg) {
    (void)arg;

    pthread_mutex_lock(&mu2);
    pthread_mutex_lock(&mu3);
    pthread_mutex_lock(&mu4);
    pthread_mutex_lock(&mu1);

    puts("f2 says: hi");

    pthread_mutex_unlock(&mu1);
    pthread_mutex_unlock(&mu4);
    pthread_mutex_unlock(&mu3);
    pthread_mutex_unlock(&mu2);
}

int main() {
    pthread_mutex_init(&mu1, NULL);
    pthread_mutex_init(&mu2, NULL);
    pthread_mutex_init(&mu3, NULL);
    pthread_mutex_init(&mu4, NULL);

    pthread_t thread1;
    pthread_t thread2;

    pthread_create(&thread1, NULL, f1, NULL);
    pthread_create(&thread2, NULL, f2, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    pthread_mutex_destroy(&mu1);
    pthread_mutex_destroy(&mu2);
    pthread_mutex_destroy(&mu3);
    pthread_mutex_destroy(&mu4);
}
