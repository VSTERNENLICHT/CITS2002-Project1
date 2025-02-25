//  CITS2002 Project 1 2024
//  Student1:   23902644   Seoyoung Park
//  Student2:   23915299   Harper Chen
//  Platform:   Apple

// Compile this program with: cc -std=c11 -Wall -Werror -o runml runml.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h> 
#include <unistd.h>


// Define a structure to track variables
typedef struct {
    char name[50];
    bool declared;
} Variable;

Variable variables[50]; // Declare an array to store variables
int varCount = 0;
int FuncdefineCount = 0;    // Track the number of open function blocks

// Check if the file extension is .ml
bool checkFileExtension(const char* filePath) {
    size_t len = strlen(filePath);
    return (len >= 3 && strcmp(filePath + len - 3, ".ml") == 0);
}

// Check if the syntax is valid
bool isValidSyntax(const char* line) {
    if (line[0] == '#') return true;        // Skip comments that start with '#'

    if (strncmp(line, "print", 5) == 0) return true;        // Check if it is a print statement
    if (strncmp(line, "\tprint", 6) == 0) return true;

    if (strncmp(line, "return", 6) == 0) return true;       // Check if it is a return statement
    if (strncmp(line, "\treturn",7) == 0) return true;

    if (strncmp(line, "\0", 1) == 0) return true;

    if (strstr(line, "<-") != NULL) return true;        // Check if it is an assignment "<-"
    
    if (isalpha(line[0])) return true;      // Check if it is a function call (assuming the function name starts with a letter)
    
    return false;       // Return false if invalid syntax
}

