#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_ENTRIES 10000
#define MAX_TIMESTAMP_LEN 20
#define MAX_VALUE_LEN 10

typedef struct {
    char timestamp[MAX_TIMESTAMP_LEN];
    double value;
} TimeValue; //struct that stores each timestamp and temperature


typedef struct {
    TimeValue entries[MAX_ENTRIES];
    int count;
} DataSet; //struct that stores an array of TimeValue entries (timestamps and temperatures), and the count of total entries


char* readTextFile(const char *filename); //reads the txt file and stores its data into a string and returns it
void trimData(const char *buffer, DataSet *dataset); //trims and filters the data stored in the buffer, parses the timestamps and temperatures and stores them into a DataSet struct
void swap(double *a, double *b); //classic swap function needed for the quicksort function
void quickSort(TimeValue *array, int left, int right); //uses quicksort algorithm to sort a TimeValue array
void merge(TimeValue *array, int start, int mid, int end); //used in the mergesort function
void mergeSort(TimeValue *array, int start, int end); //uses mergesort algorithm to sort a TimeValue array
double measureSortingTime(TimeValue *array, int count, void (*sortFunction)(TimeValue*, int, int)); /*measures CPU time for sorting algorithms,
arguments are the array that is being sorted, the total number of entries of the array and a pointer to the function that does the sort(quicksort or mergesort)*/

int main() {
    DataSet aarhus_data;
    char* buffer = readTextFile("tempm.txt");

    if (buffer == NULL) {
        printf("Failed to read file. Exiting.\n");
        return 1;
    }

    trimData(buffer, &aarhus_data);
    DataSet aarhus_data_copy = aarhus_data;
    free(buffer); // free the allocated memory for the buffer

    double quick_sort_time = measureSortingTime(aarhus_data.entries, aarhus_data.count, quickSort);
    printf("Quick sort results:\n\n");
    for (int i = 0; i < aarhus_data.count; i++) {
        printf("%d. Timestamp: %s | Temperature: %.1f\n", i+1, aarhus_data.entries[i].timestamp, aarhus_data.entries[i].value);
    }
    printf("=================================\n");
    printf("Total entries: %d\n", aarhus_data.count);

    double merge_sort_time = measureSortingTime(aarhus_data_copy.entries, aarhus_data_copy.count, mergeSort);
    printf("=================================\n");
    printf("Merge sort results:\n\n");
    for (int i = 0; i < aarhus_data_copy.count; i++) {
        printf("%d. Timestamp: %s | Temperature: %.1f\n", i+1, aarhus_data_copy.entries[i].timestamp, aarhus_data_copy.entries[i].value);
    }
    printf("=================================\n");
    printf("Total entries: %d\n", aarhus_data_copy.count);
    printf("=================================\n");
    printf("Quick sort time: %.3f seconds\n", quick_sort_time);
    printf("Merge sort time: %.3f seconds\n", merge_sort_time);
    if (quick_sort_time > merge_sort_time) {
        printf("Merge sort is faster than quick sort.\n");
    }
    else if (quick_sort_time < merge_sort_time) {
        printf("Quick sort is faster than merge sort.\n");
    }
    else {
        printf("Both algorithms are equal in speed.\n");
    }

    return 0;
}

char* readTextFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file\n");
        return NULL;
    }

    fseek(file, 0, SEEK_END); //move pointer to eof
    long size = ftell(file); //get the size of the file in bytes
    rewind(file); //get the pointer back to the beginning of the file

    char *buffer = (char *)malloc(size + 1); // +1 is for the '\0' character
    if (buffer == NULL) {
        printf("Error allocating memory\n");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, size, file); //read the file into the buffer
    buffer[size] = '\0'; //add the '\0' character to the end of the buffer

    fclose(file);

    return buffer;
}

