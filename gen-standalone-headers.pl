#!/usr/bin/perl

# TODO: print stuff to $out_file
# TODO: make die messages clear

use strict;
use warnings;
use IO::File;
use File::Spec;
use Getopt::Long;
use 5.26.1;

my $target_dir = undef;
my $input_file = undef;
my $style = undef;
my $out_file = undef;

sub stl_lib_handler
{
    my ($value) = @_;
    my $inc = File::Spec->catfile ($target_dir, $value);
    say ("stl lib stuff with value $value");
    say ("#include \"$inc\"");
}

sub stl_stl_handler
{
    my ($value) = @_;
    say ("stl stl stuff with value $value");
    say ("#include <$value>");
}

sub stl_sizeof_handler
{
    my ($value) = @_;
    my $uc_value = uc ($value);
    say ("stl sizeof stuff with value $value");
    say ("#define SIZEOF_$uc_value (sizeof ($value))");
}

my %stl_type_handlers = (
    'lib' => \&stl_lib_handler,
    'stl' => \&stl_stl_handler,
    'sizeof' => \&stl_sizeof_handler,
    );

sub gcc_lib_handler
{
    my ($value) = @_;
    say ("gcc lib stuff with value $value");
}

sub gcc_stl_handler
{
    my ($value) = @_;
    say ("gcc stl stuff with value $value");
}

sub gcc_sizeof_handler
{
    my ($value) = @_;
    say ("gcc sizeof stuff with value $value");
}

my %gcc_type_handlers = (
    'lib' => \&gcc_lib_handler,
    'stl' => \&gcc_stl_handler,
    'sizeof' => \&gcc_sizeof_handler,
    );

my $type_handlers_per_style = {
    'gcc' => \%gcc_type_handlers,
    'stl' => \%stl_type_handlers,
};

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

$out_file = File::Spec->catfile ($target_dir, (File::Spec->splitpath ($input_file))[2]);
say ("out file is $out_file");

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
