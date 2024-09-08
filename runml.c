#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>  // Ensure this line is present and correctly spelled



// Define a simple structure to track variables
typedef struct {
    char name[50];
    bool declared;
} Variable;

// Declare an array to store variables (max 50)
Variable variables[50];
int varCount = 0;

// Check if the file extension is .ml
bool checkFileExtension(const char* filePath) {
    size_t len = strlen(filePath);
    return (len >= 3 && strcmp(filePath + len - 3, ".ml") == 0);
}

// Check if the syntax is valid
bool isValidSyntax(const char* line) {
    // Skip comments that start with '#'
    if (line[0] == '#') return true;

    // Check if it is a print statement
    if (strncmp(line, "print", 5) == 0) return true;
    
    // Check if it is an assignment "<-"
    if (strstr(line, "<-") != NULL) return true;
    
    // Check if it is a function call (assuming the function name starts with a letter)
    if (isalpha(line[0])) return true;
    
    // Invalid syntax
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

// Translate ml code into C11 code and write to the output file
// Translate ml code into C11 code and write to the output file
// Translate ml code into C11 code and write to the output file
void translateToC(FILE *outputFile, const char* line) {
    // Skip comments that start with '#'
    if (line[0] == '#') {
        fprintf(outputFile, "// %s\n", line + 1);  // Write comment to C file, excluding the '#' character
        return;
    }

    // Handle function definition
    if (strncmp(line, "function", 8) == 0) {
        char funcName[50], param1[50], param2[50];
        sscanf(line, "function %s %s %s", funcName, param1, param2);  // Parse function name and parameters
        fprintf(outputFile, "void %s(double %s, double %s) {\n", funcName, param1, param2);  // Define function in C
        return;
    }

    // Handle function call
    if (strchr(line, '(') != NULL && strchr(line, ')') != NULL) {
        char funcCall[200];
        sscanf(line, "%[^\n]", funcCall);  // Extract the function call
        fprintf(outputFile, "%s;\n", funcCall);  // Call function in C
        return;
    }

    // Handle print statement with expressions, such as "print a + b"
    if (strncmp(line, "print", 5) == 0) {
        char expression[200];
        sscanf(line + 6, "%[^\n]", expression);  // Extract everything after 'print'

        // Print the expression, check if it's an integer
        fprintf(outputFile,
            "if ((%s) == (int)(%s)) printf(\"%%d\\n\", (int)(%s)); else printf(\"%%.6f\\n\", (double)(%s));\n",
            expression, expression, expression, expression);
        return;
    }

    // Handle assignment statement
    else if (strstr(line, "<-") != NULL) {
        char var[50], expr[200];
        sscanf(line, "%s <- %[^\n]", var, expr);  // Extract the variable and expression

        // If the variable hasn't been declared, declare it as a double
        if (!isVariableDeclared(var)) {
            fprintf(outputFile, "double %s = %s;\n", var, expr);
            declareVariable(var);
        } else {
            // If the variable is already declared, just assign the value
            fprintf(outputFile, "%s = %s;\n", var, expr);
        }
        return;
    }

    // Handle end of function block
    if (strncmp(line, "end", 3) == 0) {
        fprintf(outputFile, "}\n");  // Close function block in C
        return;
    }

    // Handle other expressions or function calls
    if (isalpha(line[0])) {
        fprintf(outputFile, "%s;\n", line);  // Simple handling as a function call
    }
}


// Compile the generated C code
void compileCFile(const char* cFileName) {
    char command[256];
    snprintf(command, sizeof(command), "cc -std=c11 -o generated_program %s", cFileName);
    int result = system(command);  // Compile the C file
    if (result == 0) {
        printf("C code compiled successfully!\n");
    } else {
        fprintf(stderr, "Error during compilation!\n");
        exit(EXIT_FAILURE);
    }
}

// Run the compiled executable
void runExecutable() {
    int result = system("./generated_program");  // Run the compiled executable
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

    // Open a C file to write the translated C code
    FILE *cFile = fopen("generated_program.c", "w");
    if (!cFile) {
        fprintf(stderr, "Error creating C file\n");
        exit(EXIT_FAILURE);
    }

    // Write standard headers for the C code
    fprintf(cFile, "#include <stdio.h>\n\nint main() {\n");

    // Read and process the ml file line by line
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Remove the newline character at the end of each line
        line[strcspn(line, "\n")] = 0;
        
        // Check if the syntax is correct
        if (isValidSyntax(line)) {
            // If the syntax is valid, translate it into C code
            translateToC(cFile, line);
        } else {
            fprintf(stderr, "Syntax error in line: %s\n", line);
            fclose(file);
            fclose(cFile);
            exit(EXIT_FAILURE);
        }
    }

    // End the C program's main function
    fprintf(cFile, "return 0;\n}\n");

    fclose(file);
    fclose(cFile);

    // Compile the generated C code
    compileCFile("generated_program.c");

    // Run the compiled program
    runExecutable();

    return 0;
}