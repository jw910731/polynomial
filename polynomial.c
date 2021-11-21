#include "polynomial.h"

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void polynomial_init(struct polynomial *p) {
    p->size = 0;
    p->cap = 16;
    p->terms = calloc(sizeof(struct term), p->cap);
}

void polynomial_free(struct polynomial *p) {
    free(p->terms);
    free(p);
}

static int term_comp(const void *_a, const void *_b) {
    const struct term *a = (struct term *)_a, *b = (struct term *)_b;
    if (a->exp == b->exp) {
        if (a->coeff == b->coeff)
            return 0;
        return (a->coeff < b->coeff) ? -1 : 1;
    }
    return (a->exp < b->exp) ? -1 : 1;
}

struct polynomial *polynomial_parser(const char *str) {
    struct polynomial *p = calloc(sizeof(struct polynomial), 1);
    polynomial_init(p);
    for (const char *it = str; *it;) {
        int sign = 1;

        // space eater
        for (; isspace(*it) && *it; ++it)
            ;
        if (!*it) // return if end of string
            break;

        // process sign
        if (*it == '-') {
            sign = -1;
            ++it;
        } else if (*it == '+') {
            ++it;
        }

        // space eater
        for (; isspace(*it) && *it; ++it)
            ;
        if (!*it) // return if end of string
            break;

        const char *ed_it = it;
        for (; (isdigit(*ed_it) || *ed_it == '.') && *ed_it; ++ed_it)
            ; // find number end

        // process coefficient
        double coeff = 1.0;
        char *buf = calloc(ed_it - it + 1, sizeof(char));
        strncpy(buf, it, ed_it - it);
        sscanf(buf, "%lf", &coeff);
        free(buf);
        buf = NULL;
        coeff *= sign;
        it = ed_it;

        for (; isspace(*it) && *it; ++it)
            ; // eat all space
        if (!*it) {
            // tmp.emplace_back(coeff, 0);
            if (p->size >= p->cap) {
                p->cap *= 2;
                p->terms = realloc(p->terms, sizeof(struct term) * p->cap);
            }
            p->terms[p->size++] = (struct term){coeff, 0};
            break;
        }

        if (*it == 'x') {
            ++it;
            int exp;
            for (; isspace(*it) && *it; ++it)
                ;
            if (*it == '^') { // exponential not 1
                it++;
                for (; isspace(*it) && *it; ++it)
                    ; // eat all space
                if (!*it) {
                    free(p);
                    return NULL;
                }

                ed_it = it;
                for (; isdigit(*ed_it) && *ed_it; ++ed_it)
                    ;
                buf = calloc(ed_it - it + 1, sizeof(char));
                strncpy(buf, it, ed_it - it);
                // asserted no error will exist
                sscanf(buf, "%d", &exp);
                free(buf);
                buf = NULL;
                it = ed_it; // increment
            } else {
                exp = 1;
            }
            if (p->size >= p->cap) {
                p->cap *= 2;
                p->terms = realloc(p->terms, sizeof(struct term) * p->cap);
            }
            p->terms[p->size++] = (struct term){coeff, exp};
        } else {
            for (; isspace(*it) && *it; ++it)
                ;       // eat all space
            if (!*it) { // there might exist remaining constant term
                if (p->size >= p->cap) {
                    p->cap *= 2;
                    p->terms = realloc(p->terms, sizeof(struct term) * p->cap);
                }
                p->terms[p->size++] = (struct term){coeff, 0};
                break;
            }
            if (*it == '+' || *it == '-' || isspace(*it)) {
                if (p->size >= p->cap) {
                    p->cap *= 2;
                    p->terms = realloc(p->terms, sizeof(struct term) * p->cap);
                }
                p->terms[p->size++] = (struct term){coeff, 0};
                continue;
            }
        }
    }
    qsort(p->terms, p->size, sizeof(struct term), term_comp);
    /*struct polynomial *tmp = (struct polynomial){.size = 0,
                              .cap = p->size,
                              .terms = calloc(p->size, sizeof(struct term))};*/
    struct polynomial *tmp = calloc(1, sizeof(struct polynomial));
    *tmp = (struct polynomial){.size = 0,
                               .cap = p->size,
                               .terms = calloc(p->size, sizeof(struct term))};
    int term = 0;
    bool first = false;
    for (int i = 0; i < p->size; ++i) {
        if (first && tmp->terms[term].exp < p->terms[i].exp) {
            tmp->terms[++term].exp = p->terms[i].exp;
        }
        if (!first) {
            tmp->terms[term].exp = p->terms[i].exp;
            first = true;
        }
        tmp->terms[term].coeff += p->terms[i].coeff;
    }
    tmp->size = term + 1;
    polynomial_free(p);
    return tmp;
}

