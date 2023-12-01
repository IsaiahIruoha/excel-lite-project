#include "model.h"
#include "interface.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

stack_ptr stack_new() { // This function creates a new stack and returns a pointer to it
    stack_ptr s = malloc(sizeof(struct stack));
    s->head = NULL;
    s->size = 0;
    return s;
}

void stack_free(stack_ptr s) { // This function frees the stack and all of its nodes
    while (s->head != NULL){ // While the stack is not empty
        struct stack_node* temp = s->head; // Create a temporary pointer to the top of the stack
        s->head = s->head->next; // "Metaphorically remove the top plate"
        if (temp->value) { // Check if the previous top node had any values
            //free(temp->value); // If it did free the values (TAKEN OUT SINCE FREEING A SINGLE CHARACTER IS NOT NEEDED)
        }
        free(temp); // Free the entire node
    }
    free(s);
}

void stack_push(stack_ptr s, double c) { // This function is the same as stack_pop except it does not remove the top node
    struct stack_node* new_node = malloc(sizeof(struct stack_node)); // Allocate memory for the new node
    new_node->value = c; // Set the value of the new node
    new_node->next = s->head; // Set the new node to point to the current top
    s->head = new_node; // Set the current to be the new node
    s->size++; // Increment the size of the stack
}

bool stack_pop(stack_ptr s, double *out) { // This function is the same as stack_peek except it removes the top node
    if (s->head == NULL) { // Check if the stack exists
        return false; // Return false if it does not exist
    }
    struct stack_node* temp = s->head; // Create a temporary pointer to a stack_node storing the memory address of the current head
    *out = s->head->value; // Dereference the character storage location, assign it to store the value at the head node
    s->head = s->head->next; // Change the head node to the next node
    free(temp); // Free the previous head node
    s->size--; // Decrement the size of the stack
    return true; // Return true to show that a pop has occured 
}

void model_init() { // Initialize each cell with default values
    for (int row = ROW_1; row < NUM_ROWS; row++) {
        for (int col = COL_A; col < NUM_COLS; col++) {
            spreadsheet[row][col].text = "";
            spreadsheet[row][col].numeric_value = 0.0;
        }
    }
}

bool parse_only_double(const char *input, double *out) { // This function is used to check if the input is completely a number or not
    // Try to parse a double at the current position
    char *endptr;
    double result = strtod(input, &endptr);

    // If no characters were used, then the input was invalid
    if (input == endptr) { // endptr is the same as input if no characters were used
        return false;
    } else {
        *out = result;
        return true;
    }
}

bool is_valid_cell(const char *cell_ref) { // Function to check if a given cell reference is valid
    size_t cell_ref_length = strlen(cell_ref);

    if (cell_ref_length < 2) { // Check if the length is within bounds
        return false;
    }

    if (!isalpha(cell_ref[0]) || (cell_ref[0] < 'A' || (cell_ref[0] > 'G' && cell_ref[0] < 'a') || cell_ref[0] > 'g')) { // Check if the first character is a letter (A-G or a-g)
        return false;
    }

    if (!isdigit(cell_ref[1]) || cell_ref[1] == '0') { // Check if the second character is a digit (1-9)
        return false;
    }

    if (cell_ref[2] == '\0' || cell_ref[2] == '+') { // Check if the third character is the null terminator
        return true;
    }
    
    return false;  // All checks passed, valid cell reference
}

bool valid_formula(const char *input) { // Function to check if the characters to the left and right of the decimal are digits
    size_t formula_length = strlen(input);

    if (formula_length == 0 || input[0] != '=') { // Check if the formula is empty or starts with an equals sign
        return false;
    }

    for (size_t i = 1; i < formula_length; i++) { // Start checking from the second character
        char current_char = input[i];
        
        // Check if the current character is a digit, a decimal point, a '+', a '-', or a valid cell reference
        if (!isdigit(current_char) && current_char != '.' && current_char != '+' && current_char != '-' && !(i + 2 <= formula_length && is_valid_cell(input + i))) {
            return false;  // Not a digit, a decimal point, '+', '-', or a valid cell reference
        }

        if (current_char == '.') { // Check if the decimal point is preceded and followed by digits
            if (formula_length < 4) { // This include the equals sign, decimal and only 1 digit
                return false; // Not enough characters to the left and right of the decimal point
            }
        
        // Check the character to the left of the decimal point
        if (i > 1 && !isdigit(input[i - 1])) {
            return false;  // Not a digit to the left of the decimal point
        }

        // Check the character to the right of the decimal point
        if (i < formula_length - 1 && !isdigit(input[i + 1])) {
            return false;  // Not a digit to the right of the decimal point
        }
        }   

        if (current_char == '-') { // Check if a negative sign
            if (formula_length < 3) { // This include the equals sign, minus sign and no digits
                return false; // Not enough characters
            }
        
            // Check the character to the left of the minus sign
            if (i > 1 && input[i-1] != '+' && !isdigit(input[i - 1]) && !is_valid_cell(input + i - 2)) {
                return false;  // Not a digit or a valid cell reference to the left of the minus sign
            }

            // Check the character to the right of the minus sign
            if (i < formula_length - 1 && !isdigit(input[i + 1]) && !is_valid_cell(input + i + 1)) {
                return false;  // Not a digit or a valid cell reference to the right of the minus sign
            }
        } 
    }
    return true;  // All checks passed
}

