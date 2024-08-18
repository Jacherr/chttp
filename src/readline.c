#include "readline.h"

#define CH_NEWLINE '\n'
#define CH_NULL '\0'

/// @brief Read a line from a file descriptor. A line is defined as a sequence of bytes delimited by a newline (\\n)
/// @param fd File descriptor to read from.
/// @param buffer The buffer to read into.
/// @param n The maximum size of the buffer. Any bytes read beyond the maximum size are dropped, with consideration for a final null byte.
/// @return The number of bytes read, excluding the final null byte.
ssize_t
read_line(int fd, void *buffer, size_t n)
{
    if (n <= 0 || buffer == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    ssize_t last_read; // bytes fetched in last read()
    size_t total_read; // total bytes read
    char *buf;         // line read
    char next;         // next char read

    // cannot perform pointer arithmetic on void*
    buf = buffer;

    total_read = 0;
    for (;;)
    {
        // fetch next byte
        last_read = read(fd, &next, 1);
        if (last_read == -1)
        {
            // error
            if (errno == EINTR)
            {
                // interrupted: try again
                continue;
            }
            else
            {
                // any other generic error
                return -1;
            }
        }
        else if (last_read == 0)
        {
            // EOF
            if (total_read == 0)
            {
                // we didnt read anything
                return 0;
            }
            else
            {
                // we read some bytes; append null
                break;
            }
        }
        else
        {
            // last_read must be 1
            // n - 1 to leave space for final null
            if (total_read < n - 1)
            {
                total_read++;
                *buf++ = next;
            }

            if (next == CH_NEWLINE)
            {
                // end of line
                break;
            }
        }
    }

    // append null to end
    *buf = CH_NULL;
    return total_read;
}