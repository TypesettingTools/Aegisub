// A dummy replacement for Catalogue.cxx that doesn't support any lexers

#include <stdlib.h>

#include "ILexer.h"
#include "LexerModule.h"
#include "Catalogue.h"

const LexerModule *Catalogue::Find(int language) { return nullptr; }
const LexerModule *Catalogue::Find(const char *languageName) { return nullptr; }
void Catalogue::AddLexerModule(LexerModule *plm) { }
int Scintilla_LinkLexers() { return 1; }

