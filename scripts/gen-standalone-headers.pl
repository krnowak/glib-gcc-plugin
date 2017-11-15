#!/usr/bin/perl

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
    my $token_inc = File::Spec->catfile (@{$context->{'out_components'}}, 'token.hh');
    my @token_def_parts = (
        @{$context->{'out_components'}},
        'token',
    );
    my $token_def = join ('_', map { uc ($_) } @token_def_parts);
    my $lines = $context->{'state'}{'lines'};
    push (@{$lines}, "#include \"$token_inc\"", "#define $value $token_def");
}

sub stl_done_handler
{
    my ($context, $fd) = @_;
    my $lines = $context->{'state'}{'lines'};
    for my $line (@{$lines})
    {
        $fd->say ($line);
    }
}

sub stl_lib_handler
{
    my ($context, $value) = @_;
    my $inc = File::Spec->catfile (@{$context->{'out_gen_components'}}, $value);
    my $lines = $context->{'state'}{'lines'};
    push (@{$lines}, "#include \"$inc\"");
}

sub stl_sizeof_handler
{
    my ($context, $value) = @_;
    my $uc_value = uc ($value);
    my $lines = $context->{'state'}{'lines'};
    push (@{$lines}, "#define SIZEOF_$uc_value (sizeof ($value))");
}

sub stl_start_handler
{
    my ($context) = @_;
    $context->{'state'}{'lines'} = [];
}

sub stl_stl_handler
{
    my ($context, $value) = @_;
    my $lines = $context->{'state'}{'lines'};
    push (@{$lines}, "#include <$value>");
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
    'done' => \&stl_done_handler,
    'lib' => \&stl_lib_handler,
    'sizeof' => \&stl_sizeof_handler,
    'start' => \&stl_start_handler,
    'stl' => \&stl_stl_handler,
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

my $include_input = undef;
my $input_file = undef;
my $out_components = undef;
my $out_gen_components = undef;
my $output_file = undef;
my $style = undef;

GetOptions ("include-input=s" => \$include_input,
            "input=s" => \$input_file,
            "out-components=s" => \$out_components,
            "out-gen-components=s" => \$out_gen_components,
            "output=s" => \$output_file,
            "style=s" => \$style) or die;

die unless (defined ($include_input));
die unless (defined ($input_file));
die unless (defined ($out_components));
die unless (defined ($out_gen_components));
die unless (defined ($output_file));
die unless (defined ($style));
die unless (exists ($type_handlers_per_style->{$style}));

my $encoding = ':encoding(UTF-8)';
my $in = IO::File->new ($input_file, 'r');
my $out = IO::File->new ($output_file, 'w');
die unless (defined ($in));
die unless (defined ($out));
die unless ($in->binmode ($encoding));
die unless ($out->binmode ($encoding));
my $type_handlers = $type_handlers_per_style->{$style};
my $context = {
    'out_components' => [split (',', $out_components)],
    'out_gen_components' => [split (',', $out_gen_components)],
    'state' => {},
};
die unless (exists ($type_handlers->{'start'}));
$type_handlers->{'start'}($context);
while (defined (my $line = $in->getline ()))
{
    chomp ($line);
    last if ($line =~ /^namespace/);
    next if ($line !~ m!^[/][*][<]\s*(\w+)\s*:\s*(\S+)\s*[>][*][/]$!);
    my $type = $1;
    my $value = $2;
    die unless (exists ($type_handlers->{$type}));
    $type_handlers->{$type}($context, $value);
}
die unless (exists ($type_handlers->{'done'}));
$type_handlers->{'done'}($context, $out);
$out->say ("");
$out->say ("#include \"$include_input\"");
$out->close ();
$in->close ();
