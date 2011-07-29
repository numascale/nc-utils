#!/usr/bin/perl
#
# Id: $Id$
#
# Copyright © 2008 Numascale AS, Oslo, Norway
# Author: Arne Georg Gleditsch
#
# Distribution restricted.
#

use strict;
use re 'eval';
use Getopt::Long;

my $balanced;
$balanced = qr/
        (?: 
           (?> [^{}]* )
         | 
           { (??{$balanced}) }
        )*
    /x;


sub expandmacro {
    my ($tmpl, $args) = @_;

    my $subst = $$tmpl{'subst'};
    my $re = '\#('.join('|', 1..$$tmpl{'args'}).')\b';
    $subst =~ s/$re/$$args[$1-1]/gse;

    return $subst;
}


sub match_groups {
    my ($str) = @_;
    
    my @g = map { substr($$str, $-[$_], $+[$_] - $-[$_]) } 1 .. $#+;
    return \@g;
}


sub parse_doc {
    my @regs;
    my $reg;
    my $field;
    my %ids;
    my %macro;

    my $text = join('', <>);

    $text =~ s(\\newcommand{(\\[^}]+)}\[(\d)\]{($balanced)})
	($macro{$1} = {'args' => $2, 'subst' => $3}; "")gse;
    for my $m (keys %macro) {
	my $re = qr/\Q$m\E/.
	    join('', (qr/{($balanced)}/) x $macro{$m}{'args'});
	$text =~ s($re)(expandmacro($macro{$m}, match_groups(\$text)))ge;
    }

    my @lines = split(/\n/, $text);
    for (@lines) {
	s/#(.*)//;

	if (/\\textbf{([FG]\d)x(\[.*?\]|\S+\s*-\s*\1x[0-9A-F]+|\S+)\s*([^}]*)/) {
	    my ($fun, $idx, $desc) = ($1, $2, $3);
	    undef $field;
	    next unless $desc =~ /\S/;

	    my $id;
	    $desc =~ s/\\(.)/$1/g;
	    if ($desc =~ s/^\s*(\S+_\S+)\s*//) {
		$id = uc($1);
	    }
	    else {
		$id = uc($desc);
	    }
	    $id =~ s/[^A-Z0-9_ ]+/ /g;
	    $id =~ s/ +/_/g;
	    $id =~ s/^_|_$//g;

	    if ($idx =~ /^\[(.*)\]$/) {
		$idx = [reverse(split(/, */, $1))];
	    }
	    elsif ($idx =~ /(\S+)\s*-\s*[FG]\dx(\S+)/) {
		$idx = [map { sprintf("%x", $_*4) } hex($1)/4..hex($2)/4];
	    }

	    $reg = {'idx' => $idx,
		    'id' => "CSR_${fun}_".$id,
		    'name' => $id,
		    'fun' => $fun,
		    'desc' => $desc,
		    'fields' => [],
		    'mask' => 0xFFFF_FFFF,
		    'text' => ''};
	    push(@regs, $reg);
	}
	elsif (/Reset: ([0-9A-F]{4})[_ ]?([0-9A-F]{4})/) {
	    $$reg{'reset'} = $1.$2;
	}
	elsif (/(\d+(?::\d+|))\s*\&\s*(RW|RO|WO)\s*\&\s*(.*?)\\\\/) {
	    $field = {
		'range' => $1,
		'access' => $2,
		'id' => lc($3),
		'desc' => $3,
		'text' => ''};
	    $$field{'id'} =~ s/\\textbf{(.*)}.*|.*?(\S+).*/$1$2/g;
	    $$field{'id'} =~ s/\\(.)/$1/g;
	    $$field{'id'} =~ s/[^a-z0-9 ]/ /g;
	    $$field{'id'} =~ s/^ +| +$//g;
	    $$field{'id'} =~ s/ +/_/g;
	    $$field{'id'} =~ s/^reserved$//;
	    $$field{'id'} =~ s/^(.+)/_$1/;
	    push(@{$$reg{'fields'}}, $field);
	    my @range = split(/:/, $$field{'range'});
	    push(@range, @range) unless @range > 1;
	    foreach my $i ($range[1]..$range[0]) {
		$$reg{'mask'} ^= (1 << $i) if $$field{'access'} eq 'RW';
	    }
	}
	else {
#	    undef $field unless /\S/;
#	    if ($field) {
#		$$field{'text'} .= $_;
#	    }
#	    else {
#		$$reg{'text'} .= $_;
#	    }
	}
    }
    return \@regs;
}

