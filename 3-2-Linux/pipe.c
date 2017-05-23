
// An implementation of command
// $ grep ¨Cv usr < /etc/passwd | wc ¨Cl > result.txt
//
// By John Lee, 2017/5/22

#include <stdio.h>  // for fprintf
#include <stdlib.h>  // for exit
#include <fcntl.h>  // for open
#include <unistd.h>  // for fork, dup2, close, execlp
#include <sys/wait.h>  // for wait

int main (int argc, char *argv[])
{
    int fd[2], sv;
    pipe (fd);
    if (fork () == 0)
    {
        int f = open ("/etc/passwd", O_RDONLY);
        if (f != -1)
        {
            dup2 (f, 0);
            dup2 (fd[1], 1); close (fd[0]); close (fd[1]);
            execlp ("grep", "grep", "-v", "usr", NULL);
        }
        fprintf (stderr, "exec failed: grep\n");
        exit (1);
    }
    else if (fork () == 0)
    {
        int f = open ("result.txt", O_WRONLY | O_CREAT, 0666);
        if (f != -1)
        {
            dup2 (f, 1);
            dup2 (fd[0], 0); close (fd[0]); close (fd[1]);
            execlp ("wc", "wc", "-l", NULL);
        }
        fprintf (stderr, "exec failed: wc\n");
        exit (1);
    }
    close (fd[0]); close (fd[1]);
    wait (&sv); wait (&sv);
    return 0;
}
