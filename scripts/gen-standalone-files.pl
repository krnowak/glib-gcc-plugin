#!/usr/bin/perl

# TODO: make die messages clear

use strict;
use warnings;
use 5.26.1;

use File::Path;
use File::Spec;
use Getopt::Long;
use IO::File;

# common handlers

sub common_check_handler
{
    my ($context, $value) = @_;
    my $token_inc = File::Spec->catfile (@{$context->{'out_components'}}, 'token.hh');
    my @token_def_parts = (
        @{$context->{'out_components'}},
        'token',
    );
    my $token_def = join ('_', map { uc ($_) } @token_def_parts);
    return ["#include \"$token_inc\"", "#define $value $token_def"];
}

# style std

sub std_check_handler
{
    my ($context, $value) = @_;
    my $generated_lines = common_check_handler ($context, $value);
    my $lines = $context->{'state'}{'lines'};
    push (@{$lines}, @{$generated_lines});
}

sub std_done_handler
{
    my ($context, $fd) = @_;
    my $lines = $context->{'state'}{'lines'};
    for my $line (@{$lines})
    {
        $fd->say ($line);
    }
}

sub std_lib_handler
{
    my ($context, $value) = @_;
    my $inc = File::Spec->catfile (@{$context->{'out_gen_components'}}, $value);
    my $lines = $context->{'state'}{'lines'};
    push (@{$lines}, "#include \"$inc\"");
}

sub std_sizeof_handler
{
    my ($context, $value) = @_;
    my $uc_value = uc ($value);
    my $lines = $context->{'state'}{'lines'};
    push (@{$lines}, "#define SIZEOF_$uc_value (sizeof ($value))");
}

sub std_start_handler
{
    my ($context) = @_;
    $context->{'state'}{'lines'} = [];
}

sub std_stl_handler
{
    my ($context, $value) = @_;
    my $lines = $context->{'state'}{'lines'};
    push (@{$lines}, "#include <$value>");
}

# style gcc

sub gcc_check_handler
{
    my ($context, $value) = @_;
    my $generated_lines = common_check_handler ($context, $value);
    my $pre_gcc_lines = $context->{'state'}{'pre_gcc_lines'};
    push (@{$pre_gcc_lines}, @{$generated_lines});
}

sub gcc_done_handler
{
    my ($context, $fd) = @_;
    my $pre_gcc_lines = $context->{'state'}{'pre_gcc_lines'};
    my $gcc_lines = $context->{'state'}{'gcc_lines'};
    my $post_gcc_lines = $context->{'state'}{'post_gcc_lines'};
    for my $lines ($pre_gcc_lines, $gcc_lines, $post_gcc_lines)
    {
        for my $line (@{$lines})
        {
            $fd->say ($line);
        }
    }
}

sub gcc_lib_handler
{
    my ($context, $value) = @_;
    my $inc = File::Spec->catfile (@{$context->{'out_gen_components'}}, $value);
    my $post_gcc_lines = $context->{'state'}{'post_gcc_lines'};
    push (@{$post_gcc_lines}, "#include \"$inc\"");
}

sub insert_gcc_header
{
    my ($context) = @_;
    my $gcc_lines = $context->{'state'}{'gcc_lines'};
    if (@{$gcc_lines} > 0)
    {
        return;
    }
    push (@{$gcc_lines}, "#include \"ggp/gcc/gcc.hh\"");
}

sub gcc_sizeof_handler
{
    my ($context) = @_;
    insert_gcc_header ($context);
}

sub gcc_start_handler
{
    my ($context) = @_;
    $context->{'state'}{'pre_gcc_lines'} = [];
    $context->{'state'}{'gcc_lines'} = [];
    $context->{'state'}{'post_gcc_lines'} = [];
    $context->{'state'}{'gcc_stl_headers'} = {
        'algorithm' => 1,
        'list' => 1,
        'map' => 1,
        'set' => 1,
        'string' => 1,
        'vector' => 1,
    };
}

sub gcc_stl_handler
{
    my ($context, $value) = @_;
    my $gcc_stl_headers = $context->{'state'}{'gcc_stl_headers'};
    if (exists ($gcc_stl_headers->{$value}))
    {
        insert_gcc_header ($context);
    }
    else
    {
        my $post_gcc_lines = $context->{'state'}{'post_gcc_lines'};
        push (@{$post_gcc_lines}, "#include <$value>");
    }
}

my %std_type_handlers = (
    'check' => \&std_check_handler,
    'done' => \&std_done_handler,
    'lib' => \&std_lib_handler,
    'sizeof' => \&std_sizeof_handler,
    'start' => \&std_start_handler,
    'stl' => \&std_stl_handler,
    );

my %gcc_type_handlers = (
    'check' => \&gcc_check_handler,
    'done' => \&gcc_done_handler,
    'lib' => \&gcc_lib_handler,
    'sizeof' => \&gcc_sizeof_handler,
    'start' => \&gcc_start_handler,
    'stl' => \&gcc_stl_handler,
    );

my $type_handlers_per_style = {
    'gcc' => \%gcc_type_handlers,
    'std' => \%std_type_handlers,
};

my $in_components = undef;
my $input_file = undef;
my $out_components = undef;
my $out_gen_components = undef;
my $output_file = undef;
my $style = undef;

GetOptions ("in-components=s" => \$in_components,
            "input=s" => \$input_file,
            "out-components=s" => \$out_components,
            "out-gen-components=s" => \$out_gen_components,
            "output=s" => \$output_file,
            "style=s" => \$style) or die;

die unless (defined ($in_components));
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
my $include_input = File::Spec->catfile (split (',', $in_components), (File::Spec->splitpath ($input_file))[2]);
$out->say ("");
$out->say ("#include \"$include_input\"");
$out->close ();
$in->close ();
