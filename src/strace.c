// LCOV_EXCL_START
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void
print_trace(void)
{
    const size_t max_frames = 10;
    void*        array[max_frames];
    int          size;

    size = backtrace(array, (int)max_frames);
    backtrace_symbols_fd(array, size, STDOUT_FILENO);
    puts("");
}
// LCOV_EXCL_STOP
