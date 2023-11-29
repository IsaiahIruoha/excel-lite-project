#include "model.h"
#include "interface.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

// A data structure representing each cell in the grid (10x10).
typedef struct {
    char *text;
    double numeric_value;
    char *formula;
} CellContent;

// Static global 2D array of CellContent structs.
CellContent spreadsheet[NUM_ROWS][NUM_COLS];

void model_init() {
    // Initialize each cell with default values
    for (int row = ROW_1; row < NUM_ROWS; row++) {
        for (int col = COL_A; col < NUM_COLS; col++) {
            spreadsheet[row][col].text = "test";
            spreadsheet[row][col].numeric_value = 0.0;
            spreadsheet[row][col].formula = NULL;
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
    if (text == NULL) { // If the user enters nothing, clear the cell
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
    if(text_copy[0] == '=') { // If the text is a formula, store the formula and the numeric value
        spreadsheet[row][col].formula = text_copy; // Store the formula, unparsed (DEALING WITH THE FUNCTION WILL BE LEFT IN A LATER STAGE) 
        spreadsheet[row][col].numeric_value = 0.0; //Will not always store 0.0, this is TEMPORARY till evaluation is implemented
        spreadsheet[row][col].text = "FUNC"; // Set the text to FUNC to indicate that the cell contains a formula TEMPORARY
    } else { // If the text is a value, store the value and set the formula to NULL
        spreadsheet[row][col].formula = NULL;
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
    // TODO: implement this.

    // To do later, but must free(spreadsheet[row][col].text) (this will free the text_copy in set_cell_value)
    // and free(spreadsheet[row][col].formula) (this will free the formula in set_cell_value)
    
    // This just clears the display without updating any data structure. You will need to change this.
    update_cell_display(row, col, "");
}

char *get_textual_value(ROW row, COL col) {
    // TODO: implement this.
    return NULL;
}
