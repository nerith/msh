#ifndef _SHELL_H
#define _SHELL_H

typedef struct env_t {
    int piping;
    int background;
    int npipes;
} env_t;

#define MAX_ARGUMENTS 8192

#endif
