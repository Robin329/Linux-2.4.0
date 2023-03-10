#!/usr/bin/perl

$OBJDUMP=$ARGV[0];

sub swapdata {
	local ($num) = @_;

	return substr($num, 6, 2).substr($num, 4, 2).substr ($num, 2, 2).substr ($num, 0, 2);
}

open (DATA, $OBJDUMP.' --full-contents --section=.data getconsdata.o | grep \'^ 00\' |') ||
	die ('Cant objdump!');
while (<DATA>) {
	($addr, $data0, $data1, $data2, $data3) = split (' ');
	$dat[hex($addr)] = hex(&swapdata($data0));
	$dat[hex($addr)+4] = hex(&swapdata($data1));
	$dat[hex($addr)+8] = hex(&swapdata($data2));
	$dat[hex($addr)+12] = hex(&swapdata($data3));
}
close (DATA);

open (DATA, $OBJDUMP.' --syms getconsdata.o |') || die ('Cant objdump!');
while (<DATA>) {
	/elf32/ && ( $elf = 1 );
	/a.out/ && ( $aout = 1 );
	next if ($aout && ! / 07 /);
	next if ($elf && ! (/^0*0...... g/ && /.data/));
	next if (!$aout && !$elf);

	if ($aout) {
		($addr, $flags, $sect, $a1, $a2, $a3, $name) = split (' ');
		$nam[hex($addr)] = substr($name, 1);
	}
	if ($elf) {
		chomp;
		$addr = substr ($_, 0, index($_, " "));
		$name = substr ($_, rindex($_, " ") + 1);
		$nam[hex($addr)] = $name;
	}
}
close (DATA);

print "/*\n * *** This file is automatically generated from getconsdata.c.  Do not edit! ***\n */\n";
for ($i = 0; $i < hex($addr)+4; $i += 4) {
	print "#define $nam[$i] $dat[$i]\n";
}
