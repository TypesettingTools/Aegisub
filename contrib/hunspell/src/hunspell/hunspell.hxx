#include "hashmgr.hxx"
#include "affixmgr.hxx"
#include "suggestmgr.hxx"
#include "csutil.hxx"
#include "langnum.hxx"

#define  SPELL_COMPOUND  (1 << 0)
#define  SPELL_FORBIDDEN (1 << 1)
#define  SPELL_ALLCAP    (1 << 2)
#define  SPELL_NOCAP     (1 << 3)
#define  SPELL_INITCAP   (1 << 4)

#define MAXSUGGESTION 15
#define MAXSHARPS 5

#ifndef _MYSPELLMGR_HXX_
#define _MYSPELLMGR_HXX_

#ifdef HUNSPELL_STATIC
	#define DLLEXPORT
#else
	#ifdef HUNSPELL_EXPORTS
		#define DLLEXPORT  __declspec( dllexport )
	#else
		#define DLLEXPORT  __declspec( dllimport )
	#endif
#endif

#ifdef W32
class DLLEXPORT Hunspell
#else
class Hunspell
#endif
{
  AffixMgr*       pAMgr;
  HashMgr*        pHMgr;
  SuggestMgr*     pSMgr;
  char *          encoding;
  struct cs_info * csconv;
  int             langnum;
  int             utf8;
  int             complexprefixes;
  char**          wordbreak;

public:

  /* Hunspell(aff, dic) - constructor of Hunspell class
   * input: path of affix file and dictionary file
   */
  
  Hunspell(const char * affpath, const char * dpath);

  ~Hunspell();

  /* spell(word) - spellcheck word
   * output: 0 = bad word, not 0 = good word
   *   
   * plus output:
   *   info: information bit array, fields:
   *     SPELL_COMPOUND  = a compound word 
   *     SPELL_FORBIDDEN = an explicit forbidden word
   *   root: root (stem), when input is a word with affix(es)
   */
   
  int spell(const char * word, int * info = NULL, char ** root = NULL);

  /* suggest(suggestions, word) - search suggestions
   * input: pointer to an array of strings pointer and the (bad) word
   *   array of strings pointer (here *slst) may not be initialized
   * output: number of suggestions in string array, and suggestions in
   *   a newly allocated array of strings (*slts will be NULL when number
   *   of suggestion equals 0.)
   */

  int suggest(char*** slst, const char * word);

  char * get_dic_encoding();

 /* morphological functions */

 /* analyze(result, word) - morphological analysis of the word */
 
  int analyze(char*** slst, const char * word);

 /* stem(result, word) - stemmer function */
  
  int stem(char*** slst, const char * word);
  
 /* stem(result, analysis, n) - get stems from a morph. analysis
  * example:
  * char ** result, result2;
  * int n1 = analyze(result, "words");
  * int n2 = stem(result2, result, n1);   
  */
 
  int stem(char*** slst, char ** morph, int n);

 /* generate(result, word, word2) - morphological generation by example(s) */

  int generate(char*** slst, const char * word, const char * word2);

 /* generate(result, word, desc, n) - generation by morph. description(s)
  * example:
  * char ** result;
  * char * affix = "is:plural"; // description depends from dictionaries, too
  * int n = generate(result, "word", &affix, 1);
  * for (int i = 0; i < n; i++) printf("%s\n", result[i]);
  */

  int generate(char*** slst, const char * word, char ** desc, int n);

  /* functions for run-time modification of the dictionary */

  /* add word to the run-time dictionary */
  
  int add(const char * word);

  /* add word to the run-time dictionary with affix flags of
   * the example (a dictionary word): Hunspell will recognize
   * affixed forms of the new word, too.
   */
  
  int add_with_affix(const char * word, const char * example);

  /* remove word from the run-time dictionary */
  /* NOTE: not implemented yet */

  int remove(const char * word);

  /* other */

  /* get extra word characters definied in affix file for tokenization */
  const char * get_wordchars();
  unsigned short * get_wordchars_utf16(int * len);

  struct cs_info * get_csconv();
  const char * get_version();
  
  /* experimental and deprecated functions */

#ifdef HUNSPELL_EXPERIMENTAL
  /* suffix is an affix flag string, similarly in dictionary files */  
  int put_word_suffix(const char * word, const char * suffix);
  char * morph_with_correction(const char * word);

  /* spec. suggestions */
  int suggest_auto(char*** slst, const char * word);
  int suggest_pos_stems(char*** slst, const char * word);
  char * get_possible_root();
#endif

private:
   int    cleanword(char *, const char *, int * pcaptype, int * pabbrev);
   int    cleanword2(char *, const char *, w_char *, int * w_len, int * pcaptype, int * pabbrev);
   void   mkinitcap(char *);
   int    mkinitcap2(char * p, w_char * u, int nc);
   int    mkinitsmall2(char * p, w_char * u, int nc);
   void   mkallcap(char *);
   int    mkallcap2(char * p, w_char * u, int nc);
   void   mkallsmall(char *);
   int    mkallsmall2(char * p, w_char * u, int nc);
   struct hentry * checkword(const char *, int * info, char **root);
   char * sharps_u8_l1(char * dest, char * source);
   hentry * spellsharps(char * base, char *, int, int, char * tmp, int * info, char **root);
   int    is_keepcase(const hentry * rv);
   int    insert_sug(char ***slst, char * word, int ns);
   char * cat_result(char * result, char * st);
   char * stem_description(const char * desc);

};

#endif
