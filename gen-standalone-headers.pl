#!/usr/bin/perl

use strict;
use warnings;
use IO::File;
use Getopt::Long;
use 5.26.1;

sub stl_lib_handler
{
    my ($value) = @_;
    say ("lib stuff with value $value");
}

sub stl_stl_handler
{
    my ($value) = @_;
    say ("stl stuff with value $value");
}

sub stl_sizeof_handler
{
    my ($value) = @_;
    say ("sizeof stuff with value $value");
}

my %stl_type_handlers = (
    'lib' => \&stl_lib_handler,
    'stl' => \&stl_stl_handler,
    'sizeof' => \&stl_sizeof_handler,
    );

my %gcc_type_handlers = (
    'lib' => \&gcc_lib_handler,
    'stl' => \&gcc_stl_handler,
    'sizeof' => \&gcc_sizeof_handler,
    );

my $type_handlers_per_style = {
    'gcc' => \%gcc_type_handlers,
    'stl' => \%stl_type_handlers,
};

my $target_dir = undef;
my $input_file = undef;
my $style = undef;

# flags:
# - target directory
# - input file
# - style (stl, gcc)

GetOptions ("target-dir=s" => \$target_dir,
            "input-file=s" => \$input_file,
            "style=s" => \$style) or die;

die unless (defined ($target_dir));
die unless (-d $target_dir);
die unless (defined ($input_file));
die unless (-f $input_file);
die unless (defined ($style));
die unless (exists ($type_handlers_per_style->{$style}));

my $in = IO::File->new ($input_file, 'r');
die unless defined ($in);
my $type_handlers = $type_handlers_per_style->{$style};
while (defined (my $line = $in->getline ()))
{
    chomp ($line);
    last if ($line =~ /^namespace/);
    next if ($line !~ m!^[/][*][<]\s*(\w+)\s*:\s*(\S+)\s*[>][*][/]$!);
    my $type = $1;
    my $value = $2;
    say ("$type -> $value");
    if (exists ($type_handlers->{$type}))
    {
        $type_handlers->{$type}($value);
    }
    else
    {
        say ("unknown type: $type");
    }
}
$in->close ();
