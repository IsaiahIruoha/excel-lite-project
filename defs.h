#ifndef ASSIGNMENT_DEFS_H
#define ASSIGNMENT_DEFS_H

#define NUM_ROWS 9
#define NUM_COLS 7
#define MAX_CELL_REF_LENGTH 3

// Rows of the spreadsheet.
// NOTE: enums are 0-based, so the constant 'ROW_1' has the numerical value 0.
typedef enum {
    ROW_1,
    ROW_2,
    ROW_3,
    ROW_4,
    ROW_5,
    ROW_6,
    ROW_7,
    ROW_8,
    ROW_9,
    //ROW_10,
} ROW;

typedef enum {
    COL_A,
    COL_B,
    COL_C,
    COL_D,
    COL_E,
    COL_F,
    COL_G,
} COL;

#endif //ASSIGNMENT_DEFS_H
