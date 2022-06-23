#ifndef ANSI_COLOR_LABELS
#define ANSI_COLOR_LABLES

#include <cstdio>

/* COLORS
 * BLK - BLACK
 * RED - RED
 * GRN - GREEN
 * YLW - YELLOW
 * BLU - BLUE
 * MAG - MAGENTA
 * CYN - CYAN
 * WHT - WHITE
 *
 * FORMAT
 * FOR TEXT FORMATING:	prefix B for bold text, U for underline text
 *			suffix B for background color
 */

/* Text colors */
#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YLW "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"

/* Bold text colors */
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYLW "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define BWHT "\e[1;37m"

/* Underline text colors */
#define UBLK "\e[4;30m"
#define URED "\e[4;31m"
#define UGRN "\e[4;32m"
#define UYLW "\e[4;33m"
#define UBLU "\e[4;34m"
#define UMAG "\e[4;35m"
#define UCYN "\e[4;36m"
#define UWHT "\e[4;37m"

/* Text background colors */
#define BLKB "\e[40m"
#define REDB "\e[41m"
#define GRNB "\e[42m"
#define YLWB "\e[43m"
#define BLUB "\e[44m"
#define MAGB "\e[45m"
#define CYNB "\e[46m"
#define WHTB "\e[47m"

/* Reset to default colors */
#define RESET "\e[0m"

/* functions to use text color */
void black();
void red();
void green();
void yellow();
void blue();
void magenta();
void cyan();
void white();

/* functions to use bold text color */
void bold_black();
void bold_red();
void bold_green();
void bold_yellow();
void bold_blue();
void bold_magenta();
void bold_cyan();
void bold_white();

/* functions to use underline text color */
void underlined_black();
void underlined_red();
void underlined_green();
void underlined_yellow();
void underlined_blue();
void underlined_magenta();
void underlined_cyan();
void underlined_white();

/* functions to use text background color */
void black_background();
void red_background();
void green_background();
void yellow_background();
void blue_background();
void magenta_background();
void cyan_background();
void white_background();

/* function to reset ansi color */
void reset_color();

#endif /* ANSI_COLOR_LABLES */
