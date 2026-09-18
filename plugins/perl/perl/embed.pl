use strict;
use warnings;

package Editor;

# ask_user() is provided by C++ via XS

package main;

sub main {
    my $result = Editor::ask_user("Enter something:");

    if ($result->{ok}) {
        print "User typed: $result->{text}\n";
    } else {
        print "User cancelled\n";
    }
}
