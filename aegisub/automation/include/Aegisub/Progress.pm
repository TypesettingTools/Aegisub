package Aegisub::Progress;
use Exporter 'import';

@EXPORT = qw( set_progress set_task set_title is_cancelled );
@EXPORT_OK = qw( set task title );

sub set { set_progress @_ }
sub task { set_task @_ }
sub title { set_title @_ }

1;
