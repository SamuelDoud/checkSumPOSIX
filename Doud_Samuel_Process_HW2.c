#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
int check(char *bufferedSection, int length)
{
    //this function takes a string bufferedSection and length length. Takes the XOR checksum of its parts
    int XORint=0, index;// current position in the string and current checksum value
    for (index = 0; index < length; index++)
    {
        XORint = XORint ^ bufferedSection[index];//take the checksum of the current calculation and the current char in the string
    }
    return XORint;//return the calculation
}
int main(int argc, char *argv[])
{
    if (argc != 3)/*User entered improper number of arguments.*/
    {
        exit(EXIT_FAILURE);/*Close the program*/
    }
    int inFd, returnStatus, index;//the file directories for the users supplied file names.
    inFd = open(argv[1], O_RDONLY);//open the file the user passed with argv[1]
    int numberOfProcesses = atoi(argv[2]);//take the number of processes from the user
    struct stat inStat;//stat structures
    fstat(inFd, &inStat);
    int inSize = inStat.st_size;//take the size of the in file
    int strLength = inSize / numberOfProcesses;//How long each process will handle
    if (inSize % numberOfProcesses != 0)//not a perfect divisor... add one
    {
        strLength++;
    }
    int finalSum = 0, tempSum = 0;//sums to be used for XOR calculations
    pid_t childProcesses[numberOfProcesses];//array of processes
    int fd[numberOfProcesses][2];//2d array to handle piping
    char currentJob[strLength];//currentJob is written to right before a fork so that a certain process has a certain string to work with
    for(index = 0; index < numberOfProcesses; index++) //read until index meets the size of the input file
    {
        pipe(fd[index]);//open a pipe for the process about to be created
        strLength = read(inFd,currentJob, strLength);//read the file and pass it to the"currentJob"
        childProcesses[index] = fork();//create a new process within an array of process IDs
        if (childProcesses[index] == 0)//is this process the child process?
        {
            tempSum = check(currentJob, strLength);//get the checksum of this string with the length read
            close(fd[index][0]);//close the input pipe
            write(fd[index][1], &tempSum, sizeof(tempSum));//code to pass that int back up the parent...
            exit(0);//the child process is done computing
        }
    }
    close(inFd);//close the file as it isn't needed anymore
    for(index = 0; index < numberOfProcesses; index++) //this loop goes through each process and waitts for them to complete
    {
        //also performs the XOR calculations
        waitpid(childProcesses[index], &returnStatus, 0);//wait for this particular pid to complete
        close(fd[index][1]);//close the output side
        read(fd[index][0], &tempSum, sizeof(&tempSum));
        finalSum = finalSum ^ tempSum;//take the current calcultion and XOR it to the current thread's and make it the new current calculation
    }
    printf("%d\n", finalSum);//I'm only printing this because it is required by the assignment
    return finalSum;//return the final checkSum
}

