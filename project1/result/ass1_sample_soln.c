/* Program to apply "queries" to the lines in a text file,
   where a query is a list of prefixes of words, and a word in an
   input line is counted towards an overall score for that line if
   any of the query terms is a prefix of or equal to that word after
   case-folding.
   Queries are input via the commandline; the text to be scanned
   arrives via stdin.

   Written by Alistair Moffat, August 2017.
*/ 

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

/* maximum length of any input line, in characters */
#define MAX_LINE_LEN 1000

/* number of "top" lines to be retained and displayed at end of input */
#define MAX_LINES_OUT 5

/* return values from the line-reading function */
#define LINE_OK 0
#define LINE_DONE (-1)

/* turn on for additional debugging output */
#define DEBUG 0

/* turn off to see *only* the top-scoring output lines, and not all
   the "stage" output as well */
#define VOLUBLE 1

/* separator strings for output lines */
#define SEP_STR "---"
#define LNG_STR "------------------------------------------------"

/* constants used for Stage 3 scoring, as specified in handout */
#define CONST1 1.0
#define CONST2 8.5

/* not available in every instance of math.h */
#define LOG2(x) (log(x)/log(2.0))


/* type definitions --------------------------------------------------*/

/* structure to retain a single input line and also an array of pointers
   to the starts of the wrds in that line */
typedef struct {
	int linenum;
	char chars[MAX_LINE_LEN+1]; /* original line */
	char charw[MAX_LINE_LEN+1]; /* and then split into words */
	int nchrs;
	char *words[MAX_LINE_LEN];  /* pointers to starts of words in charw */
	int nwrds;
	double score;
} oneline_t;

/* each query is a NULL-sentinelled array of pointers to strings, and in
   reality is just argv being pased around into functions */
typedef char **query_t;

/* function prototypes -----------------------------------------------*/

int    mygetchar();
int    getoneline(oneline_t *line);
void   do_stage1(query_t query);
int    has_bad_chars(char *s);
void   do_stage2(oneline_t *line);
void   do_stage3(oneline_t *line, query_t query);
void   do_stage4(oneline_t bestlines[], int nbestlines);
void   consider_keeping(oneline_t *line, oneline_t bestlines[],
		int *nbestlines);

/* where it all happens ----------------------------------------------*/

int
main(int argc, char *argv[]) {
	oneline_t line;
	oneline_t bestlines[MAX_LINES_OUT];
	int nbestlines=0;
	query_t query;
	
	/* the input query starts at argv[1] */
	query = argv+1;
	/* use will be made of the fact that query[argc] is set to NULL
	   as a sentinel at the end of the set of input prefixes, hence
	   no need for a buddy variable */

	/* initial output, including checking the query */
	do_stage1(query);

	/* each iteration here processes one line */
	while (getoneline(&line) != LINE_DONE) {
		do_stage2(&line);
		do_stage3(&line, query);
		/* decide whether to retain this line */
		consider_keeping(&line, bestlines, &nbestlines);
	}

	/* generate the final "top lines" output section */
	do_stage4(bestlines, nbestlines);

	/* ta daa! */
	return EXIT_SUCCESS;
}

/* some of the little helpers ----------------------------------------*/

/* get a character from stdin and bypass any CR's that might cause
   confusion with data generated in the PC world */
int
mygetchar() {
	int c;
	while ((c=getchar())=='\r') {
	}
	return c;
}

/* some of the bigger helpers ----------------------------------------*/

/* read one line from input in to the struct that stores lines.
   Lines are counted using a static variable, and limited in length.
   Only part of the input argument oneline structure is initialized
   in this function.
 */
int
getoneline(oneline_t *line) {
	static int linenum=1;
	int c;
	int i;
	int inaword;
	while ((c=mygetchar()) == '\n') {
		/* completely bypass empty lines */
		linenum++;
	}
	/* is there even a line? */
	if (c==EOF) {
		return LINE_DONE;
	}
	/* yes there is, now get the rest of it */
	line->linenum = linenum;
	line->nchrs = 0;
	line->chars[line->nchrs++] = c;
	while ((line->nchrs<MAX_LINE_LEN) && (c=mygetchar()) != '\n') {
		line->chars[line->nchrs++] = c;
	}
	if (c=='\n') {
		linenum++;
	}
	/* close off the line properly */
	line->chars[line->nchrs] = '\0';
	/* and duplicate into the second array, this one will get cut up
	   into words */
	strcpy(line->charw, line->chars);
	/* now process the line into a sequence of words, each composed
	   of max length lowercase/numeric characters */
	line->nwrds = 0;
	inaword = 0;
	for (i=0; i<line->nchrs; i++) {
		if (isupper(line->charw[i])) {
			line->charw[i] = tolower(line->charw[i]);
		}
		if (islower(line->charw[i]) || isdigit(line->charw[i])) {
			if (!inaword) {
				/* start of a new word */
				line->words[line->nwrds++] = (line->charw)+i;
				inaword = 1;
			} 
		} else {
			/* make sure not part of a word */
			line->charw[i] = '\0';
			inaword = 0;
		}
	}
	return LINE_OK;
}

