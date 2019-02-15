/*comp10002 assignment1 by Xulin Yang 904904, September 2017*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define LOG2(x) log(x)/log(2.0)

#define MAX_CHARACTER 1001 /*maximum char in one line*/
#define MAX_LINE 5 /*maximum line can be stored*/
#define 

#define VALID 1
#define INVALID 0
#define EPSILON 1e-3

#define SPACE ' '
#define SINGLE_QUOTE_MARK '\''
#define HYPHEN '-'
#define NO_QUERY "No query specified, must provide at least one word"
#define INVALID_CHARACTER ": invalid character(s) in query"

#define END_STAGE "---"
#define STAGE_ONE "S1: "
#define STAGE_TWO "S2: "
#define STAGE_THREE "S3: "
#define STAGE_FOUR "S4: "

#define NUMERATOR_CONSTANT 1.0             /****************************/
#define DENOMINATOR_CONSTANT 8.5
#define ZERO_DECIMAL 0.0

typedef struct proc_line proc_line_t;
typedef struct stored_line stored_line_t;

struct stored_line {
	char sentence[MAX_CHARACTER];
	double score;
	int line;
};

struct proc_line {
	int bytes;
	int words;
	stored_line_t details;
};

void check_argc(int argc);
void check_query(int argc, char *argv[]);
int check_query_string(char *query);
void print_query(int argc, char *argv[]);
void initialize_stored_lines(stored_line_t stored_line[]);

int mygetchar();
void get_sentence(stored_line_t stored_line[], char *query[], int query_num);

void find_match_word(char *query[], int q_num, char *word, int q_times[]);
double cal_line_score(char *query[], int q_num, int w_num, int q_times[]);
void print_proc_line(proc_line_t proc_line);

void ranking(stored_line_t stored_lines[], proc_line_t proc_line);
void move_backward(stored_line_t stored_line[], int start_index);
void print_rank(stored_line_t stored_lines[]);

int main(int argc, char *argv[]) {
	/*S1*/
	check_argc(argc);
	
	print_query(argc, argv);
	
	check_query(argc, argv);

	/*S2*//*S3*/
	stored_line_t stored_line[MAX_LINE];
	initialize_stored_lines(stored_line);

	get_sentence(stored_line, argv, argc);
	
	/*S4*/
	print_rank(stored_line);
	
	return 0;
}

/*S1*/

/*check correct number of input query*/
void check_argc(int argc) {
	if (argc == 1) {
		printf("%s%s", STAGE_ONE, NO_QUERY);
		exit(EXIT_FAILURE);
	}
	return;
}

/*check which input query in invalid*/
void check_query(int argc, char *argv[]) {
	int i, validation = VALID;

	for (i = 1; i < argc; i++) {
		if (!check_query_string(argv[i])) {
			printf("\n%s%s%s", STAGE_ONE, argv[i], INVALID_CHARACTER);
			validation = INVALID;
		}
	}
	
	if (!validation) {
		exit(EXIT_FAILURE);
	}
	
	return;
}

/*a query in invalid when it is not a lower case alphabet or a digit*/
int check_query_string(char *query) {
	int i, validation = VALID;
	for (i = 0; i < strlen(query); i++) {
		if (!isdigit(query[i]) && (!islower(query[i]))) {
			validation = INVALID;
		}
	}
	return validation;
}

/*print out all invalid query*/
void print_query(int argc, char *argv[]) {
	int i;
	printf("%squery =", STAGE_ONE);
	for (i = 1; i < argc; i++) {
		putchar(SPACE);
		printf("%s", argv[i]);
	}
	return;
}

/*initialize score and line number for each stored line*/
void initialize_stored_lines(stored_line_t stored_line[]) {
	int i;
	for (i = 0; i < MAX_LINE; i++) {
		stored_line[i].score = ZERO_DECIMAL;
		stored_line[i].line = 0;
	}
}

/*S2*/

/*strip '\r' in file*/
int mygetchar() {
	int c;
	while ((c=getchar())=='\r') {
	}
	return c;
}

/*print non-empty processing line and its information, 
  calculate score for each line,
  find top5 query matched line*/