double eval_formula(char *eqn) { // Function to evaluate a formula
    double result = 0;  // Initialize the result to 0
    int index = 0;      // Initialize an index counter for formula elements
    double output;      // Variable to store popped values from the stack
    stack_ptr stack_number = stack_new(); // Create a stack to store numeric values

    // If the formula starts with '=', skip the '=' character
    if (eqn[0] == '=') {
        ++eqn;
    }

    // Variable to track if the current number is negative
    bool is_negative = false;

    // Iterate through the formula string
    while (*eqn) {
        // Check if the character is a negative sign
        if (eqn[0] == '-') {
            is_negative = true;
            ++eqn;    // Move to the next character
        }
        else if (eqn[0] == '+') {
            ++index;  // Increment the index for each addition operation
            ++eqn;    // Move to the next character
        }
        // Check if the character is an alphabet (indicating a cell reference)
        else if (isalpha(*eqn)) {
            // If the next character is not a digit, return false
            if (!isdigit(eqn[1])) {
                return false;
            }
            // Extract the column and row information from the cell reference
            COL current_col = (COL)(eqn[0] - 'A');
            ROW current_row = strtol(eqn += 1, &eqn, 10) - 1; 
            // Check if the cell reference is within the valid range
            if (current_col * current_row >= NUM_COLS * NUM_ROWS || current_col * current_row < 0) {
                return false; 
            }
            // Push the numeric value of the referenced cell onto the stack
            stack_push(stack_number, is_negative ? -spreadsheet[current_row][current_col].numeric_value : spreadsheet[current_row][current_col].numeric_value);
            is_negative = false;  // Reset the flag after processing the negative sign
        }
        // Check if the character is a digit or a decimal point
        else if (isdigit(*eqn) || *eqn == '.') {
            // Convert the numeric substring to a double and push it onto the stack
            stack_push(stack_number, is_negative ? -strtod(eqn, &eqn) : strtod(eqn, &eqn));
            is_negative = false;  // Reset the flag after processing the negative sign
        } else {
            // Free the stack and return false for unsupported characters
            stack_free(stack_number);
            return false; 
        }
    }

    // Check if the number of elements on the stack matches the expected count
    if (stack_number->size != index + 1) {
        stack_free(stack_number);
        return false; 
    }

    // Pop values from the stack and accumulate the result
    while (stack_number->size > 0) {
        stack_pop(stack_number, &output); 
        result += output; 
    }

    // Free the memory allocated for the stack
    stack_free(stack_number);

    // Return the final result of the formula
    return result;
}

void set_cell_value(ROW row, COL col, char *text) { // Function to set the value of a cell

    char *text_copy = strdup(text); // Dynamically allocate memory to store a copy of the text.
    if (text_copy == NULL) { // If malloc fails, exit the program
        exit(ENOMEM);
    }

    // Free existing memory only if the cell is not empty
        if (spreadsheet[row][col].text && strcmp(spreadsheet[row][col].text, "") != 0) {
            free(spreadsheet[row][col].text);
        }

    strcpy(text_copy, text); // Copy the text into the allocated memory

    // Logic to store the text in the 2D array
    if(text_copy[0] == '=') { // If the text is a formula, store the formula and the numeric value

        if (valid_formula(text_copy)) { // We now know that the formula is valid and can be evaluated
        spreadsheet[row][col].text = text_copy; // Set the formula to the text
        spreadsheet[row][col].numeric_value = eval_formula(spreadsheet[row][col].text); // Evaluate the formula and store the numeric value   

        // Update the display (stringify the formula value)
        int formula_size = snprintf(NULL, 0, "%f", spreadsheet[row][col].numeric_value) + 1;
        char *formula_string = malloc(formula_size);
        snprintf(formula_string, formula_size, "%.1f", spreadsheet[row][col].numeric_value);
        update_cell_display(row, col, formula_string);
        } else {
        spreadsheet[row][col].text = text_copy; // Set the text to hold the invalid formula
        spreadsheet[row][col].numeric_value = 0.0; // Set the numeric value to 0
        update_cell_display(row, col, "INVALID"); //display invalid 
        }

    } else { // If the text is a value, store the value and set the formula to NULL
        if(parse_only_double(text_copy, &spreadsheet[row][col].numeric_value)) { // If the text is a number, store the numeric value (This directly stores the number)
            spreadsheet[row][col].text = text_copy; 
        } else { // If the text is not a number or a function, store the text with everything else default
            spreadsheet[row][col].text = text_copy; 
            spreadsheet[row][col].numeric_value = 0.0;
        }
        update_cell_display(row, col, spreadsheet[row][col].text);
    }
}

void clear_cell(ROW row, COL col) { // Function to clear the value of a cell
    if (strcmp(spreadsheet[row][col].text, "") == 0) { // If the cell is not empty, free the text
        free(spreadsheet[row][col].text); // Free the text_copy from set_cell_value
    }

    // Logic to clear the cell
    spreadsheet[row][col].text = "";
    spreadsheet[row][col].numeric_value = 0.0;

    // In the future I will have to update the dependancies of any cells that depend on this cell
    // That could be done by iterating through a dependacy array the cleared cell will have, then updating each of the cells in the array
    // I could use a remove dependancy function to do this

    // This just clears the display without updating any data structure. You will need to change this.
    update_cell_display(row, col, "");
}

char* get_textual_value(ROW row, COL col) { // Function to get the textual value of a cell
    // Allocate memory for the textual representation
    size_t text_length = strlen(spreadsheet[row][col].text); // Get the length of the text
    char *textual_value; // Declare the textual_value variable to be accessed anywhere in the function
     // If the cell does not contain a formula, return the text
        textual_value = malloc(text_length + 1); // Allocate memory for the text
        if (textual_value == NULL) { // If malloc fails, exit the program
        // Handle allocation failure
        exit(ENOMEM);
        }
        strcpy(textual_value, spreadsheet[row][col].text); // Copy the text into the allocated memory
    return textual_value;
}