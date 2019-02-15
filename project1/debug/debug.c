#include <stdio.h>  
#include <string.h>  
#include <stdlib.h>

int main(int argv, char *argc[]) {
    char test_text[1001] = {0};
	char sample_text[1001] = {0};
    int len = 0, line = 0, correctness = 1;;
	int i;
    
	FILE *test = fopen(argc[1], "r");  
    if(NULL == test) {  
        printf("failed to open txt\n");  
        return 1;  
    }
	
	FILE *sample = fopen(argc[2], "r");  
    if(NULL == sample) {  
        printf("failed to open txt\n");  
        return 1;  
    }
	
    while(!feof(test))  
    {
        memset(test_text, 0, sizeof(test_text));
		memset(sample_text, 0, sizeof(sample_text));
        fgets(test_text, sizeof(sample_text) - 1, test); // 包含了\n
		fgets(sample_text, sizeof(sample_text) - 1, sample);
        //printf("%s", szTest);  
		
		for (i = 0; i < strlen(sample_text); i++) {
			if (test_text[i] != sample_text[i]) {
				correctness = 0;
				printf("%d line are not the same: \n", line);
				printf("%s\n%s\n---\n", test_text, sample_text);
				break;
			}
		}
		
		line++;
		len++;
    }  
	
	if (correctness) {
		printf("%d input and all the same\n", len);
	}
	
    fclose(test);  
    fclose(sample);
    printf("\n");  
  
    return 0;  
}

void check(int argv) {
	if (argv != 3) {
		printf("wrong inout file\n");
		exit(EXIT_FAILURE);
	}
	return;
}