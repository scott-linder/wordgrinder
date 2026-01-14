/* © 2008 David Given.
 * WordGrinder is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include <stdlib.h>
#include <locale.h>
#include "globals.h"

#include "script_table.h"

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "C.UTF-8");

    script_init();
    screen_init((const char**)argv);
    word_init();
    utils_init();
    filesystem_init();
    zip_init();
    clipboard_init();
    cmark_init();

    script_load_from_table(script_table);
    script_run((const char**)argv);

    return 0;
}
