package Aegisub;
use Exporter 'import';

@EXPORT = qw( text_extents
              log_fatal log_error log_warning log_hint log_debug log_trace log_message );
@EXPORT_OK = qw( LOG_FATAL LOG_ERROR LOG_WARNING LOG_HINT LOG_DEBUG LOG_TRACE LOG_MESSAGE
                 LOG_WX
                 log warn );

# Constants
sub LOG_FATAL	{ 0 }
sub LOG_ERROR	{ 1 }
sub LOG_WARNING	{ 2 }
sub LOG_HINT	{ 3 }
sub LOG_DEBUG	{ 4 }
sub LOG_TRACE	{ 5 }
sub LOG_MESSAGE	{ 6 }

sub LOG_WX	{ 8 }

# Shortcut functions
sub log_fatal	{ Aegisub::log LOG_FATAL, @_; }
sub log_error	{ Aegisub::log LOG_ERROR, @_; }
sub log_warning	{ Aegisub::log LOG_WARNING, @_; }
sub log_hint	{ Aegisub::log LOG_HINT, @_; }
sub log_debug	{ Aegisub::log LOG_DEBUG, @_; }
sub log_trace	{ Aegisub::log LOG_TRACE, @_; }
sub log_message	{ Aegisub::log LOG_MESSAGE, @_; }

# wxLog variety
sub wxlog {
  if($_[0] =~ /^\d+$/) {
    $_[0] |= 0x8;
  }
  else {
    unshift @_, LOG_MESSAGE | LOG_WX;
  }
  Aegisub::log @_;
}

1;
