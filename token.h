#ifndef TOKEN_H
#define TOKEN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Main Function
char** tokenizeInput(char* input);

// Function to free tokens
void freeTokens(char** tokens);

#endif /* TOKEN_H */
