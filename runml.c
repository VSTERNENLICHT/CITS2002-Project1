//  CITS2002 Project 1 2024
//  Student1:   23902644   Seoyoung Park
//  Student2:   23915299   Harper Chen
//  Platform:   Apple

// Compile this program with:
//      cc -std=c11 -Wall -Werror -o runml runml.c

// Please add comments and check if it runs without any errors before you push !!!


#include <stdio.h>	// prinf, fprintf, stderr, fopen
#include <stdlib.h>	// exit, EXIT_SUCCESS, atoi
#include <string.h>	// strlen
#include <ctype.h>	// islower

int is_ml(char *str, int len) {  // Checks if it is ml file (if the file name ends with .ml)

}

int main(int argcount, char *argvalue[])
{
    if (argcount > 2 || argcount < 1) {     // Checks if the number of argument is 1, and stderr occurs if it is more than 2 or 0.
        fprintf(stderr, "%s: program expected 1 argument, but instead received %i\n", argvalue[0], argcount-1);
        exit(EXIT_FAILURE);
    }

    int len = strlen(argvalue[1]);  // The length of the file path that passed via argument
    if (is_ml(argvalue[1], len)) {

    }
}