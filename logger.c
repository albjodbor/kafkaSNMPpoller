//logger.c
#include "logger.h"


FILE *fp ;
static int inicio = 0;

char* print_time()
{
    time_t t;
    char *buf;
    time(&t);
    const char * timechar =ctime(&t);
    size_t tam_time = strlen(timechar);
    buf = (char*)malloc(tam_time+ 1);
    snprintf(buf,strlen(timechar),"%s ", timechar);
    return buf;
}

void log_print(char* filename, int line, char *fmt,...)
{
    va_list         list;
    char            *p, *r;
    int             e;

    if(inicio > 0)
      fp = fopen ("poller-log.txt","a");
    else
      fp = fopen ("poller-log.txt","w");


    /*
     * TODO print_time devuelve malloc
     * HAY QUE LIBERAR
     */
    char * ptime = print_time();
    fprintf(fp,"%s ",ptime);
    free(ptime);
    va_start( list, fmt );

    for ( p = fmt ; *p ; ++p )
    {
        if ( *p != '%' )
            fputc( *p,fp );
        else
        {
            switch ( *++p ) {
            case 's':  {
                r = va_arg( list, char * );
                fprintf(fp,"%s", r);
                continue;
            }
            case 'd': {
                e = va_arg( list, int );
                fprintf(fp,"%d", e);
                continue;
            }
            default:
                fputc( *p, fp );
            }
        }
    }
    va_end( list );
    fprintf(fp," [%s][line: %d] ",filename,line);
    fputc( '\n', fp );
    inicio++;
    fclose(fp);
}
