#ifndef TYPE_H
#define TYPE_H

/* union_func implements the operation: b[i] = a[i] op b[i], for i=0,...,len-1. */
typedef void (*union_func)(void *a, void *b, int);

#endif
