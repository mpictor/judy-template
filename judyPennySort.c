//  Judy arrays 9 NOV 2012

//  Author Karl Malbrain, malbrain AT yahoo.com
//  with assistance from Jan Weiss.

//  Simplified judy arrays for strings and integers
//  Adapted from the ideas of Douglas Baskins of HP.

//  Map a set of keys to corresponding memory cells (uints).
//  Each cell must be set to a non-zero value by the caller.

//  STANDALONE is defined to compile into a string sorter.

//  String mappings are denoted by calling judy_open with zero as
//  the second argument.  Integer mappings are denoted by calling
//  judy_open with the Integer depth of the Judy Trie as the second
//  argument.

#ifndef STANDALONE
  #error must define STANDALONE while compiling this file and judy64.c
#endif

#include "judy64.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#ifdef linux
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/times.h>
#else
#include <windows.h>
#include <io.h>
#endif

#include <time.h>

unsigned int MaxMem = 0;

#define PRIuint            "u"

//    memory map input file and sort

//    define pennysort parameters

unsigned int PennyRecs = (4096 * 400);    // records to sort to temp files
unsigned int PennyLine = 100;            // length of input record
unsigned int PennyKey = 10;                // length of input key
unsigned int PennyOff = 0;                // key offset in input record

unsigned long long PennyMerge;    // PennyRecs * PennyLine = file map length
unsigned int PennyPasses;                // number of intermediate files created
unsigned int PennySortTime;                // cpu time to run sort
unsigned int PennyMergeTime;            // cpu time to run merge

typedef struct {
    void *buff;        // record pointer in input file map
    void *next;        // duplicate chain
} PennySort;

void sort (FILE *infile, char *outname)
{
unsigned long long size, off, offset, part;
int ifd = fileno (infile);
char filename[512];
PennySort *line;
JudySlot *cell;
unsigned char *inbuff;
void *judy;
FILE *out;
#if defined(_WIN32)
HANDLE hndl, fm;
DWORD hiword;
FILETIME dummy[1];
FILETIME user[1];
#else
struct tms buff[1];
#endif
time_t start = time(NULL);

  if( PennyOff + PennyKey > PennyLine )
    fprintf (stderr, "Key Offset + Key Length > Record Length\n"), exit(1);

  offset = 0;
  PennyPasses = 0;

#if defined(_WIN32)
  hndl = (HANDLE)_get_osfhandle(ifd);
  size = GetFileSize (hndl, &hiword);
  fm = CreateFileMapping(hndl, NULL, PAGE_READONLY, hiword, (DWORD)size, NULL);
  if( !fm )
    fprintf (stderr, "CreateFileMapping error %d\n", GetLastError()), exit(1);
  size |= (unsigned long long)hiword << 32;
#else
  size = lseek (ifd, 0L, 2);
#endif

  while( offset < size ) {
#if defined(_WIN32)
    part = offset + PennyMerge > size ? size - offset : PennyMerge;
    inbuff = MapViewOfFile( fm, FILE_MAP_READ, offset >> 32, offset, part);
    if( !inbuff )
        fprintf (stderr, "MapViewOfFile error %d\n", GetLastError()), exit(1);
#else
    inbuff = mmap (NULL, PennyMerge, PROT_READ,  MAP_SHARED, ifd, offset);

    if( inbuff == MAP_FAILED )
        fprintf (stderr, "mmap error %d\n", errno), exit(1);

    if( madvise (inbuff, PennyMerge, MADV_WILLNEED | MADV_SEQUENTIAL) < 0 )
        fprintf (stderr, "madvise error %d\n", errno);
#endif
    judy = judy_open (PennyKey, 0);

    off = 0;

    //    build judy array from mapped input chunk

    while( offset + off < size && off < PennyMerge ) {
      line = judy_data (judy, sizeof(PennySort));
      cell = judy_cell (judy, inbuff + off + PennyOff, PennyKey);
      line->next = *(void **)cell;
      line->buff = inbuff + off;

      *(PennySort **)cell = line;
      off += PennyLine;
    }

    sprintf (filename, "%s.%d", outname, PennyPasses);
    out = fopen (filename, "wb");
    setvbuf (out, NULL, _IOFBF, 4096 * 1024);

#ifndef _WIN32
    if( madvise (inbuff, PennyMerge, MADV_WILLNEED | MADV_RANDOM) < 0 )
        fprintf (stderr, "madvise error %d\n", errno);
#endif

    //    write judy array in sorted order to temporary file

    cell = judy_strt (judy, NULL, 0);

    if( cell ) do {
        line = *(PennySort **)cell;
        do fwrite (line->buff, PennyLine, 1, out);
        while( line = line->next );
    } while( cell = judy_nxt (judy) );

#if defined(_WIN32)
    UnmapViewOfFile (inbuff);
#else
    munmap (inbuff, PennyMerge);
#endif
    judy_close (judy);
    offset += off;
    fflush (out);
    fclose (out);
    PennyPasses++;
  }
  fprintf (stderr, "End Sort %d secs", time(NULL) - start);
#if defined(_WIN32)
  CloseHandle (fm);
  GetProcessTimes (GetCurrentProcess(), dummy, dummy, dummy, user);
  PennySortTime = *(unsigned long long*)user / 10000000;
#else
  times (buff);
  PennySortTime = buff->tms_utime/100;
#endif
  fprintf (stderr, " Cpu %d\n", PennySortTime);
}

