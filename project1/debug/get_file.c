#include <stdio.h>
#include <time.h>
 
int main()
{
	time_t t;
	struct tm *p;
	FILE *fp = NULL;
	char fname[256] = {0};

	t = time(NULL);
	p = gmtime(&t);

	sprintf(fname, "%04d-%02d-%02d-%02d-%02d-%02d.txt",1900+p->tm_year,1+p->tm_mon, \
			p->tm_mday, 10+p->tm_hour, p->tm_min, p->tm_sec);
	printf("%s\n", fname);
	if((fp = fopen(fname, "w+")) == NULL)
			perror("");
	fclose(fp);
	return 0;
}