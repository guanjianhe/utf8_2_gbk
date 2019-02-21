#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "unicode.h"

#define IsSurrogate(c)  ((c) >= 0xD800 && (c) <= 0xDFFF)

void gbk_to_utf8 ( FILE* in, FILE* out )
{
    uint8_t s[4]; 
    long len, nc = 0;
    uint32_t dc;
    
    /* ��ȡ�ļ��ֽ���Ŀ */
    fseek ( in, 0L, SEEK_END );
    len = ftell ( in );
    
    /* ��λ���ļ�ͷ�� */
    rewind ( in );

    while ( nc < len )
    {
        fread ( s+1, sizeof ( uint8_t ), 1, in );
        nc++;
        dc = s[1];

		/* ascii�� */
        if ( dc < 0x80 )
        {
            fwrite ( &dc, sizeof ( uint8_t ), 1, out );
        }
        else	/* gbk�� */ 
        {
            fread ( s, sizeof ( uint8_t ), 1, in );
            nc++;
            
            /* gbk ת unicode */
            dc = * ( ( uint16_t * ) s );
            dc = oem2uni ( dc );

			/* unicode ת utf-8 */             
            if ( dc < 0x800 )	/* 2�ֽ� */
            {
                s[0] = ( uint8_t ) ( 0xC0 | ( dc >> 6 & 0x1F ) );
                s[1] = ( uint8_t ) ( 0x80 | ( dc >> 0 & 0x3F ) );
                fwrite ( s, sizeof ( uint8_t ), 2, out );
            }
            else
            {
                if ( dc < 0x10000 )	/* 3�ֽ� */
                {
                    s[0] = ( uint8_t ) ( 0xE0 | ( dc >> 12 & 0x0F ) );
                    s[1] = ( uint8_t ) ( 0x80 | ( dc >> 6 & 0x3F ) );
                    s[2] = ( uint8_t ) ( 0x80 | ( dc >> 0 & 0x3F ) );
                    fwrite ( s, sizeof ( uint8_t ), 3, out );
                }
                else /* 4�ֽ� */
                {
                    s[0] = ( uint8_t ) ( 0xF0 | ( dc >> 18 & 0x07 ) );
                	s[1] = ( uint8_t ) ( 0x80 | ( dc >> 12 & 0x3F ) );
                    s[2] = ( uint8_t ) ( 0x80 | ( dc >> 6 & 0x3F ) );
                    s[3] = ( uint8_t ) ( 0x80 | ( dc >> 0 & 0x3F ) );
                    fwrite ( s, sizeof ( uint8_t ), 4, out );
                }
            }
        }
    }

    fclose ( in );
    fclose ( out );
}


void utf8_to_gbk ( FILE* in, FILE* out )
{
    uint8_t s[4];
    uint16_t rc, ct;
    uint32_t dc;
    long len, nc = 0;

    /* ��ȡ�ļ��ֽ���Ŀ */
    fseek ( in, 0L, SEEK_END );
    len = ftell ( in );

    /* ��λ���ļ�ͷ�� */
    rewind ( in );

    while ( nc < len )
    {
        /* �ȶ�һ���ֽ� */
        fread ( s, sizeof ( uint8_t ), 1, in );
        nc++;

        dc = s[0];

        if ( dc >= 0x80 )
        {
            ct = 0;

            if ( ( dc & 0xE0 ) == 0xC0 )	/* 2�ֽ� */
            {
                dc &= 0x1F;
                ct = 1;
            }

            if ( ( dc & 0xF0 ) == 0xE0 )	/* 3�ֽ� */
            {
                dc &= 0x0F;
                ct = 2;
            }

            if ( ( dc & 0xF8 ) == 0xF0 )	/* 4�ֽ� */
            {
                dc &= 0x07;
                ct = 3;
            }

            if ( ct == 0 )
            {
                continue;
            }

            /* �ٶ�ct���ֽ� */
            fread ( s, sizeof ( uint8_t ), ct, in );
            nc += ct;

            rc = 0;

            do
            {
                if ( ( s[rc] & 0xC0 ) != 0x80 )
                {
                    break;
                }

                dc = dc << 6 | ( s[rc] & 0x3F );
            } while ( ++rc < ct );

            /* ���벻�� */
            if ( rc != ct || dc < 0x80 || IsSurrogate ( dc ) || dc >= 0x110000 )
            {
                continue;
            }

            dc = uni2oem ( dc );

            fwrite ( ( ( uint8_t* ) ( &dc ) ) + 1, sizeof ( uint8_t ), 1, out );

        }

        fwrite ( &dc, sizeof ( uint8_t ), 1, out );
    }

    fclose ( in );
    fclose ( out );
}
/*
int main ( void )
{
    FILE *in, *out;

    if ( ( in = fopen ( "gbk.txt", "rb" ) ) == NULL )
    {
        exit ( EXIT_FAILURE );
    }

    if ( ( out = fopen ( "utf8.txt", "wb" ) ) == NULL )
    {
        exit ( EXIT_FAILURE );
    }

    printf ( "�ļ��򿪳ɹ�\n" );

    gbk_to_utf8 ( in, out );

}
*/

int main ( void )
{
    FILE *in, *out;

    if ( ( in = fopen ( "utf8.txt", "rb" ) ) == NULL )
    {
        exit ( EXIT_FAILURE );
    }

    if ( ( out = fopen ( "gbk.txt", "wb" ) ) == NULL )
    {
        exit ( EXIT_FAILURE );
    }

    utf8_to_gbk ( in, out );

}


#if 0
int main ( int argc, char *argv[] )
{

    FILE* fp;
    int ch;

    if ( argc < 2 )
    {
        exit ( EXIT_FAILURE );
    }

    if ( ( fp = fopen ( argv[1], "r" ) ) == NULL )
    {
        exit ( EXIT_FAILURE );
    }

    while ( ( ch = getc ( fp ) ) != EOF )
    {
        putc ( ch, stdout );
    }

    return 0;
}
#endif

