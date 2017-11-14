#!/usr/bin/perl

# TODO: print stuff to $out_file
# TODO: make die messages clear

use strict;
use warnings;
use 5.26.1;

use File::Path;
use File::Spec;
use Getopt::Long;
use IO::File;

sub stl_check_handler
{
    my ($context, $value) = @_;
    my $fd = $context->{'output_fd'};
    my $token_inc = File::Spec->catfile ($context->{'dir'}, 'token.hh');
    my ($volume, $dir, $file) = File::Spec->splitpath ($context->{'input_filename'});
    my @parts = grep { !/^$/ } File::Spec->splitdir ($dir);
    say ("v: $volume, d: $dir, f: $file");
    say ("parts: @parts,");
    push (@parts, split (/\./, $file), "check");
    my $check_var = join ('_', map { uc ($_) } @parts);
    my $check_value = join ('_', map { uc ($_) } File::Spec->splitdir ($context->{'dir'})) . '_TOKEN';
    $fd->say ("#include \"$token_inc\"");
    $fd->say ("#define $check_var $check_value");
}

sub stl_lib_handler
{
    my ($context, $value) = @_;
    my $inc = File::Spec->catfile ($context->{'generated_dir'}, $value);
    $context->{'output_fd'}->say ("#include \"$inc\"");
}

sub stl_stl_handler
{
    my ($context, $value) = @_;
    $context->{'output_fd'}->say ("#include <$value>");
}

sub stl_sizeof_handler
{
    my ($context, $value) = @_;
    my $uc_value = uc ($value);
    $context->{'output_fd'}->say ("#define SIZEOF_$uc_value (sizeof ($value))");
}

sub gcc_lib_handler
{
    my ($context, $value) = @_;
}

sub gcc_stl_handler
{
    my ($context, $value) = @_;
}

sub gcc_sizeof_handler
{
    my ($context, $value) = @_;
}

my %stl_type_handlers = (
    'check' => \&stl_check_handler,
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

GetOptions ("target-dir=s" => \$target_dir,
            "input-file=s" => \$input_file,
            "style=s" => \$style) or die;

die unless (defined ($target_dir));
die unless (-d $target_dir);
die unless (defined ($input_file));
die unless (-f $input_file);
die unless (defined ($style));
die unless (exists ($type_handlers_per_style->{$style}));

$target_dir = File::Spec->canonpath ($target_dir);
$input_file = File::Spec->canonpath ($input_file);
my $generated_dir = File::Spec->canonpath (File::Spec->catfile ($target_dir, 'generated'));
File::Path->make_path ($generated_dir);
my $final_output_file = File::Spec->catfile ($generated_dir, (File::Spec->splitpath ($input_file))[2]);
my $output_file = "$final_output_file.tmp";
my $encoding = ':encoding(UTF-8)';
my $in = IO::File->new ($input_file, 'r');
my $out = IO::File->new ($output_file, 'w');
die unless (defined ($in));
die unless (defined ($out));
die unless ($in->binmode ($encoding));
die unless ($out->binmode ($encoding));
my $type_handlers = $type_handlers_per_style->{$style};
my $context = {
    'dir' => $target_dir,
    'generated_dir' => $generated_dir,
    'input_filename' => $input_file,
    'input_fd' => $in,
    'final_output_filename' => $final_output_file,
    'tmp_output_filename' => $output_file,
    'output_fd' => $out,
    'state' => {},
};
while (defined (my $line = $in->getline ()))
{
    chomp ($line);
    last if ($line =~ /^namespace/);
    next if ($line !~ m!^[/][*][<]\s*(\w+)\s*:\s*(\S+)\s*[>][*][/]$!);
    my $type = $1;
    my $value = $2;
    if (exists ($type_handlers->{$type}))
    {
        $type_handlers->{$type}($context, $value);
    }
    else
    {
        say ("unknown type: $type");
    }
}
$out->say ("");
$out->say ("#include \"$input_file\"");
$out->close ();
$in->close ();
rename ($output_file, $final_output_file) or die;