/* generate stage 0 output lines
 */
void
do_stage1(query_t query) {
	int i;
	int somebad=0;
	/* query array is NULL_sentinelled */
	if (query[0]==NULL) {
		printf("S1: No query specified,"
			" must provide at least one word\n");
		exit(EXIT_FAILURE);
	}
#if VOLUBLE
	printf("S1: query =");
	for (i=0; query[i]!=NULL; i++) {
		printf(" %s", query[i]);
	}
	printf("\n");
#endif
	for (i=0; query[i]!=NULL; i++) {
		if (has_bad_chars(query[i])) {
			printf("S1 %s: invalid character(s)\n", query[i]);
			somebad = 1;
		}
	}
	if (somebad) {
		exit(EXIT_FAILURE);
	}
	return;
}

/* test the string supplied as argument to see if there are any non-digit
   non-lowercase alphabetic characters
*/
int
has_bad_chars(char *s) {
	while (*s) {
		if (islower(*s) || isdigit(*s)) {
			s++;
		} else {
			return 1;
		}
	}
	return 0;
}


/* generate stage 2 output line for each line read from stdin
 */
void
do_stage2(oneline_t *line) {
#if VOLUBLE
	printf("%s\n", SEP_STR);
	printf("%s\n", line->chars);
	printf("S2: line = %d, bytes = %d, words = %d\n",
		line->linenum, line->nchrs, line->nwrds);
#endif
	return;
}

/* compute the stage 3 overlap formula and add it to the struct
 */
void
do_stage3(oneline_t *line, query_t query) {
	double score = 0.0;
	int q;		/* count through query terms */
	int w;		/* count through words in the line */
	int ft;		/* frequency of that query term in the line */
	int len;	/* length of that query term */
	for (q=0; query[q]!=NULL; q++) {
		ft = 0;
		len = strlen(query[q]);
		/* now check each work in the parsed input line */
		for (w=0; w<line->nwrds; w++) {
			if (strncmp(query[q], line->words[w], len)==0) {
				/* prefix match found */
				ft++;
			}
		}
#if DEBUG
		printf("    %-10s appears %d times\n", query[q], ft);
#endif
		score += LOG2(CONST1+ft);
	}
	line->score = score/LOG2(CONST2+line->nwrds);
#if VOLUBLE
	printf("S3: line = %d, score = %.3f\n",
			line->linenum, line->score);
#endif
}

/* decide whether to retain this line as part of the top lines array
 */
void
consider_keeping(oneline_t *line, oneline_t bestlines[], int *nbestlines) {
	int j;
	oneline_t tmp;
	if (line->score==0.0) {
		/* nothing happening at all, definitely not a keeper */
		return;
	}
	if (*nbestlines<MAX_LINES_OUT) {
		/* still unused "top" spots, automatic add */
		bestlines[(*nbestlines)++] = *line;
	} else {
		/* bestlines already full, and we know that
		   last one is the smallest one */
		if (line->score>bestlines[*nbestlines-1].score) {
			/* yes, makes it in */
			bestlines[*nbestlines-1] = *line;
		}
	}
	/* and now check the ordering by sifting backwards, using
	   insertion sort's sifting mechanism */
	for (j=*nbestlines-1; j>0; j--) {
		if (bestlines[j-1].score<bestlines[j].score) {
			/* swap them */
			tmp = bestlines[j-1];
			bestlines[j-1] = bestlines[j];
			bestlines[j] = tmp;
		}
	}
	return;
}

/* generate the required Stage 4 output */
void
do_stage4(oneline_t bestlines[], int nbestlines) {
	int i;
	oneline_t *line;

#if VOLUBLE
	printf("%s\n", LNG_STR);
#endif
	/* print out each of the nbestlines that have been built up */
	for (i=0; i<nbestlines; i++) {
		line = bestlines+i;
#if VOLUBLE
		printf("S4: ");
#else
		printf("%s ", SEP_STR);
#endif
		printf("line = %d, score = %.3f\n%s\n",
			line->linenum, line->score, line->chars);
#if VOLUBLE
		printf("%s\n", SEP_STR);
#endif
	}
	return;
}
