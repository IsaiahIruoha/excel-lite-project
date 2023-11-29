#include "model.h"
#include "interface.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

stack_ptr stack_new() {
    stack_ptr s = malloc(sizeof(struct stack));
    s->head = NULL;
    s->size = 0;
    return s;
}

void stack_free(stack_ptr s) {
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

void stack_push(stack_ptr s, char c) {
    struct stack_node* new_node = malloc(sizeof(struct stack_node)); // Allocate memory for the new node
    new_node->value = c; // Set the value of the new node
    new_node->next = s->head; // Set the new node to point to the current top
    s->head = new_node; // Set the current to be the new node
    s->size++; // Increment the size of the stack
}

bool stack_pop(stack_ptr s, char *out) {
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
            spreadsheet[row][col].formula_stack = NULL;
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

void set_cell_value(ROW row, COL col, char *text) {
    if (strcmp(text, "") == 0) { // If the user enters nothing, clear the cell
        clear_cell(row, col);
        return; 
    }

    // Dynamically allocate memory to store a copy of the text.
    char *text_copy = malloc(strlen(text) + 1);

    if (text_copy == NULL) { // If malloc fails, exit the program
        exit(ENOMEM);
    }

    strcpy(text_copy, text); // Copy the text into the allocated memory

    // Logic to store the text in the 2D array
    if(text_copy[0] == '=') { // If the text is a formula, store the formula and the numeric value (NEED TO ADD FUNCTIONALITY FOR INVALID FORMULAS, POSSIBLY A "INVALID" in the box or something)
        spreadsheet[row][col].formula_stack = stack_new();  // Initialize the formula stack (This dynamically allocates memory for the stack)
        for (size_t i = 0; i < strlen(text_copy); i++) { // Iterate through the text (the first character is '=')
            stack_push(spreadsheet[row][col].formula_stack, text_copy[i]);  // Push each character onto the formula stack
        }
        spreadsheet[row][col].numeric_value = 0.0; //Will not always store 0.0, this is TEMPORARY till evaluation is implemented
        spreadsheet[row][col].text = "FUNC"; // Set the text to FUNC to indicate that the cell contains a formula TEMPORARY
    } else { // If the text is a value, store the value and set the formula to NULL
        spreadsheet[row][col].formula_stack = NULL;
        if(parse_only_double(text_copy, &spreadsheet[row][col].numeric_value)) { // If the text is a number, store the numeric value (This directly stores the number)
            spreadsheet[row][col].text = text_copy; // Set the text to also display the number (in text form)
        } else { // If the text is not a number or a function, store the text with everything else default
            spreadsheet[row][col].text = text_copy;
            spreadsheet[row][col].numeric_value = 0.0;
        }
    }
    // Update the display
    update_cell_display(row, col, spreadsheet[row][col].text);

    // Free the allocated memory
    free(text); // Free the original text which is owned by the function 
}

void clear_cell(ROW row, COL col) {
    free(spreadsheet[row][col].text); // Free the text_copy from set_cell_value
    stack_free(spreadsheet[row][col].formula_stack); // Free the formula stack
    free(spreadsheet[row][col].formula_stack); // Free the pointer to the formula stack

    // Logic to clear the cell
    spreadsheet[row][col].text = "";
    spreadsheet[row][col].numeric_value = 0.0;
    spreadsheet[row][col].formula_stack = NULL;

    // In the future I will have to update the dependancies of any cells that depend on this cell
    // That could be done by iterating through a dependacy array the cleared cell will have, then updating each of the cells in the array
    // I could use a remove dependancy function to do this

    // This just clears the display without updating any data structure. You will need to change this.
    update_cell_display(row, col, "");
}

char *get_textual_value(ROW row, COL col) {
    // Allocate memory for the textual representation
    size_t text_length = strlen(spreadsheet[row][col].text); // Get the length of the text
    char *textual_value; // Declare the textual_value variable to be accessed anywhere in the function
    if(spreadsheet[row][col].formula_stack != NULL) { // If the cell contains a formula, return the formula
        size_t formula_length = spreadsheet[row][col].formula_stack->size; // Get the ammount of characers in the formula
        textual_value = malloc(formula_length + 1); // Allocate memory for the formula
        if (textual_value == NULL) { // If malloc fails, exit the program
        // Handle allocation failure
        exit(ENOMEM);
        }

    // Use a buffer array instead of a temporary stack
    char buffer[formula_length];
    size_t buffer_index = 0;

    // Copy characters from the original formula_stack to both textual_value and the buffer
    for (size_t index = 0; index < formula_length; index++) {
        char temp_char;
        if (stack_pop(spreadsheet[row][col].formula_stack, &temp_char)) {
            buffer[buffer_index++] = temp_char;
            textual_value[formula_length - index- 1] = temp_char; // This deals with the reverse that happens whne popping from the stack (for the display)
        }
    }

    // Push characters back to the original formula_stack from the buffer
    for (size_t i = buffer_index; i > 0; i--) {
        stack_push(spreadsheet[row][col].formula_stack, buffer[i - 1]); // This deals with the reverse that happens whne popping from the stack (for the storage) 
    }

    } else { // If the cell does not contain a formula, return the text
        textual_value = malloc(text_length + 1); // Allocate memory for the text
        if (textual_value == NULL) { // If malloc fails, exit the program
        // Handle allocation failure
        exit(ENOMEM);
        }
        strcpy(textual_value, spreadsheet[row][col].text); // Copy the text into the allocated memory
    }
    return textual_value;
}