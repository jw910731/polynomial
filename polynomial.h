#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <stdio.h>

struct term {
    double coeff;
    int exp;
};

struct polynomial {
    int size, cap;
    struct term *terms;
};

struct polynomial *polynomial_init();
void polynomial_free(struct polynomial *);
struct polynomial *polynomial_parser(const char *);

void polynomial_print_fp(const struct polynomial *, FILE *fp);
double polynomial_get_term(const struct polynomial *, int exp);
void polynomial_add_term(struct polynomial *, int exp, double coeff);
int polynomial_remove_term(struct polynomial *, int exp);

void polynomial_add(struct polynomial *dest, const struct polynomial *a,
                    const struct polynomial *b);
void polynomial_sub(struct polynomial *dest, const struct polynomial *a,
                    const struct polynomial *b);
void polynomial_mul(struct polynomial *dest, const struct polynomial *a,
                    const struct polynomial *b);

#endif
