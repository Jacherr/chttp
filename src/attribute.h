#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#ifdef __GNUC__
#define NORETURN __attribute__ ((__noreturn__))
#else
#define NORETURN
#endif

#endif