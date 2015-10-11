#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
/*Samuel Doud. A program to take the checkSum of a user-specified file and
number of threads. Notable in this program is that I do not use the strlen()
to take the length of a string from the file. This is because strlen() will
not count beyond any null bytes. Null bytes won't occur in ASCII but they can
occur in a .png or other non-ASCII files. Because of this I use a struct to
pass data to the thread function as it needs both a string to operate on
and its length as its length can only be reliably be determined by the
length returned by the read() function*/
void *calculateSum(void *passedData);/*function definitions*/
int check(char *bufferedSection, int length);
typedef struct ThreadDataStruct
{
    /*Struct which holds the threadID, a string to be checkSum-ed, and a length
    INCLUDING any null (0x00) values!*/
    pthread_t threadID;/*The thread ID is contained within the structure*/
    int stringLength;/*The length that the read() function returned*/
    char *threadString;/*The string that this thread will work on*/
} ThreadDataStruct;
int main(int argc, char *argv[])
{
    if (argc != 3)/*User entered improper number of arguments.*/
    {
        exit(EXIT_FAILURE);/*Close the program*/
    }
    int inFd, checkSum, index, strLength, threadSum;/*int declarations*/
    void *ptr_threadSum; /*A void pointer to take thread return data*/
    checkSum = 0; /*intial value of checkSum to be returned by the function*/
    size_t inSize; /*The number of bytes in the file being read*/
    const int numberOfThreads = atoi(argv[2]); /*number of threads*/
    inFd = open(argv[1], O_RDONLY);/*open the file the with argv[1]*/
    ThreadDataStruct threadData[numberOfThreads];
    struct stat inStat;/*stat structure for the input file*/
    fstat(inFd, &inStat);/*associate inStat with inFd*/
    inSize = inStat.st_size;/*take the size of the in file*/
    /*A crude method of ceiling divison to find the strLength a thread gets*/
    strLength = inSize / numberOfThreads;
    if (inSize % numberOfThreads != 0)
    {
        strLength++;
    }
    for (index = 0; index < numberOfThreads; index++)
    {
        /*Iterate numberOfThread times counted by index, creating a new thread
        also read a string from the file and give it and its length to the curent
        ThreadDataStruct threadData of element index*/
        threadData[index].threadString = (char *)malloc(strLength);
        strLength = read(inFd, threadData[index].threadString, strLength);
        /*Read strLength bytes (or to the end of the file) from the input file
        to the threadData structure at element index*/
        /*put the strLength into the structure*/
        threadData[index].stringLength = strLength;
        /*Create a pthread with the ID in the element index of the threadData.
        Send the thread to calculate sum with the argument of the address of
        the threadData structure at element index*/
        pthread_create(&threadData[index].threadID, NULL, calculateSum, &threadData[index]);
    }
    close(inFd);/*We aren't reading from inFd anymore. Deallocating it*/
    for (index = 0; index < numberOfThreads; index++)
    {
        /*Iterate through numberOfThreead times counted by index, taking the
        return value of the thread at the index-th element of threadData. Then XOR
        it to the variable checkSum and save that value to checkSum. Must use a
        void pointer as that is the return value defined by calculate sum */
        pthread_join(threadData[index].threadID, &ptr_threadSum);
        threadSum = *((int *)ptr_threadSum);/*put the return value in an int**/
        free(ptr_threadSum);/*not using this anymore, deallocate it*/
        /*take the thread of index's sum & ^ to the total. Saving to total*/
        checkSum = checkSum ^ threadSum;
    }
    printf("%d\n", checkSum);/*printing the checkSum*/
    return checkSum;/*Return checkSum. The checksum of the entire file*/
}
void *calculateSum( void *passedData )
{
    /*This function takes the argument, a ThreadDataStruct, pulls the data from
       it and passes it to the check function. Then the function returns the int
       returned from check(). First, cast the argument to a threadData pointer*/
    ThreadDataStruct *threadData = (ThreadDataStruct *) passedData;
    int *length_ptr = (int *)malloc(sizeof(int));
    int *sum_ptr = (int *)malloc(sizeof(int));/*allocate needed memory*/
    *length_ptr = threadData->stringLength;/*The length of the thread string*/
    char *readStr = threadData->threadString; /*The string of the thread*/
    *sum_ptr = check(readStr, *length_ptr);/*get the checksum from the check
    function by passing it the readStr and the length of that string*/
    free(length_ptr);/*Deallocate the pointer length and the string readStr*/
    free(readStr);
    /*free(sum_ptr); I don't know how to deallocate this after its returned*/
    pthread_exit(sum_ptr);/*Pass the address of sum to the parent thread*/
}
int check(char *bufferedSection, int length)
{
    /*this function takes a string bufferedSection and length length. Takes the XOR
    checksum of its parts*/
    int XORint, index;/*int declarations*/
    XORint = 0; /*an inital value for the checkSum value*/
    for (index = 0; index < length; index++)
    {
        /*Take the XORint ^ the index-th index of bufferSection. Save to XORint.*/
        XORint = XORint ^ bufferedSection[index];
    }
    return XORint;/*return XORint. Represents the checksum of bufferedSection*/
}
