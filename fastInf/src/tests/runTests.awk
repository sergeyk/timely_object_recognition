#! awk -f
# ARGV[1]: tests file
# ARGV[2]: 0 or empty: only echo
#        : 1 run
#        : 2 create baseline
# ARGV[3]: name of specific test (optional)


BEGIN {
  TEST = "";
  if ( ARGC == 4 ) {
    TEST = ARGV[3];
    ARGV[3] = "";
    ARGC--;
  }
  if ( ARGC == 3 ) {
    METHOD = ARGV[2];
    ARGV[2] = "";
    ARGC--;  
  } else
    METHOD == 0;

  if ( METHOD!=1 && METHOD!=2 ) {
    print;
    print "To run use: runTests.awk test.list 1";
    print;
  }

  pCmd = "pwd";
  pCmd | getline CDIR;
  ODIR = CDIR "/Outputs";  # where base outputs are kept
  IDIR = CDIR "/Inputs"; # where inputs are kept

  if ( METHOD == 2 ) {
      system("mkdir -p " ODIR);
      OUTDIR = ODIR;
  } else {
    nCmd = "whoami";
    nCmd | getline NAME;
    OUTDIR = "/tmp/fastInf_" NAME;
    system("mkdir -p " OUTDIR);
    system("chmod -R 775 " OUTDIR)
  }
  ClearVals();
}

($1 == "#") {
}

# go over tests
($0 == "<test>" ) {
  ClearVals();
  inTest = 1;
}

($1 == "execute") {
  if ( inTest==0 || $2!="=" || NF!=3 ) {
    print "Error: format problem - " $0;
    exit;
  }
  execute = $3;
}

($1 == "name") {
  if ( inTest==0 || $2!="=" || NF!=3 ) {
    print "Error: format problem - " $0;
    exit;
  }
  name = $3;
}

($1 == "command") {
  if ( inTest==0 || $2!="=" || NF!=3 ) {
    print "Error: format problem - " $0;
    exit;
  }
  command = $3;
}

($1 == "params") {
  if ( inTest==0 || $2!="=" ) {
    print "Error: format problem - " $0;
    exit;
  }
  $1="";
  $2="";
  params = $0;
  gsub(/OUTDIR/,OUTDIR,params);
}

($1 == "outfiles") {
  if ( inTest==0 || $2!="=" ) {
    print "Error: format problem - " $0;
    exit;
  }
  $1="";
  $2="";
  for ( i=3 ; i<=NF ; i++ ) {
    outfiles[i-2] = $i;
    outfileNum++;
  }
}

($0 == "<end test>") {
  if ( name=="" || command=="" || params=="" ) {
    print "Error: missing test parameters";
    exit;
  }
  
  RunTest();
}

function RunTest()
{
  if ( TEST!="" && TEST!=name )
    return;

  if ( execute != "true" && TEST=="" ) 
    return;

  cmd = command " " params " >& " OUTDIR "/" name ".log";
#  if ( METHOD!=2 || system("test -r " OUTDIR "/" name ".log") ) {
    print " ";
    print "**** Test " name ": Executing ****";
    print "  - COMMAND: " cmd;

    if ( METHOD != 0 ) { # actually run
      system("cd " IDIR "; rm -f " OUTDIR "/" name ".log; " cmd "; cd " CDIR);
      if ( METHOD == 1 ) { # check against baseline
	outfiles[0] = name ".log";
        # go over files and diff each one
	for ( f=0 ; f<=outfileNum ; f++ ) { 
	  diffFile = OUTDIR "/" outfiles[f] ".diff";
	  system("rm -f " diffFile);
	  system("diff -b " OUTDIR "/" outfiles[f] " " ODIR "/" outfiles[f] " >& " diffFile);
	  wcCmd = "wc -l " diffFile " | awk '{print $1;}'";
	  wcCmd | getline line;
	  close(wcCmd);
	  if ( line != "0" ) {
            print " ";
	    print "  !!!! FILE DIFF: " outfiles[f] " !!!!";
	    while ( (getline line < diffFile) > 0 )
	      print line;
	    print "  !!!!";
	    print " ";
	  } else {
	    print "  - FILE OK: " outfiles[f] " -";
	    system("rm -f " OUTDIR "/" outfiles[f]);
	  }
	  system("rm -f " diffFile);
	} # over files
      } # diff method
    } # not just echo
    print "**** Test " name ": Done ****";
#  } // there is something to test
}

function ClearVals()
{
  name = "";
  params = "";
  inTest = 0;
  execute = true;
  cmd = "";
  for ( i in outfiles ) 
    delete outfile[i];
  outfileNum = 0;
}

END {
}
