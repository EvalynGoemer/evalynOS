#define TERM_WIDTH 80
#define TERM_HEIGHT 24
extern char terminal[TERM_HEIGHT][TERM_WIDTH];
extern int term_updated;
extern int printf(const char *fmt, ...);
extern void cleark();
