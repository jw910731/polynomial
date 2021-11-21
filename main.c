#include "hash_map.h"
#include "polynomial.h"
#include "setup.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static void poly_free_adapter(void *p) {
    polynomial_free((struct polynomial *)p);
}

int main() {
    bool run = true;
    HashTable *table = table_create(poly_free_adapter);
    while (run) {
        int cmd;
        printf("1) Input polynomial\n"
               "2) Display polynomial\n"
               "3) Query polynomial term coefficient\n"
               "4) Add term to polynomial\n"
               "5) Remove term from polynomial\n"
               "6) Add polynomials\n"
               "7) Subtract polynomials\n"
               "8) Multiply polynomials\n"
               "0) Quit\n");
        printf("Input one of the option: ");
        int ret = scanf("%d", &cmd);
        if (ret < 1) {
            // clear invalid input in input stream
            for (char c = getchar(); c != '\n' && c > 0; c = getchar())
                ;
            printf("<Invalid Input>");
            if (ret == -1)
                return 0;
            continue;
        }
        getchar();
        switch (cmd) {
        case 0:
            run = false;
            break;
        case 1: {
            char name[STRING_MAX_LEN], poly_str[STRING_MAX_LEN];
            printf("Input polynomial name: ");
            fgets(name, STRING_MAX_LEN, stdin);
            printf("Input polynomial (in ax ^ b + cx ^ d form): ");
            fgets(poly_str, STRING_MAX_LEN, stdin);
            struct polynomial *p = polynomial_parser(poly_str);
            if (p == NULL) {
                printf("Error: invalid polynomial input\n");
                break;
            }
            // item get copied into the data structure (shallow copy)
            table_emplace(table, name, p, sizeof(struct polynomial));
            // free local copy of polynomial (not include shallow copied array)
            free(p);
            break;
        }
        case 2: {
            char name[STRING_MAX_LEN] = {0};
            printf("Input polynomial name: ");
            fgets(name, STRING_MAX_LEN, stdin);
            name[strlen(name)] = 0;
            Item *itm = table_query(table, name);
            if (itm == NULL) {
                printf("Error: cannot find polynomial\n");
                break;
            }
            struct polynomial *p = (struct polynomial *)itm->data;
            fprintf(stdout, "Result: ");
            polynomial_print_fp(p, stdout);
            fprintf(stdout, "\n");
            break;
        }
        case 3: {
            char name[STRING_MAX_LEN] = {0};
            printf("Input polynomial name: ");
            fgets(name, STRING_MAX_LEN, stdin);
            Item *itm = table_query(table, name);
            if (itm == NULL) {
                printf("Error: cannot find polynomial\n");
                break;
            }
            struct polynomial *p = (struct polynomial *)itm->data;
            int exp;
            printf("Input exponential of the query term: ");
            int ret = scanf("%d", &exp);
            if (ret < 1) {
                // clear invalid input in input stream
                for (char c = getchar(); c != '\n' && c > 0; c = getchar())
                    ;
                break;
            }
            getchar();
            double coeff = polynomial_get_term(p, exp);
            printf("Result: ");
            printf("%lg\n", coeff);
            break;
        }
        case 4: {
            char name[STRING_MAX_LEN] = {0};
            printf("Input polynomial name: ");
            fgets(name, STRING_MAX_LEN, stdin);
            Item *itm = table_query(table, name);
            if (itm == NULL) {
                printf("Error: cannot find polynomial\n");
                break;
            }
            struct polynomial *p = (struct polynomial *)itm->data;
            int exp;
            double coeff;
            printf("Input coefficient of adding term: ");
            int ret = scanf("%lf", &coeff);
            if (ret < 1) {
                // clear invalid input in input stream
                for (char c = getchar(); c != '\n' && c > 0; c = getchar())
                    ;
                break;
            }
            printf("Input exponential of adding term: ");
            ret = scanf("%d", &exp);
            if (ret < 1) {
                // clear invalid input in input stream
                for (char c = getchar(); c != '\n' && c > 0; c = getchar())
                    ;
                break;
            }
            getchar();
            polynomial_add_term(p, exp, coeff);
            break;
        }
        case 5: {
            char name[STRING_MAX_LEN] = {0};
            printf("Input polynomial name: ");
            fgets(name, STRING_MAX_LEN, stdin);
            Item *itm = table_query(table, name);
            if (itm == NULL) {
                printf("Error: cannot find polynomial\n");
                break;
            }
            struct polynomial *p = (struct polynomial *)itm->data;
            int exp;
            printf("Input exponential of removing term: ");
            int ret = scanf("%d", &exp);
            if (ret < 1) {
                // clear invalid input in input stream
                for (char c = getchar(); c != '\n' && c > 0; c = getchar())
                    ;
                break;
            }
            getchar();
            ret = polynomial_remove_term(p, exp);
            if (ret) {
                printf("Error: term not found\n");
            }
            break;
        }
        case 6: {
            char name1[STRING_MAX_LEN] = {0}, name2[STRING_MAX_LEN];
            printf("Input adder's name (use enter to separate): ");
            fgets(name1, STRING_MAX_LEN, stdin);
            fgets(name2, STRING_MAX_LEN, stdin);
            Item *itm1 = table_query(table, name1),
                 *itm2 = table_query(table, name2);
            if (itm1 == NULL || itm2 == NULL) {
                printf("Error: cannot find polynomial");
                break;
            }
            struct polynomial *p;
            polynomial_add(p, itm1->data, itm2->data);
            fprintf(stdout, "Result: ");
            polynomial_print_fp(p, stdout);
            fputc('\n', stdout);
            break;
        }
        case 7: {
            char name1[STRING_MAX_LEN] = {0}, name2[STRING_MAX_LEN];
            printf("Input subtractors' name (use enter to separate): ");
            fgets(name1, STRING_MAX_LEN, stdin);
            fgets(name2, STRING_MAX_LEN, stdin);
            Item *itm1 = table_query(table, name1),
                 *itm2 = table_query(table, name2);
            if (itm1 == NULL || itm2 == NULL) {
                printf("Error: cannot find polynomial");
                break;
            }
            struct polynomial *p;
            polynomial_sub(p, itm1->data, itm2->data);
            fprintf(stdout, "Result: ");
            polynomial_print_fp(p, stdout);
            fputc('\n', stdout);
            break;
        }
        case 8: {
            char name1[STRING_MAX_LEN] = {0}, name2[STRING_MAX_LEN];
            printf("Input subtractors' name (use enter to separate): ");
            fgets(name1, STRING_MAX_LEN, stdin);
            fgets(name2, STRING_MAX_LEN, stdin);
            Item *itm1 = table_query(table, name1),
                 *itm2 = table_query(table, name2);
            if (itm1 == NULL || itm2 == NULL) {
                printf("Error: cannot find polynomial");
                break;
            }
            struct polynomial *p;
            polynomial_mul(p, itm1->data, itm2->data);
            fprintf(stdout, "Result: ");
            polynomial_print_fp(p, stdout);
            fputc('\n', stdout);
            break;
        }
        default:
            printf("<Invalid option!>\n");
            break;
        }
    }
    table_free(&table);
    return 0;
}
