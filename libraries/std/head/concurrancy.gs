/*
 * class name:  Process
 * description: Create an asynchronous process
 */

class Process{
    int pid;
    adr foo;

    int init(adr foo);
    int start(adr args);
};

Process newProcess(adr foo);

/*
 * class name: Pipe
 * description: Create a pipe
 */
class Pipe{
    int in;
    int out;

    int init();
    int write(adr buf, int len);
    int read(adr buf, int len);
};

Pipe newPipe();

int exit(int status);
int wait(Process p);