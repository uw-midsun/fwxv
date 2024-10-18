#include <stdio.h>

#include "log.h"

int main(void)
{
        int i = 0; 
        
        while(1) {
                LOG_DEBUG("Hello World %d\n", i);
                i++; 
        }

        return 0;
}