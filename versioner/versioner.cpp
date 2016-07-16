// versioner.cpp : Defines the entry point for the console application.
//

#include <cstdio>
#include <tchar.h>


int _tmain(int argc, _TCHAR* argv[])
{
	int a,b,c,d;

	FILE *fp=fopen("res\\revision.tmp","r");
	fscanf(fp, "%d.%d.%d-%d-", &a, &b, &c, &d);
	fclose(fp);

	fp = fopen("res\\revision.h", "w");
	fprintf(fp,"#define VERSION_A\t%d\n",a);
	fprintf(fp,"#define VERSION_B\t%d\n",b);
	fprintf(fp,"#define VERSION_C\t%d\n",c);
	fprintf(fp,"#define VERSION_D\t%d\n",d);
	fclose(fp);

	fp=fopen("res\\ver","w");
	fprintf(fp,"%d",d+c*10000+b*1000000+a*100000000);
	fclose(fp);

	return 0;
}