sub string_lookup {
    my ($fun, $idx, $name) = @_;

    if ($fun =~ /^G(\d+)/) {
	return "csr[$1][0x$idx/4] = \"$name\"";
    }
    elsif ($fun =~ /^F(\d+)/) {
	return "cfg[$1][0x$idx/4] = \"$name\"";
    }
    return '';
}

sub write_headers {
    my ($regs) = @_;

    my %reset;
    my %mask;
    my %structseen;
    my @str_init;

    foreach my $reg (@$regs) {
	my $offs = '';
	$offs = "+($1<<12)" if $$reg{'fun'} =~ /^G(\d)/;
	if (ref($$reg{'idx'}) eq 'ARRAY') {
	    my $id = $$reg{'id'};
	    $id =~ s/S$//;
	    my $name = $$reg{'name'};
	    $name =~ s/S$//;
	    foreach my $i (0 .. $#{$$reg{'idx'}}) {
		print("#undef H2S_${id}_$i\n");
		print("#define H2S_${id}_$i (0x$$reg{idx}[$i]$offs)\n");
		push(@{$reset{$$reg{'fun'} ||= []}},
		     "(a)[0x$$reg{'idx'}[$i]/4] = 0x$$reg{reset}")
		    if $$reg{'reset'};
		push(@{$mask{$$reg{'fun'} ||= []}},
		     sprintf("(a)[0x$$reg{'idx'}[$i]/4] = 0x%08X", $$reg{'mask'}))
		    if $$reg{'mask'};
		push(@str_init,
		     string_lookup($$reg{'fun'}, $$reg{'idx'}[$i], "${name}_$i"));
	    }
	}
	else {
	    print("#undef H2S_$$reg{id}\n");
	    print("#define H2S_$$reg{id} (0x$$reg{idx}$offs)\n");
	    push(@{$reset{$$reg{'fun'} ||= []}},
		 "(a)[0x$$reg{'idx'}/4] = 0x$$reg{reset}")
		if $$reg{'reset'};
	    push(@{$mask{$$reg{'fun'} ||= []}},
		 sprintf("(a)[0x$$reg{'idx'}/4] = 0x%08X", $$reg{'mask'}))
		if $$reg{'mask'};
	    push(@str_init,
		 string_lookup($$reg{'fun'}, $$reg{'idx'}, $$reg{'name'}));
	}
	
	next if $structseen{$$reg{'id'}};
	$structseen{$$reg{'id'}} = 1;
	my @fields;
	foreach my $f (@{$$reg{'fields'}}) {
	    push(@fields, [$$f{'id'}, split(/:/, $$f{'range'})])
		if defined($$f{'id'});
	}
	
	if (@fields) {
	    print("#ifndef __ASSEMBLER__\n",
		  "struct dnc_".lc($$reg{'id'})." {\n");
	    my %fieldseen;
	    my $curr = 0;
	    foreach my $f (sort { $$a[1] <=> $$b[1] } @fields) {
		push(@$f, $$f[1]) unless @$f > 2;

		print("\tunsigned :".($$f[2]-$curr).";\n")
		    if $curr < $$f[2];

		$curr = $$f[1]+1;
		my $id = $$f[0];
		$id = '' if $fieldseen{$id}++;
		print("\tunsigned $id:".($curr-$$f[2]).";\n");
	    }
	    print("\tunsigned :".(32-$curr).";\n")
		if $curr < 32;

	    print("};\n",
		  "#endif\n");
	}
    }
    foreach my $f (keys %reset) {
	print("#define H2S_RESET_CSR_$f(a) ".join(";\\\n", @{$reset{$f}})."\n");
    }
    foreach my $f (keys %mask) {
	print("#define H2S_MASK_CSR_$f(a) ".join(";\\\n", @{$mask{$f}})."\n");
    }
    print("#define NUMACHIP_STR_INIT_AUTO(cfg, csr) ".join(";\\\n", @str_init)."\n");
}

my $registers = parse_doc();
write_headers($registers);
