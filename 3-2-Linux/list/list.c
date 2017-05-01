
// A naive implementation of list on Linux
//
// By John Lee, 2017/5/1
//
// Usage: list [-a] [-r] [-l <min-size>] [-h <max-size>] [-m <modify-day>] [path ...]

#include <stdio.h>  // for printf
#include <stdlib.h>  // for malloc/free
#include <time.h>  // for time
#include <errno.h>  // for errno
#include <string.h>  // for strcmp, strlen

#include <sys/types.h>  // for xxx_t
#include <unistd.h>  // for getopt
#include <getopt.h>  // for getopt (fallback)
#include <dirent.h>  // for DIR, dirent, opendir, readdir, closedir
#include <sys/stat.h>  // for stat

struct Options
{
    const char *argv0;
    int rFlag;  // recursive
    int aFlag;  // all (including leading dot)
    off_t lSize;  // lower bound of size
    off_t hSize;  // upper bound of size
    time_t mTime;  // modify time in seconds
};

void Print (const struct Options *pOpts, const char *pName, int firstFlag)
{
    struct stat status;
    if (stat (pName, &status) == -1)
    {
        fprintf (stderr, "%s (stat): cannot access \'%s\': %s\n",
                 pOpts->argv0, pName, strerror (errno));
        return;
    }

    const mode_t stat_mode = status.st_mode;
    const off_t stat_size = status.st_size;
    const time_t stat_mtime = status.st_mtime;

    if (S_ISREG (stat_mode))
    {
        // Test mTime, lSize, hSize
        int regFlag =
            (!pOpts->mTime || time (NULL) - stat_mtime <= pOpts->mTime) &&
            (!pOpts->lSize || stat_size >= pOpts->lSize) &&
            (!pOpts->hSize || stat_size <= pOpts->hSize);

        if (regFlag || firstFlag)
        {
            char timeStr[64];
            strftime (timeStr, 64, "%m-%d %H:%M", localtime (&stat_mtime));
            printf ("%s (%ld byte) (%s)\n", pName, stat_size, timeStr);
        }
    }
    else if (S_ISDIR (stat_mode))
    {
        // NOT Test mTime, lSize, hSize
        printf ("%s\n", pName);

        // Open this directory
        DIR *pDir = opendir (pName);
        if (pDir == NULL)
        {
            fprintf (stderr, "%s (opendir): cannot access \'%s\': %s\n",
                     pOpts->argv0, pName, strerror (errno));
            return;
        }

        // Traverse this directory
        struct dirent *pDirEntry;
        while ((pDirEntry = readdir (pDir)) != NULL)
        {
            const char *pEntryName = pDirEntry->d_name;

            // Test aFlag
            if (!pOpts->aFlag && pEntryName[0] == '.')
                continue;

            // Test special directory
            if (strcmp (pEntryName, ".") == 0 ||
                strcmp (pEntryName, "..") == 0)
                continue;

            // Construct new name
            size_t len1 = strlen (pName);
            size_t len2 = strlen (pEntryName);
            char *newName = malloc (len1 + 1 + len2 + 1);

            strcpy (newName, pName);
            newName[len1] = '/';
            strcpy (&newName[len1 + 1], pEntryName);
            newName[len1 + 1 + len2] = 0;

            // Test rFlag, firstFlag
            if (pOpts->rFlag || firstFlag) Print (pOpts, newName, 0);

            free (newName);
        }
        closedir (pDir);
    }
}

int main (int argc, char **argv)
{
    struct Options opts = { 0 };
    opts.argv0 = argv[0];
    {
        int c = 0;
        while ((c = getopt (argc, argv, "ral:h:m:")) != -1)
            switch (c)
            {
            case 'r': { opts.rFlag = 1; break; }
            case 'a': { opts.aFlag = 1; break; }
            case 'l': { opts.lSize = atoi (optarg); break; }
            case 'h': { opts.hSize = atoi (optarg); break; }
            case 'm': { opts.mTime = atoi (optarg) * 24 * 60 * 60; break; }
            default: break;
            }
    }

    size_t argcOpt = optind < argc ? argc - optind : 1;
    const char **argvOpt = malloc (sizeof (const char *) * argcOpt);
    {
        int i = 0;
        while (optind < argc) argvOpt[i++] = argv[optind++];
        if (!i) argvOpt[i++] = ".";
    }

    // For each argument
    for (int i = 0; i < argcOpt; i++)
    {
        if (argcOpt != 1 && i != 0)
            printf ("\n");  // Print blank line between calls
        Print (&opts, argvOpt[i], 1);
    }

    free (argvOpt);
    return 0;
}
