#ifndef DEBUG_H
#define DEBUG_H

/* Predefined values dictating the debug options */
#define USER_CALL 0x01
#define LIB_CALL 0x02
#define SYS_CALL 0x04
#define ARG_CALL 0x10
#define RETURN_CALL 0x20

/* Colors */
#define KNRM "\033[0m"
#define KRED "\033[1;31m"
#define KGRN "\033[1;32m"
#define KYLW "\033[1;33m"

extern int DBGVAL;

/* Macro function that, on enter, prints function name and args based on the DBGVAL*/
#define enter(enter_policy, fmt, ...)                          \
    if (enter_policy & DBGVAL)                                 \
    {                                                          \
        fprintf(stderr, KYLW "Entering... %s(", __FUNCTION__); \
        if (ARG_CALL & DBGVAL)                                 \
        {                                                      \
            fprintf(stderr, fmt, __VA_ARGS__);                 \
        }                                                      \
        fprintf(stderr, ")\n" KNRM);                           \
    }

/* Macro function that, on leave, prints function name and retval based on the DBGVAL*/
#define leave(leave_policy, fmt, retval)                             \
    if (leave_policy & DBGVAL)                                       \
    {                                                                \
        fprintf(stderr, KYLW "Leaving... %s()", __FUNCTION__);       \
        if (RETURN_CALL & DBGVAL)                                    \
        {                                                            \
            fprintf(stderr, " with return value '" fmt "'", retval); \
        }                                                            \
        fprintf(stderr, "\n" KNRM);                                  \
    }

/* Function that, on entering main(), prints function name and args based on the DBGVAL*/
int enter_main(int enter_policy, const char *function_name, int argc, char **argv);
#endif