int merge (FILE *out, char *outname)
{
time_t start = time(NULL);
char filename[512];
JudySlot *cell;
unsigned int nxt, idx;
unsigned char **line;
unsigned int *next;
void *judy;
FILE **in;

    next = calloc (PennyPasses + 1, sizeof(unsigned int));
    line = calloc (PennyPasses, sizeof(void *));
    in = calloc (PennyPasses, sizeof(void *));

    judy = judy_open (PennyKey, 0);

    // initialize merge with one record from each temp file

    for( idx = 0; idx < PennyPasses; idx++ ) {
        sprintf (filename, "%s.%d", outname, idx);
        in[idx] = fopen (filename, "rb");
        line[idx] = malloc (PennyLine);
        setvbuf (in[idx], NULL, _IOFBF, 4096 * 1024);
        fread (line[idx], PennyLine, 1, in[idx]);
        cell = judy_cell (judy, line[idx] + PennyOff, PennyKey);
        next[idx + 1] = *(unsigned int *)cell;
        *cell = idx + 1;
    }

    //    output records, replacing smallest each time

    while( cell = judy_strt (judy, NULL, 0) ) {
        nxt = *(unsigned int *)cell;
        judy_del (judy);

        // process duplicates

        while( idx = nxt ) {
            nxt = next[idx--];
            fwrite (line[idx], PennyLine, 1, out);

            if( fread (line[idx], PennyLine, 1, in[idx]) ) {
                cell = judy_cell (judy, line[idx] + PennyOff, PennyKey);
                next[idx + 1] = *(unsigned int *)cell;
                *cell = idx + 1;
            } else
                next[idx + 1] = 0;
        }
    }

    for( idx = 0; idx < PennyPasses; idx++ ) {
        fclose (in[idx]);
        free (line[idx]);
    }

    free (line);
    free (next);
    free (in);

    fprintf (stderr, "End Merge %d secs", time(NULL) - start);
#ifdef _WIN32
    {
    FILETIME dummy[1];
    FILETIME user[1];
        GetProcessTimes (GetCurrentProcess(), dummy, dummy, dummy, user);
        PennyMergeTime = *(unsigned long long*)user / 10000000;
    }
#else
    {
    struct tms buff[1];
        times (buff);
        PennyMergeTime = buff->tms_utime/100;
    }
#endif
    fprintf (stderr, " Cpu %d\n", PennyMergeTime - PennySortTime);
    judy_close (judy);
    fflush (out);
    fclose (out);
    return 0;
}

//    compilation:
//    cc -O3 judy64j.c

//    usage:
//    a.out [in-file] [out-file] [keysize] [recordlen] [keyoffset] [mergerecs]
//    where keysize is 10 to indicate pennysort files

int main (int argc, char **argv)
{
unsigned char buff[1024];
JudySlot max = 0;
JudySlot *cell;
FILE *in, *out;
void *judy;
unsigned int len;
unsigned int idx;

    if( argc > 1 )
        in = fopen (argv[1], "rb");
    else
        in = stdin;

    if( argc > 2 )
        out = fopen (argv[2], "wb");
    else
        out = stdout;

    setvbuf (out, NULL, _IOFBF, 4096 * 1024);

    if( !in )
        fprintf (stderr, "unable to open input file\n");

    if( !out )
        fprintf (stderr, "unable to open output file\n");

    if( argc > 6 )
        PennyRecs = atoi(argv[6]);

    if( argc > 5 )
        PennyOff = atoi(argv[5]);

    if( argc > 4 )
        PennyLine = atoi(argv[4]);

    PennyMerge = (unsigned long long)PennyLine * PennyRecs;

    if( argc > 3 ) {
        PennyKey = atoi(argv[3]);
        sort (in, argv[2]);
        return merge (out, argv[2]);
    }

    judy = judy_open (1024, 0);

    while( fgets((char *)buff, sizeof(buff), in) ) {
        if( len = strlen((const char *)buff) )
            buff[--len] = 0;                // remove LF
        *(judy_cell (judy, buff, len)) += 1;        // count instances of string
        max++;
    }

    fprintf(stderr, "%" PRIuint " memory used\n", MaxMem);

    cell = judy_strt (judy, NULL, 0);

    if( cell ) do {
        len = judy_key(judy, buff, sizeof(buff));
        for( idx = 0; idx < *cell; idx++ ){        // spit out duplicates
            fwrite(buff, len, 1, out), fputc('\n', out);
        }
    } while( cell = judy_nxt (judy) );

#if 0
    // test deletion all the way to an empty tree

    if( cell = judy_prv (judy) )
        do max -= *cell;
        while( cell = judy_del (judy) );

    assert (max == 0);
#endif
    judy_close(judy);
    return 0;
}

