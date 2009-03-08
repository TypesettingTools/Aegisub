# Perl console script
# by Simone Cociancich
# This script simply call the registration function for the builtin perl console
# the perl console is chiefly intended as a development and debug tool

use strict;
use warnings;

Aegisub::Script::set_info(
'Perl console',
"\nThis script provides a console for messing with the perl engine \\^^/
(if you break something don't complain >:)",
'ShB');

use Aegisub::PerlConsole;
register_console();
