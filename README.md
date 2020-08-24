# operating-systems-challenges
## My Shell
This is a program called myshell which is capable of executing, managing, and monitoring user level programs. This program is similar in purpose and design to everyday shells like bash or tcsh. Myshell is be invoked without any arguments, and will support several different commands. The comands are...
1. wait command causes the shell to wait for any child process to exit
2. waitfor command does the same thing, but waits for a specific child process to exit
3. run command should combine the behavior of start and waitfor. run should start a program, possibly with command line arguments, wait for that particular process to finish, and print the exit status
4. watchdog takes a timeout (in seconds) and a command to run, and then executes the command in the same way as run

## Fractal
This program was created to demonstrate the idea of threads and locks. Essentially, it creates a mandlebrot set using varying amounts of threads. This program uses the escape time algorithm. For each pixel in the image, it starts with the x and y position, and then computes a recurrence relation until it exceeds a fixed value or runs for max iterations. 

## Virtual Memory
In this project, I built a simple but fully functional demand paged virtual memory.
