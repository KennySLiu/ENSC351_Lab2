#include "kenny_include.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include </usr/include/pthread.h>
#define NUM_THREADS 1000


int attendance = 0;     // Number of people in the concert
int release = 0;        // bool to signify that all threads have been created
int m3_now_serving = 1; // For method 3, this is the now_serving part of the ticket mutex
int m3_dispenser = 0;   // For method 3, this is the dispenser part of the ticket mutex
double main_start_time = 0;
double main_end_time = 0;
double t_start_times[NUM_THREADS] = {0};    // To record the start times of thread waiting
double t_end_times[NUM_THREADS] = {0};      // To record the end times of thread waiting
double t_wait_times[NUM_THREADS] = {0};     // To record the amount that each thread waited
double t_outliers[NUM_THREADS] = {0};       // Outliers of wait time for threads
double t_avg_time = 0;                      // Average wait time for threads
int m2_array[NUM_THREADS] = {0};            // The boolean array for part 2
pthread_mutex_t m1_lock;        // the lock, for part 1
pthread_mutex_t m3_lock;        // the lock, for part 3


void perform_stat_analysis();       // Sorts the thread times and gets information about them
int comparison(const void* a, const void* b);   //Comparison, for the qsort method



double get_time(){
    double retval = get_second_time();
    retval = retval * 1000000;
    retval += get_microsecond_time();
    return retval;
}


void* method_1_concert(void* identifier){
    int* id = (int*) identifier;
    while (!release){} // Wait for release to be 1
    
    double start_time = get_time();
    pthread_mutex_lock(&m1_lock);
    t_start_times[*id] = start_time;
    ++attendance;
    t_end_times[*id] = get_time();
    pthread_mutex_unlock(&m1_lock);

    return 0;
}

void* method_2_concert(void* identifier){
    int* id = (int*) identifier;
    while (!release){} // Wait for release to be 1

    double start_time = get_time();
    t_start_times[*id] = start_time;
    while (m2_array[*id] == 0){
        //do nothing
        sleep(0.001);
    }
    ++attendance;
    m2_array[*id + 1] = 1;
    t_end_times[*id] = get_time();

    return 0;
}

int test_and_inc(){
    pthread_mutex_lock(&m3_lock);
    int myIndex = m3_dispenser + 1;
    ++m3_dispenser;
    pthread_mutex_unlock(&m3_lock);
    return myIndex;
}

void* method_3_concert(void* identifier){
    int* id = (int*) identifier;
    while (!release){} // Wait for release to be 1

    t_start_times[*id] = get_time();
    int myIndex = test_and_inc();
    //printf("%d, %d\n", *id, myIndex);
    while (myIndex != m3_now_serving){
        sleep(0.01);
    }
    ++attendance;
    t_end_times[*id] = get_time();
    ++m3_now_serving;

    return 0;
}





int main(int argc, char* argv[]){

    if (argc != 2){
        printf("Usage: ./a.out $(concert_admittance_method)\n");
        printf("For example, if you wish to use method 1, ./a.out 1");
        return 0;
    }
    pthread_t threads[NUM_THREADS];
    int identifiers[NUM_THREADS];
    int method_num = atoi(argv[1]);
    if (method_num < 1 || method_num > 3){
        printf("Usage: ./a.out $(concert_admittance_method)\n");
        printf("For example, if you wish to use method 1, ./a.out 1\n");
        printf("There are only 3 methods, as outlined in the document.");
        return 0;
    }


    for (int i = 0; i<NUM_THREADS; ++i){
        identifiers[i] = i;
        
        if (method_num == 1){
            pthread_create(&threads[i], NULL, &method_1_concert, &identifiers[i]);
        } else if (method_num == 2){
            pthread_create(&threads[i], NULL, &method_2_concert, &identifiers[i]);
        } else if (method_num == 3){
            pthread_create(&threads[i], NULL, &method_3_concert, &identifiers[i]);
        }
    }


    main_start_time = get_time();

    release = 1;
    if (method_num == 2){
        m2_array[0] = 1;
    }
    
    while (attendance < NUM_THREADS){} // wait for attendance to get to 1000

    for (int i = 0; i < NUM_THREADS; ++i){
        pthread_join(threads[i], NULL);
    }
    
    main_end_time = get_time();


    
    perform_stat_analysis();


}





void perform_stat_analysis(){
    /* Get statistics on the thread times: */
    double t_max_wait = 0;
    double t_min_wait = 99999;
    double cur_time = 0;

    double interquartile_range = 0;
    double third_q = 0;
    double first_q = 0;

    /* Calculate the wait times: */ 
    for (int i = 0; i < NUM_THREADS; ++i){
        cur_time = t_end_times[i] - t_start_times[i];
        t_wait_times[i] = cur_time;
        t_avg_time += t_end_times[i] - t_start_times[i];
        if (cur_time > t_max_wait){ 
            t_max_wait = cur_time;
        }
        if (cur_time < t_min_wait){
            t_min_wait = cur_time;
        }
    }

    /* Sort the array of thread wait times: */
    qsort(t_wait_times, NUM_THREADS, sizeof(double), comparison);
    /*for (int i = 0; i < NUM_THREADS; ++i){
        printf("%lf, ", t_wait_times[i]);
    }*/

    //printf("DEBUG: iqr indices: %d, %d\n", 3*(NUM_THREADS+1)/4, (NUM_THREADS+1)/4);
    third_q = t_wait_times[3*(NUM_THREADS+1)/4];
    first_q = t_wait_times[(NUM_THREADS+1)/4];
    interquartile_range = third_q - first_q;
    //printf("DEBUG: IQR: %lf\n", interquartile_range);
    //printf("DEBUG: Upper fence & lower fence: %lf , %lf\n", third_q + 1.5*interquartile_range, first_q - 1.5*interquartile_range);
    
    /* Find outliers: */
    for (int i = 0; i < NUM_THREADS; ++i){
        cur_time = t_wait_times[i];
        if (cur_time > third_q + 1.5*interquartile_range){
            t_outliers[i] = cur_time;
        } 
        if (cur_time < first_q - 1.5*interquartile_range){
            t_outliers[i] = cur_time;
        }
    }
    
    t_avg_time /= (double) NUM_THREADS;



    printf("\n\n\n\nFORMAT: avg thread wait , total runtime , final attendance\nthread wait time outliers\n");
    printf("%lf , %lf , %d\n", t_avg_time, main_end_time - main_start_time, attendance);
    printf("Outliers: ");
    for (int i = 0; i < NUM_THREADS; ++i){
        if (t_outliers[i] > 0){
            printf("%lf, ", t_outliers[i]);
        }
    }
    fflush(stdout);

}

int comparison(const void* a, const void* b){
    double valA = *((double*) a);
    double valB = *((double*) b);

    if (valA > valB) return 1;
    if (valA < valB) return -1;
    return 0;
}

    








