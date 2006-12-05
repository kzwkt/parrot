# Copyright (C) 2001-2003, The Perl Foundation.
# $Id$

=head1 NAME

config/auto/gcc.pm - GNU C Compiler

=head1 DESCRIPTION

Determines whether the C compiler is actually C<gcc>.

=cut

package auto::gcc;

use strict;
use warnings;
use vars qw($description @args);

use base qw(Parrot::Configure::Step::Base);

use Parrot::Configure::Step ':auto';

$description = 'Determining if your C compiler is actually gcc';

@args = qw(miniparrot verbose);

sub runstep
{
    my ($self, $conf) = @_;

    my $verbose = $conf->options->get('verbose');
    my $maint   = $conf->options->get('maintainer');

    my %gnuc;

    cc_gen("config/auto/gcc/test_c.in");
    cc_build();
    %gnuc = eval cc_run() or die "Can't run the test program: $!";
    cc_clean();

    my ($gccversion, $warns, $ccwarn);
    $ccwarn = $conf->data->get('ccwarn');

    # Set gccversion to undef.  This will also trigger any hints-file
    # callbacks that depend on knowing whether or not we're using gcc.

    # This key should always exist unless the program couldn't be run,
    # which should have been caught by the 'die' above.
    unless (exists $gnuc{__GNUC__}) {
        $conf->data->set(gccversion => undef);
        return $self;
    }

    my $major = $gnuc{__GNUC__};
    my $minor = $gnuc{__GNUC_MINOR__};
    unless (defined $major) {
        print " (no) " if $verbose;
        $self->set_result('no');
        $conf->data->set(gccversion => undef);
        return $self;
    }
    if ($major =~ tr/0-9//c) {
        undef $major; # Don't use it
    }
    if (defined $minor and $minor =~ tr/0-9//c) {
        undef $minor; # Don't use it
    }
    if (defined $major) {
        $gccversion = $major;
        $gccversion .= ".$minor" if defined $minor;
    }
    print " (yep: $gccversion )" if $verbose;
    $self->set_result('yes');

    if ($gccversion) {

        # If using gcc, crank up its warnings as much as possible and make it
        # behave  ansi-ish.  Here's an attempt at a list of nasty things we can
        # use for a given version of gcc. The earliest documentation I
        # currently have access to is for 2.95, so I don't know what version
        # everything came in at. If it turns out that you're using 2.7.2 and
        # -Wfoo isn't recognised there, move it up into the next version becone
        # (2.8)

        # Don't use -ansi -pedantic.  It makes it much harder to compile using
        # the system headers, which may well be tuned to a non-strict
        # environment -- especially since we are using perl5 compilation flags
        # determined in a non-strict environment.  An example is Solaris 8.

        my @opt_and_vers = (
            0 =>
                "-W -Wall -Wstrict-prototypes -Wmissing-prototypes -Winline"
                ." -Wshadow -Wpointer-arith -Wcast-qual"
                ." -Wwrite-strings -Waggregate-return -Winline -Wno-unused"
                .($maint ? " -Wlarger-than-4096" : ""),

            # others; ones we might like marked with ?
            # ? -Wundef for undefined idenfiers in #if
            # ? -Wbad-function-cast
            #   Warn whenever a function call is cast to a non-matching type
            # ? -Wmissing-declarations
            #   Warn if a global function is defined without a previous
            #   declaration -Wmissing-noreturn
            # ? -Wredundant-decls
            #    Warn if anything is declared more than once in the same scope,
            # ? -Wnested-externs
            #    Warn if an `extern' declaration is encountered within an
            #    function.  -Wlong-long
            # Ha. this is the default! with -pedantic.
            # -Wno-long-long for the nicest bit of C99
            #
            # -Wcast-align is now removed: it gives too many false positives
            #    e.g. when accessing registers - this is all aligned

            2.7  => "",
            2.8  => "-Wsign-compare",
            2.95 => "",

            # 2.95 does align functions per default -malign-functions=4
            #      where the argument is used as a power of 2
            # 3.x  does not align functions per default, its turned on with
            #      -O2 and -O3
            #      -falign-functions=16 is the real alignment, no exponent
            3.0 => "-falign-functions=16"
                   ." -Wformat-nonliteral -Wformat-security -Wpacked"
                   ." -Wdisabled-optimization -mno-accumulate-outgoing-args"
                   ." -Wno-shadow",

            # -Wsequence-point is part of -Wall
            # -Wfloat-equal may not be what we want
            # We shouldn't be using __packed__, but I doubt -Wpacked will harm
            # us -Wpadded may prove interesting, or even noisy.
            # -Wunreachable-code might be useful in a non debugging version
        );
        $warns = "";
        while (my ($vers, $opt) = splice @opt_and_vers, 0, 2) {
            last if $vers > $gccversion;
            next unless $opt; # Ignore blank lines

            if ($opt =~ /-mno-accumulate-outgoing-args/) {
                use Config;
                if ($Config{archname} !~ /86/) {
                    $opt =~ s/-mno-accumulate-outgoing-args//;
                }
            }
            $warns .= " $opt";
        }

        # if the user overwrites the warnings remove it from $warns
        if ($ccwarn) {
            my @warns = split ' ', $warns;
            foreach my $w (split ' ', $ccwarn) {
                $w =~ s/^-W(?:no-)?(.*)$/$1/;
                @warns = grep !/^-W(?:no-)?$w$/, @warns;
            }
            $warns = join ' ', @warns;
        }
    }

    if (defined $conf->options->get('miniparrot') && $gccversion) {

        # make the compiler act as ANSIish as possible, and avoid enabling
        # support for GCC-specific features.

        $conf->data->set(
            ccwarn     => "-ansi -pedantic",
            gccversion => undef
        );

        return $self;
    }

    $conf->data->set(
        ccwarn              => "$warns $ccwarn",
        gccversion          => $gccversion,
        HAS_aligned_funcptr => 1
    );

    $conf->data->set(HAS_aligned_funcptr => 0)
        if $^O eq 'hpux';

    return $self;
}

1;

# Local Variables:
#   mode: cperl
#   cperl-indent-level: 4
#   fill-column: 100
# End:
# vim: expandtab shiftwidth=4:
