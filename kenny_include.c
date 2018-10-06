#include "kenny_include.h"

long double get_second_time(){
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec;
}

long double get_microsecond_time(){
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_usec;
}
