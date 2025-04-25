#include <iostream>
#include <thread>
#include <mutex>

std::mutex mu1;
std::mutex mu2;

void f1(void) {
    mu1.lock();
    mu2.lock();

    std::cout << "hi from f1()!" << std::endl;

    mu2.unlock();
    mu1.unlock();
}

void f2(void) {
    mu2.lock();
    mu1.lock();

    std::cout << "hello from f2()!" << std::endl;

    mu1.unlock();
    mu2.unlock();
}

int main() {
    std::thread thread1(f1);
    std::thread thread2(f2);

    thread1.join();
    thread2.join();
}
