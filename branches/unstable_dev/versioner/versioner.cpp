// versioner.cpp : Defines the entry point for the console application.
//

#include <cstdio>
#include <tchar.h>


int _tmain(int argc, _TCHAR* argv[])
{
	int a,b,c,d;

	FILE *fp=fopen("res\\version.h","r");
	fscanf(fp,"#define VERSION_A\t%d\n",&a);
	fscanf(fp,"#define VERSION_B\t%d\n",&b);
	fscanf(fp,"#define VERSION_C\t%d\n",&c);
	fclose(fp);

	fp=fopen("res\\revision.h","r");
	fscanf(fp,"#define VERSION_D %d\n",&d);
	fclose(fp);

	fp=fopen("res\\ver","w");
	fprintf(fp,"%d",d+c*10000+b*1000000+a*100000000);
	fclose(fp);

	return 0;
}