void polynomial_print_fp(const struct polynomial *p, FILE *fp) {
    bool lp = false;
    for (int i = 0; i < p->size; ++i) {
        if (p->terms[i].coeff != 0) {
            struct term *t = &(p->terms[i]);
            if (lp)
                fprintf(fp, "%c ", "-+"[t->coeff > 0]);
            if (fabs(t->coeff) != 1 || t->exp == 0)
                fprintf(fp, "%lg", ((lp) ? fabs(t->coeff) : t->coeff));
            else if (t->coeff == -1 && !lp) {
                fputc('-', fp);
            }
            if (t->exp != 0) {
                fputc('x', fp);
            }
            if (t->exp > 1) {
                fprintf(fp, "^%d", t->exp);
            }
            fputc(' ', fp);
            lp = true;
        }
    }
}
double polynomial_get_term(const struct polynomial *p, int exp) {
    for (int i = 0; i < p->size; ++i) {
        if (p->terms[i].exp == exp)
            return p->terms[i].coeff;
    }
    return 0;
}
void polynomial_add_term(struct polynomial *p, int exp, double coeff) {
    if (p->size + 1 >= p->cap) {
        p->cap *= 2;
        p->terms = realloc(p->terms, sizeof(struct term) * p->cap);
    }
    for (int i = 0; i < p->size; ++i) {
        if (p->terms[i].exp > exp) {
            // move every item one step further from here to end
            for (int j = p->size - 1; j >= i; --j) {
                p->terms[j + 1] = p->terms[j];
            }
            p->terms[i] = (struct term){.exp = exp, .coeff = coeff};
            p->size++;
            return;
        }
        if (p->terms[i].exp == exp) {
            p->terms[i] = (struct term){.exp = exp, .coeff = coeff};
            return;
        }
    }
    p->terms[p->size++] = (struct term){coeff, exp};
}
int polynomial_remove_term(struct polynomial *p, int exp) {
    for (int i = 0; i < p->size; ++i) {
        if (p->terms[i].exp == exp) {
            for (int j = i + 1; j < p->size; ++j) {
                p->terms[j - 1] = p->terms[j];
            }
            p->size--;
            return 0;
        }
    }
    return 1;
}

