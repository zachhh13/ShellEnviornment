#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "token.h"
char** tokenizeInput(char* input) {
    char** result = malloc(255 * sizeof(char*));
    char* token;
    int count = 0;
    int inQuotes = 0;
    int startIndex = 0;


if(input[strlen(input) - 1] == '\n') {
input[strlen(input) - 1] = '\0';
}



    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] == '"') {
            inQuotes = !inQuotes; // Toggle inQuotes when a double quote is found
            if (!inQuotes) {
                // If the closing quote is found, create a token without quotes
                token = malloc((i - startIndex) * sizeof(char));
                strncpy(token, input + startIndex + 1, i - startIndex - 1);
                token[i - startIndex - 1] = '\0';
                result[count++] = token;
                startIndex = i + 1;
            }
        } else if (
                        (
                         input[i] == '(' || input[i] == ')'|| input[i] == '<' || input[i] == '>' || input[i] == '|' ||
                         input[i] == ';' ||
                         input[i] == ' ' || input[i] == '\t' || input[i] == '\n') && !inQuotes) {
            if (i > startIndex) {
                // Create a token if there are characters between startIndex and i
                token = malloc((i - startIndex + 1) * sizeof(char));
                strncpy(token, input + startIndex, i - startIndex);
                token[i - startIndex] = '\0';
                result[count++] = token;
            }
            if (input[i] == ';' || input[i] == '(' || input[i] == ')'|| input[i] == '<' || input[i] == '>' || input[i] == '|') {
                // Create a semicolon token
                token = malloc(2 * sizeof(char));
                token[0] = input[i];
                token[1] = '\0';
                result[count++] = token;
            }
            startIndex = i + 1;
       	}
    }

    // Handle the last token if it is not inside quotes
    if (!inQuotes && startIndex < strlen(input)) {
        token = malloc((strlen(input) - startIndex + 1) * sizeof(char));
        strncpy(token, input + startIndex, strlen(input) - startIndex);
        token[strlen(input) - startIndex] = '\0';
        result[count++] = token;
    }

    result[count] = NULL; // Set the last element to NULL

    return result;
}


void freeTokens(char** tokens) {
    if (tokens == NULL) {
        return;
    }
    for (int i = 0; tokens[i] != NULL; i++) {
        free(tokens[i]);
    }
    free(tokens);
}
