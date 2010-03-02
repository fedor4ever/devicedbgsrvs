#
# Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description: 
#

// html_file_copier.pl
//
// Copyright (c) 2007 - 2008 Symbian Software Ltd.  All rights reserved.
//

use warnings;
use strict;
use File::Copy;

my $resDir = `testdriver resultpath`;
chomp $resDir;
print "$resDir\n";
if($resDir=~/^(.*testdriver\\[^\\]+\\armv5\\udeb\\)\d+\\$/)
	{
	print "$1\n";
	my $prevDir = $1;
	my @files = <$prevDir*>;
	foreach my $file (@files)
		{
		print "$file\n";
		if(-d $file)
			{
			print "Setting resDir: $resDir\n";
			$resDir = $file . "\\";
			next;
			}
		}
	}

if($resDir=~/^(.*testdriver\\)([^\\]+)\\armv5\\udeb\\(\d+)\\$/)
	{
	my $outputDir = $1;
	my $build = $2;
	my $run = $3;
	my $input = "$resDir" . "run$run\_$build.html";
	print "Input: $input\n";
	my $output = "$outputDir". "out.html";
	print "Output: $output\n";
	if(!copy($input, $output))
		{
		print "Couldn't copy results";
		exit (2);
		}
	}
else
	{
	print "Couldn't find results";
	exit(1);
	}
