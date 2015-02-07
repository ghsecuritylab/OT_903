#!/usr/bin/perl
use File::Find;
use File::Basename;
use IO::File;
use Cwd;
use File::Copy;
use Getopt::Long;
use File::Path;
use File::Copy;

$draftHeaderName = "udraft.h";
$draftAppend = "DRAFT_API_DO_NOT_USE";
$draftDefine = "U_HIDE_DRAFT_API";

$deprecatedHeaderName = "udeprctd.h";
$deprecatedAppend = "DEPRECATED_API_DO_NOT_USE";
$deprecatedDefine = "U_HIDE_DEPRECATED_API";

$obsoleteHeaderName = "uobslete.h";
$obsoleteAppend = "OBSOLETE_API_DO_NOT_USE";
$obsoleteDefine = "U_HIDE_OBSOLETE_API";

$systemHeaderName = "usystem.h";
$systemAppend = "SYSTEM_API_DO_NOT_USE";
$systemDefine = "U_HIDE_SYSTEM_API";

$internalHeaderName = "uintrnal.h";
$internalAppend = "INTERNAL_API_DO_NOT_USE";
$internalDefine = "U_HIDE_INTERNAL_API";

$versionAppend="";

#run the program
main();

#---------------------------------------------------------------------
# The main program

sub main(){
  GetOptions(
           "--srcdir=s" => \$srcDir,
           "--destdir=s" => \$destDir,
           "--version=s"  => \$version,
           "--exclusion-list=s"  => \$exclude,
           "--include-types" => \$includeTypes,
           "--verbose" => \$verbose
           );
  usage() unless defined $srcDir;
  usage() unless defined $destDir;
  usage() unless defined $version;
  usage() unless defined $exclude;
  $draftFile      = "$srcDir/draft.html";
  $deprecatedFile = "$srcDir/deprecated.html";
  $obsoleteFile   = "$srcDir/obsolete.html";
  $systemFile     = "$srcDir/system.html";
  $internalFile   = "$srcDir/internal.html";
  
  $versionAppend = $version;
  $versionAppend=~ s/^([0-9]+)\.([0-9]+).*/\1_\2/;
  $excludeFH = IO::File->new($exclude,"r")
            or die  "could not open the file $exclude for reading: $! \n";
  my %exclude;
  while (defined ($line = <$excludeFH>)){
    next if( $line =~ /^#/);
    $line =~ s/^\s+//;
    $line =~ s/\s+\n$//;
    $exclude{$line}="EXCLUDE";
  }    

  writeFile($draftFile, $draftHeaderName, $destDir, $draftAppend, $draftDefine, \%exclude);
  writeFile($deprecatedFile, $deprecatedHeaderName, $destDir, $deprecatedAppend, $deprecatedDefine, \%exclude);
  writeFile($obsoleteFile, $obsoleteHeaderName, $destDir, $obsoleteAppend, $obsoleteDefine, \%exclude);
  writeFile($systemFile, $systemHeaderName, $destDir, $systemAppend, $systemDefine, \%exclude);
  writeFile($internalFile, $internalHeaderName, $destDir, $internalAppend, $internalDefine, \%exclude);
}

#-----------------------------------------------------------------------
sub getHeaderDef{
    ($headerName) = @_;
    $headerDef = uc($headerName);  # this is building the constant for #define
    $headerDef =~ s/\./_/;
    return $headerDef;
}

#-----------------------------------------------------------------------
sub writeFile{
  ($infile,$outfile,$destDir, $symbolAppend, $symbolDef, $exclude) = @_;

  my $outFileName = $outfile;
  $headerDef = getHeaderDef($outfile);
  
  $outfile = $destDir."/".$outfile;

  $inFH = IO::File->new($infile,"r")
            or die  "could not open the file $infile for reading: $! \n";
  $outFH = IO::File->new($outfile,"w")
            or die  "could not open the file $outfile for writing: $! \n";

  #print "$headerDef>>> $outfile\n";
  printHeader($outFH, $outFileName, $headerDef, $symbolDef);
  parseWriteFile($inFH, $outFH, $symbolAppend, $exclude);
  printFooter($outFH, $headerDef, $symbolDef);
  close($inFH);
  close($outFH);
}

#-----------------------------------------------------------------------
sub printHeader{
    ($outFH, $headername, $HEADERDEF, $symbolDef) = @_;
    ($DAY, $MONTH, $YEAR) = (localtime)[3,4,5];
    $YEAR += 1900;
#We will print our copyright here + warnings
print $outFH <<END_HEADER_COMMENT;

#ifndef $HEADERDEF
#define $HEADERDEF

#ifdef $symbolDef

END_HEADER_COMMENT
}

#-----------------------------------------------------------------------
sub parseWriteFile{
    ($inFH, $outFH, $symbolAppend, $exclude) = @_;
    %disableRenaming;
    %enableRenaning;
    while (defined ($line = <$inFH>)){
          #just process C APIs for now
        if($line =~ /\<dt\>/ ){
            #special cases
            if( ($line =~ /LEUnicode/)|| ($line =~ /LanguageCodes/) ||
                ($line =~ /ScriptCodes/) || ($line =~ /\:+/) || 
                ($line =~ /Class/) ){
                next;
            }
            if( $line =~ /^\<dt\>File [^\>]*\>([^\<]*)/ ) {
                print "Skipping file-scope $symbolAppend $1\n";
                next;
            }
            #<dt>Global <a class="el" href="utrans_8h.html#a21">utrans_unregister</a>  </dt>
            #<dt>Global <a class="el" href="classUnicodeString.html#w1w0">UnicodeString::kInvariant</a>  </dt>
            # the below regular expression works for both the above formats.
            $line=~ m/\<dt\>.*\<a class=\".*\" href=\".*\">(.*)\<\/dt\>/;
            my $value = $1;
            $value =~ s/\<\/a\>\s*//g;
            $value =~ s/^\s+//;
            $value =~ s/\s+$//;
            #print "$exclude->{$value}\n";
            if($exclude->{$value} eq "EXCLUDE"){
                #print "$value  $exclude->{$value}\n";
                next;
            }
            print  "$value $realSymbol $nonExSymbol :: $line\n" if defined $verbose;
            next if(isStringAcceptable($value)==1);
            if($value =~ /^operator[^a-zA-Z]/) {
                print "Skipping operator $symbolAppend $value from $line\n";
                next;
            }
            $realSymbol = $value."_".$versionAppend;
            $nonExSymbol = $value."_".$symbolAppend;
            $disableRenaming{$value} = $nonExSymbol;
            $enableRenaming{$realSymbol} = $nonExSymbol;
            print  "$value $realSymbol $nonExSymbol\n" if defined $verbose;
            
        }
    }
    print "size of disableRenaming:  " . keys( %disableRenaming) . ".\n";
    print "size of enableRenaming:  " . keys( %enableRenaming) . ".\n";
    print $outFH "#    if U_DISABLE_RENAMING\n";
    foreach $key (sort (keys(%disableRenaming))) {
       print $outFH "#        define $key $disableRenaming{$key}\n";
       delete($disableRenaming{$key});
    }
    print $outFH "#    else\n";
    foreach $key (sort (keys(%enableRenaming))) {
       print $outFH "#        define $key $enableRenaming{$key}\n";
       delete($enableRenaming{$key});
    }
    print $outFH "#    endif /* U_DISABLE_RENAMING */\n";
}
#-----------------------------------------------------------------------
sub isStringAcceptable{
    ($string) = @_;
    @str = split(//, $string);
    $ret = 1;
    foreach  $val (@str){
        if(($val ne "_") && !($val =~ /[0-9A-Z]/)){
        #print "$val\n";
            $ret = 0;
        }
    }
    #print "$string : $ret\n";
    if(!(defined $includeTypes)){
        if($ret==0 && $str[0] eq 'U'){
            $ret=1;
        }
    }
    return $ret;
}

#-----------------------------------------------------------------------
sub printFooter{

      ($outFH, $headerDef, $symbolDef ) = @_;
#print the footer
print $outFH <<END_FOOTER;

#endif /* $symbolDef */
#endif /* $headerDef */

END_FOOTER
}
#-----------------------------------------------------------------------
sub usage {
    print << "END";
Usage:
gendraft.pl
Options:
        --srcdir=<directory>
        --destdir=<directory>
        --version=<current version of ICU>
        --exclusion-list=<file name>
        --include-types
e.g.: genheaders.pl  --srcdir=<icu>/source/common/docs/html --destdir=<icu>/source/common/unicode --version=2.8 --exclusion-list=exclude.txt
END
  exit(0);
}
