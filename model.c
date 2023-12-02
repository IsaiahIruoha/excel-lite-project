#include "model.h"
#include "interface.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>

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
            spreadsheet[row][col].numeric_value = 0.0; // Set the numeric value to zero
            // Initialize dependencies array
            spreadsheet[row][col].dependents = NULL;
            spreadsheet[row][col].num_dependents = 0;
            spreadsheet[row][col].max_dependents = 0;
            spreadsheet[row][col].is_updating = false;
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
    if (!(cell_ref[0] >= 'A' && cell_ref[0] <= 'G') && !(cell_ref[0] >= 'a' && cell_ref[0] <= 'g')) { // Check if the first character is a letter (A-G or a-g)
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

void add_dependent(ROW current_row, COL current_col, ROW row, COL col) {
   CellContent *cell = &spreadsheet[current_row][current_col]; // Create a pointer to the cell I am using

   // Check if there is enough space in the array
   if (cell->num_dependents < cell->max_dependents) {
       // Add the new dependent cell to the array
       cell->dependents[cell->num_dependents].row = row;
       cell->dependents[cell->num_dependents].col = col;
       cell->num_dependents++;
   } else {
       // Resize the array
       size_t new_max_dependents = (cell->max_dependents == 0) ? 1 : cell->max_dependents * 2; // Double the size of the array
       CellDependency *new_dependents = realloc(cell->dependents, new_max_dependents * sizeof(CellDependency));

       if (new_dependents == NULL) {
           // Handle memory allocation failure
           exit(ENOMEM);
       }

       cell->dependents = new_dependents;
       cell->max_dependents = new_max_dependents;

       // Add the new dependent cell to the resized array
       cell->dependents[cell->num_dependents].row = row;
       cell->dependents[cell->num_dependents].col = col;
       cell->num_dependents++; 
   }
}

void update_dependents(ROW row, COL col) { // Function to update dependents when a cell changes
    // Iterate through the dependents of the changed cell
    for (size_t i = 0; i < spreadsheet[row][col].num_dependents; i++) {
        ROW dependent_row = spreadsheet[row][col].dependents[i].row;
        COL dependent_col = spreadsheet[row][col].dependents[i].col;
        // Recalculate and update the value of each dependent cell
        set_cell_value(dependent_row, dependent_col, spreadsheet[dependent_row][dependent_col].text);
    }
}

double eval_formula(char *eqn, ROW row, COL col) { // Function to evaluate a formula
    // Clear existing dependents before adding new ones

    double result = 0, output;  // Initialize the result to 0, and the output stack output variable
    int index = 0; // Initialize an index counter for formula elements
    bool is_negative = false; // Variable to track if the current number is negative
    stack_ptr stack_number = stack_new(); // Create a stack to store numeric values

    if (eqn[0] == '=') { // If the formula starts with '=', skip the '=' character
        ++eqn;
    }

    while (*eqn) { // Iterate through the formula string
        // Check if the character is a negative sign
        if (eqn[0] == '-') {
            is_negative = true;
            ++eqn;    // Move to the next character
        }
        else if (eqn[0] == '+') {
            ++index;  // Increment the index for each addition operation
            ++eqn;    // Move to the next character
        } else if (isalpha(*eqn)) { // Check if the character is an alphabet (indicating a cell reference)
            char uppercase_char = toupper(*eqn); // Convert the character to uppercase

            // Extract the column and row information from the cell reference
            COL current_col = (COL)(uppercase_char - 'A');
            ROW current_row = strtol(eqn += 1, &eqn, 10) - 1; 

            if(current_row == row && current_col == col) { // Check if the current cell is referencing itself
                return 0.0/0.0; // Return NaN if there is a circular dependency
            }

            // I need to check if the current cell is already a dependent of the referenced cell's array of dependents
            for (int i = 0; i < spreadsheet[row][col].num_dependents; i++) {
                if (spreadsheet[row][col].dependents[i].row == current_row &&
                    spreadsheet[row][col].dependents[i].col == current_col) {
                    return 0.0/0.0; // Return NaN if there is a circular dependency
                }
            }

            bool is_already_dependent = false;
            for (int i = 0; i < spreadsheet[current_row][current_col].num_dependents; i++) {
                if (spreadsheet[current_row][current_col].dependents[i].row == row &&
                    spreadsheet[current_row][current_col].dependents[i].col == col) {
                    is_already_dependent = true;
                    break;
                }
            }

            if (!is_already_dependent) {
                add_dependent(current_row, current_col, row, col); // Add the current cell to the list of dependents of the referenced cell
            }

            // Push the numeric value of the referenced cell onto the stack
            stack_push(stack_number, is_negative ? -spreadsheet[current_row][current_col].numeric_value : spreadsheet[current_row][current_col].numeric_value);
            is_negative = false;  // Reset the flag after processing the negative sign
        } else { // Will only  ever be a digit or a decimal point
            // Convert the numeric substring to a double and push it onto the stack
            stack_push(stack_number, is_negative ? -strtod(eqn, &eqn) : strtod(eqn, &eqn));
            is_negative = false; // Reset the flag after processing the negative sign
        }
    }
    while (stack_number->size > 0) { // Pop values from the stack and accumulate the result
        stack_pop(stack_number, &output); 
        result += output; 
    }
    // Free the memory allocated for the stack
    stack_free(stack_number);
    // Return the final result of the formula
    return result;
}

void set_cell_value(ROW row, COL col, char *text) { // Function to set the value of a cellp

    if (spreadsheet[row][col].is_updating) {
            // Handle base case or return
            return;
    }

    // Set the updating flag to true
    spreadsheet[row][col].is_updating = true;

    char *text_copy = strdup(text); // Dynamically allocate memory to store a copy of the text.
    if (text_copy == NULL) { // If malloc fails, exit the program
        exit(ENOMEM);
    }

    strcpy(text_copy, text); // Copy the text into the allocated memory

    // Logic to store the text in the 2D array
    if(text_copy[0] == '=') { // If the text is a formula, store the formula and the numeric value

        if (valid_formula(text_copy)) { // We now know that the formula is valid and can be evaluated
        spreadsheet[row][col].text = text_copy; // Set the formula to the text
        spreadsheet[row][col].numeric_value = eval_formula(text_copy, row, col); // Evaluate the formula and store the numeric value   
            if(isnan(spreadsheet[row][col].numeric_value)) { // Check if the numeric value is NaN
                update_dependents(row, col); 
                update_cell_display(row, col, "CIRC"); // Display CIRC for circular dependency
            } else {
                // Update the display (stringify the formula value)
                int formula_size = snprintf(NULL, 0, "%f", spreadsheet[row][col].numeric_value) + 1;
                char *formula_string = malloc(formula_size);
                snprintf(formula_string, formula_size, "%.1f", spreadsheet[row][col].numeric_value);
                update_dependents(row, col); 
                update_cell_display(row, col, formula_string);
                // Free the allocated memory
                free(formula_string);
            }
        } else {
        spreadsheet[row][col].text = text_copy; // Set the text to hold the invalid formula
        spreadsheet[row][col].numeric_value = 0.0/0.0; // Set the numeric value to 0
        update_dependents(row, col); 
        update_cell_display(row, col, "INVALID"); //display invalid 
        }

    } else { // If the text is a value, store the value and set the formula to NULL
        if(parse_only_double(text_copy, &spreadsheet[row][col].numeric_value)) { // If the text is a number, store the numeric value (This directly stores the number)
            spreadsheet[row][col].text = text_copy;
        } else { // If the text is not a number or a function, store the text with everything else default
            spreadsheet[row][col].text = text_copy; 
            spreadsheet[row][col].numeric_value = 0.0/0.0;
        }
        update_dependents(row, col); 
        update_cell_display(row, col, spreadsheet[row][col].text);
    }
    // Reset the updating flag after updating is complete
    spreadsheet[row][col].is_updating = false;
}

void clear_cell(ROW row, COL col) { // Function to clear the value of a cell
    if (strcmp(spreadsheet[row][col].text, "") != 0) { // If the cell is not empty, free the text
        free(spreadsheet[row][col].text); // Free the text_copy from set_cell_value
    }

    // Logic to clear the cell
    spreadsheet[row][col].text = "";
    spreadsheet[row][col].numeric_value = 0.0;
    // Initialize dependencies array
    update_dependents(row, col);
    update_cell_display(row, col, "");
}

char* get_textual_value(ROW row, COL col) {
    return strdup(spreadsheet[row][col].text); // Return a copy of the textual value (memory management is handled by the caller in the interface)
}