void polynomial_add(struct polynomial *dest, const struct polynomial *a,
                    const struct polynomial *b) {
    polynomial_init(dest);
    int i, j;
    for (i = 0, j = 0; i < a->size && j < b->size;) {
        if (a->terms[i].exp == b->terms[j].exp) {
            if (dest->size >= dest->cap) {
                dest->cap *= 2;
                dest->terms = realloc(dest->terms, dest->cap);
            }
            dest->terms[dest->size++] =
                (struct term){.exp = a->terms[i].exp,
                              .coeff = a->terms[i].coeff + b->terms[j].coeff};
            ++i;
            ++j;
        } else if (a->terms[i].exp < b->terms[j].exp) {
            if (dest->size >= dest->cap) {
                dest->cap *= 2;
                dest->terms = realloc(dest->terms, dest->cap);
            }
            dest->terms[dest->size++] = (struct term){
                .exp = a->terms[i].exp, .coeff = a->terms[i].coeff};
            ++i;

        } else {
            if (dest->size >= dest->cap) {
                dest->cap *= 2;
                dest->terms = realloc(dest->terms, dest->cap);
            }
            dest->terms[dest->size++] = (struct term){
                .exp = b->terms[j].exp, .coeff = b->terms[j].coeff};
            ++j;
        }
    }
    for (; i < a->size; ++i) {
        if (dest->size >= dest->cap) {
            dest->cap *= 2;
            dest->terms = realloc(dest->terms, dest->cap);
        }
        dest->terms[dest->size++] =
            (struct term){.exp = a->terms[i].exp, .coeff = a->terms[i].coeff};
    }
    for (; j < b->size; ++j) {
        if (dest->size >= dest->cap) {
            dest->cap *= 2;
            dest->terms = realloc(dest->terms, dest->cap);
        }
        dest->terms[dest->size++] =
            (struct term){.exp = b->terms[j].exp, .coeff = b->terms[j].coeff};
    }
}
void polynomial_sub(struct polynomial *dest, const struct polynomial *a,
                    const struct polynomial *b) {
    polynomial_init(dest);
    int i, j;
    for (i = 0, j = 0; i < a->size && j < b->size;) {
        if (a->terms[i].exp == b->terms[j].exp) {
            if (dest->size >= dest->cap) {
                dest->cap *= 2;
                dest->terms = realloc(dest->terms, dest->cap);
            }
            dest->terms[dest->size++] =
                (struct term){.exp = a->terms[i].exp,
                              .coeff = a->terms[i].coeff - b->terms[j].coeff};
            ++i;
            ++j;
        } else if (a->terms[i].exp < b->terms[j].exp) {
            if (dest->size >= dest->cap) {
                dest->cap *= 2;
                dest->terms = realloc(dest->terms, dest->cap);
            }
            dest->terms[dest->size++] = (struct term){
                .exp = a->terms[i].exp, .coeff = a->terms[i].coeff};
            ++i;

        } else {
            if (dest->size >= dest->cap) {
                dest->cap *= 2;
                dest->terms = realloc(dest->terms, dest->cap);
            }
            dest->terms[dest->size++] = (struct term){
                .exp = b->terms[j].exp, .coeff = -b->terms[j].coeff};
            ++j;
        }
    }
    for (; i < a->size; ++i) {
        if (dest->size >= dest->cap) {
            dest->cap *= 2;
            dest->terms = realloc(dest->terms, dest->cap);
        }
        dest->terms[dest->size++] =
            (struct term){.exp = a->terms[i].exp, .coeff = a->terms[i].coeff};
    }
    for (; j < b->size; ++j) {
        if (dest->size >= dest->cap) {
            dest->cap *= 2;
            dest->terms = realloc(dest->terms, dest->cap);
        }
        dest->terms[dest->size++] =
            (struct term){.exp = b->terms[j].exp, .coeff = -b->terms[j].coeff};
    }
}
void polynomial_mul(struct polynomial *dest, const struct polynomial *a,
                    const struct polynomial *b) {
    polynomial_init(dest);
    for (int i = 0; i < a->size; ++i) {
        struct polynomial accumulator;
        polynomial_init(&accumulator);
        for (int j = 0; j < b->size; ++j) {
            if (accumulator.size >= accumulator.cap) {
                accumulator.cap *= 2;
                accumulator.terms = realloc(accumulator.terms, accumulator.cap);
            }
            accumulator.terms[accumulator.size++] =
                (struct term){.coeff = a->terms[i].coeff * b->terms[j].coeff,
                              .exp = a->terms[i].exp + b->terms[j].exp};
        }
        // tmp = dest + accumulator
        // dest = tmp
        struct polynomial tmp;
        polynomial_add(&tmp, dest, &accumulator);
        dest->cap = tmp.cap;
        dest->size = tmp.size;
        free(dest->terms);
        dest->terms = tmp.terms;
        free(accumulator.terms);
    }
}
