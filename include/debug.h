#ifndef DEBUG_H
#define DEBUG_H

/* Predefined values dictating the debug options */
#define USER_CALL 0x01
#define LIB_CALL 0x02
#define SYS_CALL 0x04
#define ARG_CALL 0x10
#define RETURN_CALL 0x20

extern int DBGVAL;

/* Function that, on enter, prints function name and args based on the DBGVAL*/
int enter(int enter_policy, const char *function_name, ...);

/* Function that, on leave, prints function name and retval based on the DBGVAL*/
int leave(int leave_policy, const char *function_name, int retval);

#endif