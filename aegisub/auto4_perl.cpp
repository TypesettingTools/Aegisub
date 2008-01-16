// Copyright (c) 2008, Simone Cociancich
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//   * Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//   * Neither the name of the Aegisub Group nor the names of its contributors
//     may be used to endorse or promote products derived from this software
//     without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// -----------------------------------------------------------------------------
//
// AEGISUB
//
// Website: http://aegisub.cellosoft.com
// Contact: mailto:jiifurusu@gmail.com
//


#ifdef WITH_PERL


#include "auto4_perl.h"


namespace Automation4 {

  
///////////////////////////////////
// Perl -> C++ interface (XSUBS)
//
  EXTERN_C void boot_DynaLoader (pTHX_ CV* cv);  

  // Copypasted from somewhere
  EXTERN_C void xs_perl_main(pTHX)
  {
	dXSUB_SYS;
	
	/* DynaLoader is a special case */
	newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, __FILE__);

	// My XSUBS ^^
	xs_perl_script(aTHX);
	xs_perl_misc(aTHX);
	xs_perl_console(aTHX);
  }


///////////////////////
// PerlScriptFactory
//
  class PerlScriptFactory : public ScriptFactory {
  private:
	PerlInterpreter *parser;

  public:
	PerlScriptFactory()
	{ 
	  // Script engine properties
	  engine_name = _T("Perl");
	  filename_pattern = _T("*") _T(PERL_SCRIPT_EXTENSION);
	  
	  // Perl interpreter initialization (ONE FOR ALL THE SCRIPTS)
	  char** env = NULL;
	  int argc = 3;
	  char *argv[3] = { "aegisub", "-e", "0" };
#ifdef __WINDOWS__
	  char **argv2 = (char**) argv;
	  PERL_SYS_INIT3(&argc,&argv2,&env);
#endif
	  parser = perl_alloc();
	  perl_construct(parser);
	  perl_parse(parser, xs_perl_main,
				 argc, argv,
				 NULL);
	  //free(argv);
	  // (That was pretty magic o_O)

	  // Let's register the perl script factory \o/
	  Register(this);
	}
	
	~PerlScriptFactory()
	{
	  // Perl interpreter deinitialization
	  perl_destruct(parser);
	  perl_free(parser);
	}
	
	virtual Script* Produce(const wxString &filename) const
	{
	  if(filename.EndsWith(_T(PERL_SCRIPT_EXTENSION))) {
		return new PerlScript(filename);
	  }
	  else {
		return 0;
	  }
	}
  };

  // The one and only (thank goodness ¬.¬) perl engine!!!
  PerlScriptFactory _perl_script_factory;

};


#endif //WITH_PERL
