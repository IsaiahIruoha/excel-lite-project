#ifndef ASSIGNMENT_MODEL_H
#define ASSIGNMENT_MODEL_H

#include "defs.h"
#include <stdbool.h>
#include <stddef.h>

struct stack_node {
    double value; 
    struct stack_node* next; 
};

struct stack {
    // Null if the stack is empty.
    struct stack_node* head;
    size_t size; // The size of the stack
};

typedef struct stack* stack_ptr;

// Creates an empty stack.
// The stack pointer should be passed to stack_free when it is no longer needed,
// to clean up its allocations.
stack_ptr stack_new();

// Destroys a stack, freeing all allocated memory.
void stack_free(stack_ptr s);

// Adds an entry to a stack.
void stack_push(stack_ptr s, double c);

// Removes an entry from a stack, storing the removed value in '*out'.
// Returns false if the stack is empty.
bool stack_pop(stack_ptr s, double *out);

// A data structure representing each cell in the grid (10x10).
typedef struct {
    char *text;
    double numeric_value;
} CellContent;

// Static global 2D array of CellContent structs.
CellContent spreadsheet[NUM_ROWS][NUM_COLS];

// Initializes the data structure.
//
// This is called once, at program start.
void model_init();

// Sets the value of a cell based on user input.
//
// The string referred to by 'text' is now owned by this function and/or the
// cell contents data structure; it is its responsibility to ensure it is freed
// once it is no longer needed.
void set_cell_value(ROW row, COL col, char *text);

// Clears the value of a cell.
void clear_cell(ROW row, COL col);

// Gets a textual representation of the value of a cell, for editing.
//
// The returned string must have been allocated using 'malloc' and is now owned
// by the interface. The cell contents data structure must not modify it or
// retain any reference to it after the function returns.
char *get_textual_value(ROW row, COL col);

#endif //ASSIGNMENT_MODEL_H
