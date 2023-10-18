/*
* luainternals.h
* Code quoted from MIT-licensed Lua 5.1.4 internals
* See copyright notice in lua.h
*/

#ifndef LUABINS_LUAINTERNALS_H_INCLUDED_
#define LUABINS_LUAINTERNALS_H_INCLUDED_

/*
* BEGIN COPY-PASTE FROM Lua 5.1.4 luaconf.h
* WARNING: If your Lua config differs, fix this!
*/

#define luai_numeq(a,b)		((a)==(b))
#define luai_numisnan(a)	(!luai_numeq((a), (a)))

/*
* END COPY-PASTE FROM Lua 5.1.4 luaconf.h
*/

/*
* BEGIN COPY-PASTE FROM Lua 5.1.4 lobject.h
*/

int luaO_log2 (unsigned int x);

#define ceillog2(x)       (luaO_log2((x)-1) + 1)

/*
* END COPY-PASTE FROM Lua 5.1.4 lobject.h
*/

/*
* BEGIN COPY-PASTE FROM Lua 5.1.4 ltable.c
*/

/*
** max size of array part is 2^MAXBITS
*/
#define LUAI_BITSINT 32
#if LUAI_BITSINT > 26
#define MAXBITS		26
#else
#define MAXBITS		(LUAI_BITSINT-2)
#endif

#define MAXASIZE	(1 << MAXBITS)

/*
* END COPY-PASTE FROM Lua 5.1.4 ltable.c
*/

#endif /* LUABINS_LUAINTERNALS_H_INCLUDED_ */
