#!/usr/bin/env perl
use strict;
use warnings;
use utf8;
use feature qw(say state);

BEGIN {
    $| = 1;
}

=pod

=head1 NAME

SweetLine::Demo - Perl sample for SweetLine

=head1 DESCRIPTION

This file exercises sigils, POD, regex operators, quote-like forms,
here-documents, packages, and common builtins.

=cut

package SweetLine::Demo;

our $VERSION = '1.0.0';

sub normalize_name {
    my ($name, $fallback) = @_;
    $fallback //= 'unknown';

    if (!defined $name || $name eq '') {
        return $fallback;
    }

    $name = lc $name;
    $name =~ s/^\s+|\s+$//g;
    $name =~ s/[^a-z0-9._-]+/_/g;
    $name =~ s/_+/_/g;
    return length($name) ? $name : $fallback;
}

sub clamp_value {
    my ($value, $min_value, $max_value) = @_;
    $min_value //= 0;
    $max_value //= 100;

    if ($value < $min_value) {
        return $min_value;
    }
    if ($value > $max_value) {
        return $max_value;
    }

    return $value;
}

sub summarize_values {
    my ($values, $label, %extra) = @_;
    $label //= 'series';

    my $count = scalar @$values;
    my $total = 0;
    $total += $_ for @$values;

    my $average = $count ? $total / $count : 0;
    my @sorted = sort { $a <=> $b } @$values;
    my $median = $count ? $sorted[int($count / 2)] : 0;

    return {
        label => normalize_name($label),
        count => $count,
        total => $total,
        average => $average,
        median => $median,
        extra => \%extra,
    };
}

sub render_report {
    my ($values, $title) = @_;
    $title //= 'report';

    my @lines;
    for my $index (0 .. $#$values) {
        push @lines, sprintf('%s[%d] = %s', $title, $index, $values->[$index]);
    }

    return join("\n", @lines);
}

sub exercise_operators {
    my ($name) = @_;

    my $literal = q{https://example.com/demo};
    my $interpolated = qq{Hello, $name};
    my $command = qx{printf "%s" "SweetLine"};
    my $regex = qr{^[A-Za-z_]\w*$};
    my @words = qw(alpha beta gamma delta);

    my $path = '  sample path.txt  ';
    $path =~ s/^\s+|\s+$//gr;
    my $upper = $path;
    $upper =~ tr/a-z/A-Z/;

    my $is_match = $name =~ $regex ? 1 : 0;
    my $replaced = $path =~ s/\s+/_/gr;

    return {
        literal => $literal,
        interpolated => $interpolated,
        command => $command,
        regex => $regex,
        words => \@words,
        upper => $upper,
        is_match => $is_match,
        replaced => $replaced,
    };
}

sub read_here_doc {
    my $text = <<'EOF';
This is a here-doc literal.
It can mention https://perldoc.perl.org and stay readable.
EOF

    return $text;
}

sub build_dataset {
    my @values = (1, 2, 3, 4, 5, 8, 13, 21, 34, 55);
    my @names = qw(alpha beta gamma delta epsilon);
    my %flags = (
        alpha => 1,
        beta => 0,
        gamma => 1,
        delta => 1,
        epsilon => 0,
    );

    my @filtered = grep { $_ % 2 } @values;
    my @mapped = map { $_ * 2 } @values;
    my @sorted = sort { $a <=> $b } @mapped;
    my $joined = join(", ", @names);

    for my $name (@names) {
        if ($flags{$name}) {
            say "enabled: $name";
        } else {
            say "disabled: $name";
        }
    }

    my $total = 0;
    my $index = 0;
    while ($index < @values) {
        $total += $values[$index];
        $index++;
        last if $total > 80;
    }

    return {
        values => \@values,
        names => \@names,
        flags => \%flags,
        filtered => \@filtered,
        mapped => \@mapped,
        sorted => \@sorted,
        joined => $joined,
        total => $total,
    };
}

sub demo_subroutines {
    my ($input) = @_;
    my $clean = normalize_name($input, 'fallback');
    my $clamped = clamp_value(length($clean), 3, 20);
    my $report = render_report([split //, $clean], 'chars');
    my $summary = summarize_values([1, 2, 3, 4, 5], 'numbers', source => 'example.pl');
    my $here = read_here_doc();
    my $ops = exercise_operators($clean);

    return {
        clean => $clean,
        clamped => $clamped,
        report => $report,
        summary => $summary,
        here => $here,
        ops => $ops,
    };
}

my $package_name = __PACKAGE__;
my $class_name = 'SweetLine::Demo';
my $demo_result = demo_subroutines("  Perl Demo  ");
my $package_ref = bless { name => $class_name }, $class_name;

if ($demo_result->{summary}->{count} > 0) {
    say "package: $package_name";
    say "class: $class_name";
    say "summary total: $demo_result->{summary}->{total}";
} elsif (!defined $demo_result) {
    die "unexpected undefined result";
} else {
    warn "demo result missing count";
}

my $regex_test = "SweetLine123" =~ qr{^[A-Za-z_]\w*$} ? 'yes' : 'no';
my $substitution = "alpha beta gamma";
$substitution =~ s/\s+/-/g;
my $translation = "abcdef";
$translation =~ tr/a-z/A-Z/;

say "regex_test = $regex_test";
say "substitution = $substitution";
say "translation = $translation";
say "command output = $demo_result->{ops}->{command}";
say "here-doc length = " . length($demo_result->{here});

END {
    say "goodbye from Perl";
}

package main;
SweetLine::Demo::demo_subroutines('main package');

1;
