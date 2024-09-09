#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h> 
#include <unistd.h>


// Define a simple structure to track variables
typedef struct {
    char name[50];
    bool declared;
} Variable;

// Declare an array to store variables
Variable variables[50];
int varCount = 0;
bool currentFunctionReturnsValue = false;  // Track if the current function should return a value

// Check if the file extension is .ml
bool checkFileExtension(const char* filePath) {
    size_t len = strlen(filePath);
    return (len >= 3 && strcmp(filePath + len - 3, ".ml") == 0);
}

// Check if the syntax is valid
bool isValidSyntax(const char* line) {
    if (line[0] == '#') return true;
    if (strncmp(line, "print", 5) == 0 || strncmp(line, "\tprint", 6) == 0) return true;
    if (strncmp(line, "return", 6) == 0 || strncmp(line, "\treturn", 7) == 0) return true;
    if (strstr(line, "<-") != NULL) return true;
    if (isalpha(line[0])) return true;
    return false;
}

// Check if a variable has been declared before
bool isVariableDeclared(const char* var) {
    for (int i = 0; i < varCount; i++) {
        if (strcmp(variables[i].name, var) == 0) {
            return variables[i].declared;
        }
    }
    return false;
}

// Mark a variable as declared
void declareVariable(const char* var) {
    strcpy(variables[varCount].name, var);
    variables[varCount].declared = true;
    varCount++;
}

// Modify this function to handle both void and non-void functions
void translateToC(FILE *outputFile, const char* line) {
    if (line[0] == '#') {
        fprintf(outputFile, "// %s\n", line + 1);
        return;
    }

    // Handle function definition
    if (strncmp(line, "function", 8) == 0) {
        char funcName[50], param1[50], param2[50];
        sscanf(line, "function %s %s %s", funcName, param1, param2);
        
        // Check if the function is an arithmetic function that returns a value (e.g., multiply)
        if (strcmp(funcName, "multiply") == 0 || strcmp(funcName, "add") == 0) {
            currentFunctionReturnsValue = true;  // Mark this function as returning a value
            fprintf(outputFile, "double %s(double %s, double %s) {\n", funcName, param1, param2);
        } else {
            currentFunctionReturnsValue = false;  // Default to void functions
            fprintf(outputFile, "void %s(double %s, double %s) {\n", funcName, param1, param2);
        }
        return;
    }

    // Handle return statements for non-void functions
    if (strncmp(line, "return", 6) == 0 || strncmp(line, "\treturn", 7) == 0) {
        char returnExpr[200];
        sscanf(line + (line[0] == '\t' ? 7 : 6), "%[^\n]", returnExpr);
        if (currentFunctionReturnsValue) {
            fprintf(outputFile, "\treturn %s;\n", returnExpr);  // Return the expression
        }
        return;
    }

    // Handle print statement inside a function
    if (strncmp(line, "\tprint", 6) == 0) {
        char expression[200];
        sscanf(line + 7, "%[^\n]", expression);
        fprintf(outputFile, "\tprintf(\"%%.6f\\n\", (double)(%s));\n", expression);
        return;
    }

    // Handle print statement outside of function
    if (strncmp(line, "print ", 6) == 0) {
        char expression[200];
        sscanf(line + 6, "%[^\n]", expression);
        fprintf(outputFile, "printf(\"%%.6f\\n\", (double)%s);\n", expression);
        return;
    }

    // Handle function call and clean up spaces
    if (strchr(line, '(') != NULL && strchr(line, ')') != NULL) {
        char funcCall[200];
        sscanf(line, "%[^\n]", funcCall);
        char cleanFuncCall[200];
        int i = 0, j = 0;
        while (funcCall[i] != '\0') {
            if (funcCall[i] != ' ' || (i > 0 && funcCall[i - 1] != '(' && funcCall[i - 1] != ')')) {
                cleanFuncCall[j++] = funcCall[i];
            }
            i++;
        }
        cleanFuncCall[j] = '\0';
        fprintf(outputFile, "%s;\n", cleanFuncCall);
        return;
    }

    // Handle assignment statement
    else if (strstr(line, "<-") != NULL) {
        char var[50], expr[200];
        sscanf(line, "%s <- %[^\n]", var, expr);
        if (!isVariableDeclared(var)) {
            fprintf(outputFile, "double %s = %s;\n", var, expr);
            declareVariable(var);
        } else {
            fprintf(outputFile, "%s = %s;\n", var, expr);
        }
        return;
    }

    // Handle end of function block
    if (strncmp(line, "end", 3) == 0) {
        fprintf(outputFile, "}\n");
        return;
    }

    // Handle other expressions or function calls
    if (isalpha(line[0])) {
        fprintf(outputFile, "%s;\n", line);
    }
}