void trimData(const char *buffer, DataSet *dataset) {
    const char *p = buffer;
    dataset->count = 0;

    while (*p && dataset->count < MAX_ENTRIES) {
        // Look for pattern: "2014-"
        const char *timestamp_start = strstr(p, "2014-");
        if (!timestamp_start) break;

        // Back up to find the opening quote
        const char *quote_start = timestamp_start - 1;
        while (quote_start > p && *quote_start != '"' && *quote_start != '\'' &&
               *quote_start != '"' && *quote_start != '"') {
            quote_start--;
        }

        if (*quote_start == '"' || *quote_start == '\'' || *quote_start == '"' || *quote_start == '"') {
            // Find the end of timestamp
            const char *timestamp_end = timestamp_start;
            while (*timestamp_end && *timestamp_end != '"' && *timestamp_end != '\'' &&
                   *timestamp_end != '"' && *timestamp_end != '"') {
                timestamp_end++;
            }

            // Copy timestamp
            size_t timestamp_length = timestamp_end - timestamp_start;
            if (timestamp_length < MAX_TIMESTAMP_LEN) {
                strncpy(dataset->entries[dataset->count].timestamp, timestamp_start, timestamp_length);
                dataset->entries[dataset->count].timestamp[timestamp_length] = '\0';

                // Find the colon and value
                const char *colon_pos = strchr(timestamp_end, ':'); //returns the location of the first occurrence of the ':' character
                if (colon_pos) {
                    const char *value_start = colon_pos + 1;

                    // Skip whitespace and quotes
                    while (*value_start && (*value_start <= ' ' || *value_start == '"' ||
                           *value_start == '\'' || *value_start == '"' || *value_start == '"')) {
                        value_start++;
                    }


                    if (*value_start && (isdigit(*value_start) || *value_start == '-' ||
                        *value_start == '+' || *value_start == '.')) {
                        char *value_end;
                        dataset->entries[dataset->count].value = strtod(value_start, &value_end); // convert to double

                        if (value_end > value_start) {
                            dataset->count++;
                        }
                    }
                }
            }
        }

        // Move past this timestamp for next iteration
        p = timestamp_start + 1;
    }
}

void swap(double *a, double *b) {
    double temp = *a;
    *a = *b;
    *b = temp;
}

void quickSort(TimeValue *array, int left, int right) {
    int pivot, leftArrow, rightArrow;
    leftArrow = left;
    rightArrow = right;
    pivot = array[(left + right) / 2].value;
    while (leftArrow <= rightArrow) {
        while (array[leftArrow].value < pivot) leftArrow++;
        while (array[rightArrow].value > pivot) rightArrow--;
        if (leftArrow <= rightArrow) {
            swap(&array[leftArrow].value, &array[rightArrow].value);
            leftArrow++;
            rightArrow--;
        }
    }
    if (left < rightArrow) quickSort(array, left, rightArrow);
    if (right > leftArrow) quickSort(array, leftArrow, right);
}

void merge(TimeValue *array, int start, int mid, int end) {
    TimeValue temp[end - start + 1];
    int i = start;
    int j = mid + 1;
    int k = 0;
    while (i <= mid && j <= end) {
        if (array[i].value <= array[j].value) {
            temp[k++] = array[i++];
        }
        else temp[k++] = array[j++];
    }
    while (i <= mid) temp[k++] = array[i++];
    while (j <= end) temp[k++] = array[j++];
    for (i = start; i <= end; i++) array[i] = temp[i - start];
}

void mergeSort(TimeValue *array, int start, int end) {
    if (start < end) {
        int mid = (start + end) / 2;
        mergeSort(array, start, mid);
        mergeSort(array, mid + 1, end);
        merge(array, start, mid, end);
    }
}

double measureSortingTime(TimeValue *array, int count, void (*sortFunction)(TimeValue*, int, int)) {
    clock_t start_time, end_time;

    // Record start time
    start_time = clock();

    // Execute the sorting function
    if (*sortFunction == quickSort) {
        quickSort(array, 0, count - 1);
    } else if (*sortFunction == mergeSort) {
        mergeSort(array, 0, count - 1);
    }

    // Record end time
    end_time = clock();

    // Calculate execution time in seconds
    double cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;


    return cpu_time_used;
}
