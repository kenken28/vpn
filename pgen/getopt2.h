/*
 * POSIX getopt for Windows
 *
 * AT&T Public License
 *
 * Code given out at the 1985 UNIFORUM conference in Dallas.
 */
#ifndef _GETOPT2_H_
#define _GETOPT2_H_

extern int optind2;
extern int optopt2;
extern char *optarg2;
extern char getopt2(int argc, char **argv, char *opts);

#endif  /* _GETOPT2_H_ */