// Compile the generated C code
void compileCFile(const char* cFileName, int pid) {
    char command[256];
    snprintf(command, sizeof(command), "cc -std=c11 -o ml-%d %s", pid, cFileName);
    int result = system(command);
    if (result == 0) {
        printf("C code compiled successfully!\n");
    } else {
        fprintf(stderr, "Error during compilation!\n");
        exit(EXIT_FAILURE);
    }
}

// Run the compiled executable
void runExecutable(char* cFileName) {
    int len = strlen(cFileName);
    cFileName[len - 2] = '\0';
    char executableFile[100];
    snprintf(executableFile, sizeof(executableFile), "./%s", cFileName);
    int result = system(executableFile);
    if (result != 0) {
        fprintf(stderr, "Error running the executable!\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2 || !checkFileExtension(argv[1])) {
        fprintf(stderr, "Usage: %s program.ml\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Error opening file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    int pid = getpid();
    char cFileName[50];
    snprintf(cFileName, sizeof(cFileName), "ml-%d.c", pid);

    FILE *cFile = fopen(cFileName, "w");
    if (!cFile) {
        fprintf(stderr, "Error creating C file\n");
        exit(EXIT_FAILURE);
    }

    // Write standard headers for the C code
    fprintf(cFile, "#include <stdio.h>\n\n");

    FILE *mainFuncFile = tmpfile();
    FILE *funcDefFile = tmpfile();

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;
        if (isValidSyntax(line)) {
            if (strncmp(line, "function", 8) == 0) {
                translateToC(funcDefFile, line);
            } else if (strncmp(line, "\t", 1) == 0) {
                translateToC(funcDefFile, line);
            } else {
                translateToC(mainFuncFile, line);
            }
        } else {
            fprintf(stderr, "Syntax error in line: %s\n", line);
            fclose(file);
            fclose(cFile);
            fclose(funcDefFile);
            fclose(mainFuncFile);
            exit(EXIT_FAILURE);
        }
    }

    fseek(funcDefFile, 0, SEEK_SET);
char funcDefLine[256];
bool hasFunctionDefinitions = false;  // Flag to track if any function definitions were written

// Loop through the function definitions and write them to the C file
while (fgets(funcDefLine, sizeof(funcDefLine), funcDefFile)) {
    fputs(funcDefLine, cFile);
    hasFunctionDefinitions = true;  // Mark that we have function definitions
}

// After writing all function definitions, only close the function with `}` if we wrote any definitions
if (hasFunctionDefinitions) {
    fprintf(cFile, "}\n");
}

    

    fseek(mainFuncFile, 0, SEEK_SET);
    fprintf(cFile, "int main() {\n");
    char mainFuncLine[256];
    while (fgets(mainFuncLine, sizeof(mainFuncLine), mainFuncFile)) {
        fputs(mainFuncLine, cFile);
    }

    fprintf(cFile, "return 0;\n}\n");

    fclose(file);
    fclose(cFile);
    fclose(funcDefFile);
    fclose(mainFuncFile);

    compileCFile(cFileName, pid);
    runExecutable(cFileName);

    return 0;
}
