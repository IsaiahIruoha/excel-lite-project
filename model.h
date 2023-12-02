// I, Isaiah Iruoha, attest that the following code represents my own work.

// Functionality: This file contains the declarations for the functions used in model.c

// External Resources: Prettier (Code Formatting)

#ifndef ASSIGNMENT_MODEL_H
#define ASSIGNMENT_MODEL_H

#include "defs.h"
#include <stdbool.h>
#include <stddef.h>

// Struct to represent a node in the stack
struct stack_node
{
    double value;
    struct stack_node *next;
};

// Struct to represent a stack
struct stack
{
    struct stack_node *head;
    size_t size; // The size of the stack
};

typedef struct stack *stack_ptr; // A pointer to a stack

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

typedef struct
{
    // Struct to represent a cell's dependencies
    ROW row;
    COL col;
} CellDependency;

typedef struct
{
    // Data structure representing each cell in the grid (7x9).
    char *text;
    double numeric_value;
    bool is_updating;           // Flag to prevent circular dependencies
    CellDependency *dependents; // Array of cells that depend on this cell
    size_t num_dependents;      // Number of cells that depend on this cell
    size_t max_dependents;      // Maximum number of cells that can depend on this cell
} CellContent;

CellContent spreadsheet[NUM_ROWS][NUM_COLS]; // Static global 2D array of CellContent structs.

// Adds a dependent to a cell
void add_dependent(ROW current_row, COL current_col, ROW row, COL col);

// Updates all cells that depend on a cell
void update_dependents(ROW row, COL col);

// Initializes the data structure.
// This is called once, at program start.
void model_init();

// Sets the value of a cell based on user input.
// The string referred to by 'text' is now owned by this function and/or the
// cell contents data structure; it is its responsibility to ensure it is freed
// once it is no longer needed.
void set_cell_value(ROW row, COL col, char *text);

// Clears the value of a cell.
void clear_cell(ROW row, COL col);

// Gets a textual representation of the value of a cell, for editing.
// The returned string must have been allocated using 'malloc' and is now owned
// by the interface. The cell contents data structure must not modify it or
// retain any reference to it after the function returns.
char *get_textual_value(ROW row, COL col);

#endif // ASSIGNMENT_MODEL_H