void get_sentence(stored_line_t stored_line[], char *query[], int query_num) {
	int i = 0, line_len = 0, word_len = 0;
	int alphabet = 0;
	char c, word[MAX_CHARACTER];
	proc_line_t proc_line;
	proc_line.words = 0;

	while((c = mygetchar(stdin)) && (line_len < MAX_CHARACTER)) {
		/*initialize times for each query before each line is processed*/
		if (!line_len) {
			memset(&query_times[0], 0, sizeof(query_times));
		}
		
		/*store character for word if it is an alphabet or digit*/
		if (isalnum(c)) {
			word[word_len++] = c;
			alphabet = 1;
		}
		/*if turns from alphabet and digit to other character, then a word
		has ended*/
		else if (alphabet) {
			word[word_len] = '\0';

			/*find whether this word is in query or not*/
			find_match_word(query, query_num, word, query_times);

			proc_line.words++;
			word_len = 0;
			alphabet = 0;
		}
		
		/*end of one line*/
		if (c == '\n') {
			/*assign processing value*/
			proc_line.details.sentence[line_len] = '\0';
			proc_line.bytes = line_len--;
			proc_line.details.line = ++i;
			proc_line.details.score = cal_line_score(query, query_num,
				proc_line.words, query_times);
				
			/*check whether this line should be stored*/
			ranking(stored_line, proc_line);
			
			/*print stage three when line is not empty*/
			if (line_len > 0) {
				print_proc_line(proc_line);
			}
			
			word_len = 0;
			line_len = 0;
			proc_line.words = 0;
			
			continue;
		}
		
		/*store character in line*/
		proc_line.details.sentence[line_len++] = c;
	}
}

/*S3*/

/*compare word with each query to find matched word*/
void find_match_word(char *query[], int q_num, char *word, int q_times[]) {
	int i;
	
	/*if the word matched with query, increase query's
	  number of appearance in line by 1*/
	for (i = 0; i < q_num - 1; i++) {
		if ((strlen(query[i+1]) <= strlen(word)) &&
			(!strncasecmp(query[i+1], word, strlen(query[i+1])))) {
			q_times[i]++;
		}
	}
}

/*calculate and return score for each line*/
double cal_line_score(char *query[], int q_num, int w_num, int q_times[]) {
	int i;
	double score = ZERO_DECIMAL;

	for (i = 0; i < q_num-1; i++) {
		score += LOG2((NUMERATOR_CONSTANT + (double)q_times[i]));
	}
	score /= LOG2(DENOMINATOR_CONSTANT + (double)w_num);

	return score;
}

/*print stage three output*/
void print_proc_line(proc_line_t proc_line) {
	printf("\n%s", END_STAGE);
	printf("\n%s", proc_line.details.sentence);
	printf("\n%sline = %d, bytes = %d, words = %d",
		STAGE_TWO,
		proc_line.details.line,
		proc_line.bytes,
		proc_line.words);
	printf("\n%sline = %d, score = %.3lf",
		STAGE_THREE,
		proc_line.details.line,
		proc_line.details.score);
}

/*S4*/

/*find appropriate ranking for proc_line to be stored*/
void ranking(stored_line_t stored_lines[], proc_line_t proc_line) {
	int i;
	for (i = 0; i < MAX_LINE; i++) {
		/*if processing line's score > stored line's score
		  move other record 1 index backward and insert the current resord*/
		if (proc_line.details.score - stored_lines[i].score > EPSILON) {
			move_backward(stored_lines, i);
			stored_lines[i] = proc_line.details;
			break;
		}
	}
}

/*move other storage 1 index backward*/
void move_backward(stored_line_t stored_line[], int start_index) {
	int i;
	/*if something needed to be inserted, last record in not needed to be
	  considered. so started with index 3 and move backward*/
	for (i = MAX_LINE - 2; i >= start_index; i--) {
		stored_line[i+1] = stored_line[i];
	}
}

/*print out final query output*/
void print_rank(stored_line_t stored_lines[]) {
	int i;
	for (i = 0; i < MAX_LINE; i++) {
		/*only print out non-zero-line-number record which is valid*/
		if (stored_lines[i].line > 0) {
			if (i == 0) {
				printf("\n------------------------------------------------");
			}
			printf("\n%sline = %d, score = %.3lf", STAGE_FOUR,
				stored_lines[i].line, stored_lines[i].score);
			printf("\n%s\n%s", stored_lines[i].sentence, END_STAGE);
		}
	}
}

/*Algorithms are fun!*/