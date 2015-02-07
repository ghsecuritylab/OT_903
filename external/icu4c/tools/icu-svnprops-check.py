#! /usr/bin/python
import sys
import os
import os.path
import re
import getopt

#
#  svn autoprops definitions.
#      Copy and paste here the ICU recommended auto-props from
#      http://icu-project.org/docs/subversion_howto/index.html
#
#  This program will parse this autoprops string, and verify that files in
#  the repository have the recommeded properties set.
#
svn_auto_props = """
### Section for configuring automatic properties.
[auto-props]
### The format of the entries is:
###   file-name-pattern = propname[=value][;propname[=value]...]
### The file-name-pattern can contain wildcards (such as '*' and
### '?').  All entries which match will be applied to the file.
### Note that auto-props functionality must be enabled, which
### is typically done by setting the 'enable-auto-props' option.
*.c = svn:eol-style=native
*.cc = svn:eol-style=native
*.cpp = svn:eol-style=native
*.h = svn:eol-style=native
*.rc = svn:eol-style=native
*.dsp = svn:eol-style=native
*.dsw = svn:eol-style=native
*.sln = svn:eol-style=native
*.vcproj = svn:eol-style=native
configure = svn:eol-style=native;svn:executable
*.sh = svn:eol-style=native;svn:executable
*.pl = svn:eol-style=native;svn:executable
*.py = svn:eol-style=native;svn:executable
*.txt = svn:mime-type=text/plain;svn:eol-style=native
*.java = svn:eol-style=native
*.ucm = svn:eol-style=native
*.html = svn:eol-style=native;svn:mime-type=text/html
*.htm = svn:eol-style=native;svn:mime-type=text/html
*.xml = svn:eol-style=native
Makefile = svn:eol-style=native
*.in = svn:eol-style=native
*.mak = svn:eol-style=native
*.mk = svn:eol-style=native
*.png = svn:mime-type=image/png
*.jpeg = svn:mime-type=image/jpeg
*.jpg = svn:mime-type=image/jpeg
*.bin = svn:mime-type=application/octet-stream
*.brk = svn:mime-type=application/octet-stream
*.cnv = svn:mime-type=application/octet-stream
*.dat = svn:mime-type=application/octet-stream
*.icu = svn:mime-type=application/octet-stream
*.res = svn:mime-type=application/octet-stream
*.spp = svn:mime-type=application/octet-stream
# new additions 2007-dec-5 srl
*.rtf = mime-type=text/rtf
*.pdf = mime-type=application/pdf
# changed 2008-04-08: modified .txt, above, adding mime-type
"""


# file_types:  The parsed form of the svn auto-props specification.
#              A list of file types - .cc, .cpp, .txt, etc.
#              each element is a [type, proplist]
#              "type" is a regular expression string that will match a file name
#              prop list is another list, one element per property.
#              Each property item is a two element list, [prop name, prop value]
file_types = list()

def parse_auto_props():
    aprops = svn_auto_props.splitlines()
    for propline in aprops:
        if re.match("\s*(#.*)?$", propline):         # Match comment and blank lines
            continue
        if re.match("\s*\[auto-props\]", propline):  # Match the [auto-props] line.
            continue
        if not re.match("\s*[^\s]+\s*=", propline):  # minimal syntax check for <file-type> =
            print "Bad line from autoprops definitions: " + propline
            continue
        file_type, string_proplist = propline.split("=", 1)

        #transform the file type expression from autoprops into a normal regular expression.
        #  e.g.  "*.cpp"  ==>  ".*\.cpp$"
        file_type = file_type.strip()
        file_type = file_type.replace(".", "\.")
        file_type = file_type.replace("*", ".*")
        file_type = file_type + "$"

        # example string_proplist at this point: " svn:eol-style=native;svn:executable"
        string_proplist = string_proplist.split(";")
        proplist = list()
        for prop in string_proplist:
            if prop.find("=") >= 0:
                prop_name, prop_val = prop.split("=")
            else:
                # properties with no explicit value, e.g. svn:executable
                prop_name, prop_val = prop, ""
            prop_name = prop_name.strip()
            prop_val = prop_val.strip()
            proplist.append((prop_name, prop_val))

        file_types.append((file_type, proplist))
    # print file_types

        
def runCommand(cmd):
    output_file = os.popen(cmd);
    output_text = output_file.read();
    exit_status = output_file.close();
    if exit_status:
        print >>sys.stderr, '"', cmd, '" failed.  Exiting.'
        sys.exit(exit_status)
    return output_text


def usage():
    print "usage: " + sys.argv[0] + " [-f | --fix] [-h | --help]"

    
def main(argv):
    fix_problems = False;
    try:
        opts, args = getopt.getopt(argv, "fh", ("fix", "help"))
    except getopt.GetoptError:
        print "unrecognized option: " + argv[0]
        usage()
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("-h", "--help"):
            usage()
            sys.exit()
        if opt in ("-f", "--fix"):
            fix_problems = True
    if args:
        print "unexpected command line argument"
        usage()
        sys.exit()

    parse_auto_props()
    output = runCommand("svn ls -R ");
    file_list = output.splitlines()

    for f in file_list:
        if os.path.isdir(f):
            # print "Skipping dir " + f
            continue
        if not os.path.isfile(f):
            print "Repository file not in working copy: " + f
            continue;

        for file_pattern, props in file_types:
            if re.match(file_pattern, f):
                # print "doing " + f
                for propname, propval in props:
                    actual_propval = runCommand("svn propget --strict " + propname + " " + f)
                    #print propname + ": " + actual_propval
                    if not (propval == actual_propval or (propval == "" and actual_propval == "*")):
                        print "svn propset %s %s %s" % (propname, propval, f)
                        if fix_problems:
                            os.system("svn propset %s %s %s" % (propname, propval, f))
                    if propname == "svn:eol-style" and propval == "native":
                        if os.system("grep -q -v \r " + f):
                            if fix_problems:
                                print f + ": Removing DOS CR characters."
                                os.system("sed -i s/\r// " + f);
                            else:
                                print f + " contains DOS CR characters."

if __name__ == "__main__":
    main(sys.argv[1:])
