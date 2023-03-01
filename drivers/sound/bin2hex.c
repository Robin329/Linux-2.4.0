#include <stdio.h>
#include <string.h>

int main( int argc, const char * argv [] )
{
    const char * varname;
    int i = 0;
    int c;
    int id = 0;

    if(argv[1] && strcmp(argv[1],"-i")==0)
    {
    	argv++;
    	argc--;
    	id=1;
    }
    	
    if(argc==1)
    {
    	fprintf(stderr, "bin2hex: [-i] firmware\n");
    	exit(1);
    }
    
    varname = argv[1];
    printf( "/* automatically generated by bin2hex */\n" );
    printf( "static unsigned char %s [] %s =\n{\n", varname , id?"__initdata":"");

    while ( ( c = getchar( ) ) != EOF )
    {
	if ( i != 0 && i % 10 == 0 )
	    printf( "\n" );
	printf( "0x%02lx,", c & 0xFFl );
	i++;
    }

    printf( "};\nstatic int %sLen =  %d;\n", varname, i );
    return 0;
}