// Update the variable declaration status
void updateVariableDeclaration(const char* var, bool declaredStatus) {
    for (int i = 0; i < varCount; i++) {
        if (strcmp(variables[i].name, var) == 0) {
            variables[i].declared = declaredStatus;  // Update the declared status
            return;
        }
    }
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

// Translate ml syntax into c syntax
void translateToC(FILE *outputFile, const char* line) {
    if (strncmp(line, "\n", 1) == 0){
        fprintf(outputFile, "\n");  // Write comment to C file, excluding the new line character
        return;
    }

    // Check if there is a '#' and ignore everything after it
    char* commentPos = strchr(line, '#');
    if (commentPos != NULL) {
        *commentPos = '\0';  // Truncate the line at '#'
    }
    
    // Handle function definition
    if (strncmp(line, "function", 8) == 0) {
        char funcName[50];
        char params[5][50] = {{""}};  // Array to store up to 5 parameters (function name + 4 parameters)
        int paramCount = sscanf(line, "function %s %s %s %s %s", funcName, params[0], params[1], params[2], params[3]);

        // Check for a valid number of parameters (function name + up to 4 parameters)
        if (paramCount > 5) {
            fprintf(stderr, "Error: Invalid function definition in line: %s\n", line);
            exit(EXIT_FAILURE);
        }

        fprintf(outputFile, "void %s(", funcName);  // Write the function definition

        // Write each parameter, separating with commas
        for (int i = 1; i < paramCount; i++) {
            if (i > 1) {
                fprintf(outputFile, ", ");
            }
            fprintf(outputFile, "double %s", params[i - 1]);
        }

        // Close the function parameter list and add the function body opening brace
        fprintf(outputFile, ") {\n");
        FuncdefineCount++;
        return;
    }

    // Handle print statement inside a function
    if (strncmp(line, "\tprint", 6) == 0) {
        char expression[200];
        sscanf(line + 7, "%[^\n]", expression);  // Extract everything after 'print'
        // Print the expression inside the function
        fprintf(outputFile, "if ((double)((int)(%s)) == %s) {\n", expression, expression);
        fprintf(outputFile, "\tprintf(\"%%.0f\\n\", (double)%s);\n", expression);
        fprintf(outputFile, "} else {\n");
        fprintf(outputFile, "\tprintf(\"%%.6f\\n\", (double)%s);\n", expression);
        fprintf(outputFile, "}\n");
        return;
    }

    // Handle print statement outside of function
    if (strncmp(line, "print ", 6) == 0) {
        char expression[200];
        sscanf(line + 6, "%[^\n]", expression);  // Extract everything after 'print'
        fprintf(outputFile, "if ((double)((int)(%s)) == %s) {\n", expression, expression);
        fprintf(outputFile, "\tprintf(\"%%.0f\\n\", (double)%s);\n", expression);
        fprintf(outputFile, "} else {\n");
        fprintf(outputFile, "\tprintf(\"%%.6f\\n\", (double)%s);\n", expression);
        fprintf(outputFile, "}\n");
        return;
    }

    // Handle return statement inside a function
    if (strncmp(line, "\treturn", 7) == 0 || strncmp(line, "return", 6) == 0) {
        char expression[200];
        
        if (line[0] == '\t') {
            sscanf(line + 8, "%[^\n]", expression);  // Extract everything after '\treturn'
        } else {
            sscanf(line + 7, "%[^\n]", expression);  // Extract everything after 'return'
        }

        fprintf(outputFile, "\treturn %s;\n", expression);  // Write the return statement
        return;
    }

    // Handle assignment statement
    if (strstr(line, "<-") != NULL) {
        char var[50], expr[200];
        sscanf(line, "%s <- %[^\n]", var, expr);  // Extract the variable and expression
        // If the variable hasn't been declared, declare it as a double
        if (!isVariableDeclared(var)) {
            fprintf(outputFile, "double %s = %s;\n", var, expr);
            declareVariable(var);
        // If the variable is already declared, just assign the value
        } else {
            fprintf(outputFile, "%s = %s;\n", var, expr);
        }
        updateVariableDeclaration(var, false);  // Mark as undeclared
        return;
    }

    // Handle other expressions or function calls
    if (isalpha(line[0])) {
        fprintf(outputFile, "%s;\n", line);  // Simple handling as a function call
    }
}

// Compile the generated C code
void compileCFile(const char* cFileName, int pid) {
    char command[256];
    snprintf(command, sizeof(command), "cc -std=c11 -o ml-%d %s", pid, cFileName);
    int result = system(command);  // Compile the C file
    if (result != 0) {
        fprintf(stderr, "Error during compilation!\n"); 
        exit(EXIT_FAILURE);
    }
}

// Run the compiled executable
void runExecutable(char* cFileName) {
    int len = strlen(cFileName);
    cFileName[len - 2] = '\0';
    const char* originalFileName = cFileName;  // Example file name
    char executableFile[100];  // Buffer to hold the result with "./" prepended

    // Use snprintf to concatenate "./" and the original file name safely
    snprintf(executableFile, sizeof(executableFile), "./%s", originalFileName);

    int result = system(executableFile);  // Run the compiled executable
    if (result != 0) {
        fprintf(stderr, "Error running the executable!\n"); 
        exit(EXIT_FAILURE);
    }
}

// Remove the generated C file and the executable
void cleanUpFiles(const char* cFileName, int pid) {
    // Remove the .c file
    char removeCFile[100];
    snprintf(removeCFile, sizeof(removeCFile), "rm -f %s.c", cFileName);  // -f flag forces removal
    int removeCFileStatus = system(removeCFile);
    if (removeCFileStatus != 0) {
        fprintf(stderr, "Error removing the generated C file: %s\n", cFileName);
    }

    // Remove the executable
    char removeExecutable[100];
    snprintf(removeExecutable, sizeof(removeExecutable), "rm -f ml-%d", pid);
    int removeExecStatus = system(removeExecutable);
    if (removeExecStatus != 0) {
        fprintf(stderr, "Error removing the compiled executable: ml-%d\n", pid);
    }
}

// Function to write the main function body from one file to another
void writeMainFunctionBody(FILE *inputFile, FILE *outputFile) {
    char line[256];
    while (fgets(line, sizeof(line), inputFile)) {
        fputs(line, outputFile);  // Write each line from inputFile to outputFile
    }
}

// Function to process lines, replace "void" with "double", and write to the appropriate file
void processUpperFuncFile(FILE *inputFile, FILE *outputFile, FILE *mainFuncFile, bool funcExists) {
    char line[256];

    while (fgets(line, sizeof(line), inputFile)) {
        // Check if "void" exists in the current line
        if (strstr(line, "void") != NULL) {
            // Replace "void" with "double"
            char *pos = strstr(line, "void");
            if (pos != NULL) {
                char temp[256]; // Create a temporary buffer for the modified line
                strncpy(temp, line, pos - line);  // Copy the part of the line before "void"
                temp[pos - line] = '\0';  // Null-terminate the string
                
                // Concatenate "double" and the remaining part of the line after "void"
                strcat(temp, "double");
                strcat(temp, pos + 4);  // Skip 4 characters ("void")

                strcpy(line, temp);  // Copy the modified line back into line
            }
        }
        // Write the possibly modified line into the appropriate file
        if (funcExists) {
            fputs(line, outputFile);
        } else {
            fputs(line, mainFuncFile);
        }
    }
}

// Function to process function definitions and adjust return type if needed
void processFunctionDefinitions(FILE *funcDefFile, FILE *cFile) {
    char funcDefLine[256];
    bool functionOpen = false;  // Track if a function block is open

    // Process each line in the function definitions file
    while (fgets(funcDefLine, sizeof(funcDefLine), funcDefFile)) {
        bool returnExistsInside = false;  // Local flag for return within a specific function

        // Check if we have reached a new function definition
        if (strstr(funcDefLine, "void") != NULL) {
            // If a function is already open, close it
            if (functionOpen) {
                fprintf(cFile, "}\n");  // Close the previous function
                functionOpen = false;
            }

            // Now handle the new function definition
            char *pos = strstr(funcDefLine, "void");

            // Check if the current function contains a return statement by scanning ahead
            long currentPos = ftell(funcDefFile);  // Save the current position in the file
            char checkLine[256];
            while (fgets(checkLine, sizeof(checkLine), funcDefFile)) {
                if (strstr(checkLine, "return") != NULL) {
                    returnExistsInside = true;
                    break;  // Stop checking if a return is found
                }
                // Break if another function definition starts
                if (strstr(checkLine, "void") != NULL) {
                    break;
                }
            }
            fseek(funcDefFile, currentPos, SEEK_SET);  // Restore the position to the start of the current function definition

            if (pos != NULL) {
                char temp[256];  // Temporary buffer for the modified line
                strncpy(temp, funcDefLine, pos - funcDefLine);  // Copy the part of the line before "void"
                temp[pos - funcDefLine] = '\0';  // Null-terminate the string

                // Replace "void" with "double" if return statement exists, otherwise keep "void"
                if (returnExistsInside) {
                    strcat(temp, "double");
                } else {
                    strcat(temp, "void");
                }

                // Concatenate the remaining part of the line after "void"
                strcat(temp, pos + 4);

                // Write the modified function definition to the C file
                fputs(temp, cFile);

                // Set the functionOpen flag to true, indicating an open function block
                functionOpen = true;
            }
        } else {
            fputs(funcDefLine, cFile);  // Handle lines inside the function body
        }
    }

    // Ensure that any remaining open function block is closed
    if (functionOpen) {
        fprintf(cFile, "}\n");
    }
}

// Function to process the lines from the input file, check syntax, and write to the appropriate file
void processFileLines(FILE *file, FILE *funcDefFile, FILE *upperFuncFile, FILE *mainFuncFile, FILE *cFile, bool *funcExists) {
    char line[256];

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;  // Remove the newline character at the end of each line

        // Check if the syntax is correct
        if (isValidSyntax(line)) {
            // If the line defines a function, write it to the function definitions section
            if (strncmp(line, "function", 8) == 0) {
                *funcExists = true;
                translateToC(funcDefFile, line);  // Write function definitions to funcDefFile
            } 
            // If the line starts with a tab, assume it's part of a function definition
            else if (strncmp(line, "\t", 1) == 0) {
                translateToC(funcDefFile, line);
            } 
            // For other lines, decide where to write based on whether a function has been encountered
            else {
                if (!*funcExists) {
                    translateToC(upperFuncFile, line);  // Write lines before any function definition to upperFuncFile
                } else {
                    translateToC(mainFuncFile, line);  // Write main function lines to mainFuncFile
                }
            }
        } 
        // Handle syntax errors
        else {
            fprintf(stderr, "Syntax error in line: %s\n", line);
            fclose(file);
            fclose(cFile);
            fclose(funcDefFile);
            fclose(mainFuncFile);
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2 || !checkFileExtension(argv[1])) { // Check the file extension is .ml
        fprintf(stderr, "Usage: %s program.ml\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(argv[1], "r");  // Open .ml file
    if (!file) {
        fprintf(stderr, "Error opening file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    int pid = getpid(); // Get process id to set program name
    char cFileName[50];
    snprintf(cFileName, sizeof(cFileName), "ml-%d.c", pid);

    FILE *cFile = fopen(cFileName, "w");  // Open a C file to write the translated C code
    if (!cFile) {
        fprintf(stderr, "Error creating C file\n");
        exit(EXIT_FAILURE);
    }

    fprintf(cFile, "#include <stdio.h>\n\n");  // Write standard headers for the C code

    FILE *mainFuncFile = tmpfile();  // Create a temporary file to store main function body
    FILE *funcDefFile = tmpfile();  // Create a temporary file to store function definitions
    FILE *upperFuncFile = tmpfile();  // Create a temporary file to store lines before function
    bool funcExists = false;  // Checks if lines for the main statement has been read

    // Read and process the .ml file line by line
    processFileLines(file, funcDefFile, upperFuncFile, mainFuncFile, cFile, &funcExists);

    // Move upperFuncFile tmp file to the top of the file
    fseek(upperFuncFile, 0, SEEK_SET);
    processUpperFuncFile(upperFuncFile, cFile, mainFuncFile, funcExists);

    // Move function definitions to the top of the file
    fseek(funcDefFile, 0, SEEK_SET);  // Move to the beginning of the funcDefFile
    processFunctionDefinitions(funcDefFile, cFile);

    // Write the main function header
    fprintf(cFile, "int main() {\n");       

    // Write the main function body
    fseek(mainFuncFile, 0, SEEK_SET);  // Move to the beginning of the mainFuncFile
    writeMainFunctionBody(mainFuncFile, cFile);

    // End the C program's main function
    fprintf(cFile, "return 0;\n}\n");       

    // Close files
    fclose(file);   
    fclose(cFile);
    fclose(funcDefFile);
    fclose(mainFuncFile);

    // Compile the generated C code
    compileCFile(cFileName, pid);

    // Run the compiled program
    runExecutable(cFileName);

    // Remove the compiled and c file
    cleanUpFiles(cFileName, pid);

    exit(EXIT_SUCCESS);
}