/*
 * Copyright 2003 Kevin B. Hendricks, Stratford, Ontario, Canada
 * And Contributors.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. All modifications to the source code must be clearly marked as
 *    such.  Binary redistributions based on modified source code
 *    must be clearly marked as modified versions in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY KEVIN B. HENDRICKS AND CONTRIBUTORS 
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL 
 * KEVIN B. HENDRICKS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

 
 #ifndef _MYTHES_HXX_
#define _MYTHES_HXX_

// some maximum sizes for buffers
#define MAX_WD_LEN 200
#define MAX_LN_LEN 16384


// a meaning with definition, count of synonyms and synonym list
struct mentry {
  char*  defn;
  int  count;
  char** psyns;
};


class MyThes
{

       int  nw;                  /* number of entries in thesaurus */
       char**  list;               /* stores word list */
       unsigned int* offst;              /* stores offset list */
       char *  encoding;           /* stores text encoding; */
 
        FILE  *pdfile;

	// disallow copy-constructor and assignment-operator for now
	MyThes();
	MyThes(const MyThes &);
	MyThes & operator = (const MyThes &);

public:
	MyThes(const char* idxpath, const char* datpath);
	~MyThes();

        // lookup text in index and return number of meanings
	// each meaning entry has a defintion, synonym count and pointer 
        // when complete return the *original* meaning entry and count via 
        // CleanUpAfterLookup to properly handle memory deallocation

        int Lookup(const char * pText, int len, mentry** pme); 

        void CleanUpAfterLookup(mentry** pme, int nmean);

        char* get_th_encoding(); 

private:
        // Open index and dat files and load list array
        int thInitialize (const char* indxpath, const char* datpath);
        
        // internal close and cleanup dat and idx files
        int thCleanup ();

        // read a text line (\n terminated) stripping off line terminator
        int readLine(FILE * pf, char * buf, int nc);

        // binary search on null terminated character strings
        int binsearch(char * wrd, char* list[], int nlst);

};

#endif





