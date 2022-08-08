#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <getopt.h>

bool g_bVerbose ;
int g_iSetIndexBit ;
int g_iLinePerBit ;
int g_iBlockOffsetBit ;
FILE * g_fpTracingFile ;

int g_iHit ;
int g_iMiss ;
int g_iEvict ;

typedef struct cacheData
{
    bool m_bValid ;
    unsigned long long m_ullTag ;
    int m_iLRU ;
} cacheData ;

void checkArgument ( int argc , char * argv [] )
{
    int iOperation ;



    if ( 1 == argc )
    {
        printf ( "Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>" ) ;

        exit ( 1 ) ;
    }
    
    while ( 1 != ( iOperation = getopt ( argc , argv , "hvs:E:b:t:") ) )
    {
        if ( 'h' == iOperation )
        {
            printf ( "Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n" ) ;
            printf ( "Options:\n" ) ;
            printf ( "  -h         Print this help message.\n" ) ;
            printf ( "  -v         Optional verbose flag.\n" ) ;
            printf ( "  -s <num>   Number of set index bits.\n" ) ;
            printf ( "  -E <num>   Number of lines per set.\n" ) ;
            printf ( "  -b <num>   Number of block offset bits.\n" ) ;
            printf ( "  -t <file>  Trace file.\n" ) ;
            printf ( "\n" ) ;
            printf ( "Examples:\n" ) ;
            printf ( "  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n" ) ;
            printf ( "  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n" ) ;
            exit ( 1 ) ;
        }
        else if ( 'v' == iOperation )
        {
            g_bVerbose = true ;
        }
        else if ( 's' == iOperation )
        {
            g_iSetIndexBit = atoi ( optarg );
        }
        else if ( 'E' == iOperation )
        {
            g_iLinePerBit = atoi ( optarg );
        }
        else if ( 'b' == iOperation )
        {
            g_iBlockOffsetBit = atoi ( optarg );
        }
        else if ( 't' == iOperation )
        {
            g_fpTracingFile = fopen ( optarg , "r" ) ;
        }
        else
        {
            printf ( "Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>" ) ;

            exit ( 1 ) ;
        }
    }
}

void accessCache ( unsigned long long ullTag , cacheData * pCacheSet , int iLineCount )
{
    int i = 0 ;
    int iEvictIndex = 0 ;
    int iEvictLRU = 0 ;



    for ( i = 0 ; i < g_iLinePerBit ; ++i )             // Find then HIT
    {
        if ( ( pCacheSet [ i ].m_bValid ) && ( pCacheSet [ i ].m_ullTag == ullTag ) )
        {
            pCacheSet [ i ].m_iLRU = iLineCount ;
            ++ g_iHit ;

            if ( g_bVerbose )
                printf ( "HIT\t%d\n" , iLineCount ) ;

            return ;
        }
    }


    ++ g_iMiss ;                                        // Can't find at former, so MISS

    for ( i = 0 ; i < g_iLinePerBit ; ++i )
    {
        if ( ! pCacheSet [ i ].m_bValid )
        {
            pCacheSet [ i ].m_bValid = true ;
            pCacheSet [ i ].m_ullTag = ullTag ;
            pCacheSet [ i ].m_iLRU = iLineCount ;

            if ( g_bVerbose )
                printf ( "MISS\t%d\n" , iLineCount ) ;

            return ;
        }

        if ( pCacheSet [ i ].m_iLRU < iEvictLRU )
        {
            iEvictLRU = pCacheSet [ i ].m_iLRU ;
            iEvictIndex = i ;
        }
    }


    ++ g_iEvict ;                                       // Can't find empty set, so EVICT

    pCacheSet [ iEvictIndex ].m_bValid = true ;
    pCacheSet [ iEvictIndex ].m_ullTag = ullTag ;
    pCacheSet [ iEvictIndex ].m_iLRU = iLineCount ;

    if ( g_bVerbose )
        printf ( "EVICT\t%d\n" , iLineCount ) ;
}

void simulateCache ()
{
    int iCacheSize = 1 << g_iSetIndexBit ;
    cacheData ** dpCacheTable = malloc ( iCacheSize * sizeof ( cacheData * ) ) ;
    int i = 0 ;
    int j = 0 ;
    char crgLine [ 30 ] ;
    int iLineCount = 0 ;
    char cOperation ;
    int iAddress ;
    int iSize ;
    unsigned long long ullTag ;
    unsigned int uiSet ;



    for ( i = 0 ; i < iCacheSize ; ++i )
    {
        dpCacheTable [ i ] = malloc ( g_iLinePerBit * sizeof ( cacheData ) ) ;

        for ( j = 0 ; j < g_iLinePerBit ; ++j )
        {
            dpCacheTable [ i ] [ j ].m_bValid = false ;
            dpCacheTable [ i ] [ j ].m_ullTag = 0 ;
            dpCacheTable [ i ] [ j ].m_iLRU = 0 ;
        }
    }


    while ( fgets ( crgLine , sizeof ( crgLine ) , g_fpTracingFile ) )
    {
        ++ iLineCount ;

        if ( 'I' == * crgLine )
            continue ;


        sscanf ( crgLine , " %c %x,%d" , & cOperation , & iAddress , & iSize ) ;

        ullTag = iAddress >> ( g_iSetIndexBit + g_iBlockOffsetBit ) ;
        uiSet = ( iAddress >> g_iBlockOffsetBit ) & ( ( 1 << g_iSetIndexBit ) - 1 ) ;

        if ( 'M' == cOperation )
        {
            accessCache ( ullTag , dpCacheTable [ uiSet ] , iLineCount ) ;
            accessCache ( ullTag , dpCacheTable [ uiSet ] , iLineCount ) ;
        }
        else
        {
            accessCache ( ullTag , dpCacheTable [ uiSet ] , iLineCount ) ;
        }
    }

    for ( i = 0 ; i < iCacheSize ; ++i )
    {
        free ( dpCacheTable [ i ] ) ;
    }
    free ( dpCacheTable ) ;
}

int main ( int argc , char * argv [] )
{
    checkArgument ( argc , argv ) ;
    simulateCache () ;
    printSummary ( g_iHit , g_iMiss , g_iEvict ) ;


    return 0 ;
}