
// An implementation of command
// $ grep -v usr < /etc/passwd | wc -l > result.txt
//
// By John Lee, 2017/5/22

#include <stdio.h>  // for fprintf
#include <stdlib.h>  // for exit
#include <fcntl.h>  // for open
#include <unistd.h>  // for fork, dup2, close, execlp
#include <sys/wait.h>  // for wait

int main ()
{
    int fd[2], sv;
    pipe (fd);
    if (fork () == 0)
    {
        dup2 (open ("/etc/passwd", O_RDONLY), 0);
        dup2 (fd[1], 1);
        close (fd[0]); close (fd[1]);
        execlp ("grep", "grep", "-v", "usr", NULL);
    }
    else
    {
        dup2 (open ("result.txt", O_WRONLY | O_CREAT, 0666), 1);
        dup2 (fd[0], 0);
        close (fd[0]); close (fd[1]);
        execlp ("wc", "wc", "-l", NULL);
    }
}
