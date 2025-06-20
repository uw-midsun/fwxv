#include <stdio.h>
#include <unistd.h> // for sleep()

// This function increases the number by 1
int increment_value(int *counter) {
    *counter = *counter + 1;
    return *counter;
}

int main() {
    int counter = 0;

    while (1) {
        int result = increment_value(&counter);
        printf("Hello World! Counter: %d\n", result);
        sleep(1); // wait for 1 second
    }

    return 0;
}
