#include "model.h"
#include "testrunner.h"
#include "tests.h"

void run_tests() {
    set_cell_value(ROW_2, COL_A, "1.4");
    assert_display_text(ROW_2, COL_A, "1.4");
    set_cell_value(ROW_2, COL_B, "2.9");
    assert_display_text(ROW_2, COL_B, "2.9");
    set_cell_value(ROW_2, COL_C, "=A2+B2+0.4");
    assert_edit_text(ROW_2, COL_C, "=A2+B2+0.4");
    assert_display_text(ROW_2, COL_C, "4.7");
    set_cell_value(ROW_2, COL_B, "3.1");
    assert_display_text(ROW_2, COL_C, "4.9");
}
