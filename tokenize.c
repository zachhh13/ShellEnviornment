#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "token.h"

int main() {
    char input[255];
    fgets(input, sizeof(input), stdin);

    char** result = tokenizeInput(input);

    int j = 0;
    while (result[j] != NULL) {
        printf("%s\n", result[j]);
        j++;
    }

//    freeTokens(result);
    return 0;
}
