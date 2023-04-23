#include "Enclave.h"
#include "Enclave_t.h" /* print_string */
#include <stdarg.h>
#include <stdio.h> /* vsnprintf */
#include <string.h>

int rand_state = 42;

int
glibc_gcc_rand()
{
    rand_state = 1103515245 * rand_state + 12345;
    return (rand_state & 0x7fffffff);
}


void
init_array (int array[], int array_len)
{
    int i;
    for (i = 0; i < array_len; i++) {
        array[i] = glibc_gcc_rand();
    }
}


int
count_lt_threshold (int array[], int array_len, int threshold_val)
{
    int count;
    int i;

    count = 0;
    for (i = 0; i < array_len; i++) {
        if (array[i] < threshold_val) {
            ++count;
        }
    }
    return count;
}


int
compare_ints (const void *a, const void *b)
{
    int x, y;
    x = *((int *) a);
    y = *((int *) b);
    if (x < y) {
        return -1;
    } else if (x > y) {
        return 1;
    }
    return 0;
}

void ecall_qsort(void) {
    int array_len;
    int num_passes, pass;
    int i;
    int threshold_val;
    int is_sorted;
    int count1, count2;
    int *array;
    unsigned nbytes;

    int max_array_len = (1 << 10);

    nbytes = max_array_len * sizeof(int);
    array = (int *) malloc(nbytes);
    if (array == 0) {
        return;
    }

    for (array_len = 512; array_len <= max_array_len; array_len <<= 1) {
	num_passes = (1 << 27) / array_len;
        init_array(array, array_len);

        threshold_val = 1 << 30;
	count1 = 0;
        for (pass = 0; pass < num_passes; pass++) {
            for (i = 0; i < array_len; i++) {
                if (array[i] < threshold_val) {
                    //++count1;
                    count1 += array[i];
                }
            }
        }

	qsort(array, array_len, sizeof(int), compare_ints);
        is_sorted = 1;
        for (i = 0; i < array_len - 1; i++) {
            if (array[i] > array[i+1]) {
                is_sorted = 0;
                break;
            }
        }

	count2 = 0;
        for (pass = 0; pass < num_passes; pass++) {
            for (i = 0; i < array_len; i++) {
                if (array[i] < threshold_val) {
                    //++count2;
                    count2 += array[i];
                }
            }
        }
    }
}
