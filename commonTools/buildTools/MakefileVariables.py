#! /usr/bin/env python

"""
MakefileVariables.py - This script can be run as a shell command or imported as
a module into a python script.  Its purpose is to read a Makefile and obtain a
map of make variables and their values.

Shell usage: MakefileVariables.py [options] [filename]

options:
    -h | --help       Print this message and exit
    -m | --make       Output in Makefile format
    -p | --python     Output as a python dictionary (default)
    -v | --version    Print the version number and exit

Python usage: import MakefileVariables

Available functions:

    joinContinuationLines([string, string, ...]) -> None
        Combine any strings that end with '\' with its following string.

    isNotBlankLine(string) -> bool
        Return True if string is not blank.

    parseMakefile(string) -> dict
        Open filename and obtain make variable names and their raw values.

    evaluate(string,dict) -> string
        Substitute variable values in dict for $(...) in string.

    makeSubstitutions(dict) -> None
        Interpret dict as varName/value pairs; wherever '$(...)' appears in
        values, make appropriate substitutions.

    specialNormPath(string) -> string
        Normalize a path, even if it starts with -I, -L, etc.

    uniqiufyList([string1, string2, ...]) -> None
        Remove duplicate strings from the list.

    uniquifyString(string) -> string
        Remove duplicate substrings from the string.

    uniquifyDict(dict) -> None
        Uniquify each value of the given dictionary.

    processMakefile(string) -> dict
        Given the name of a Makefile, parse the file for make variable names and
        values, make substitutions wherever '$(...)' is found in the values, and 
        then uniquify the resulting values.
"""

__version__ = "1.1"
__author__  = "Bill Spotz"
__date__    = "Sep 1 2005"

# Import python modules for command-line options, the operating system, regular
# expressions, and system functions
import commands
from   getopt import *
import os
import re
import sys

# Define regular expressions for Makefile assignments, blank line, continuation
# lines, include statements and variable references
assignRE   = re.compile(r"^\s*([A-Za-z_][A-Za-z_0-9]*)\s*=\s*(.*)$")
blankRE    = re.compile(r"^\s*$"                                   )
continueRE = re.compile(r"(.*)\\\s*$"                              )
includeRE  = re.compile(r"\s*include\s+(.+)"                       )
makeVarRE  = re.compile(r"\$\(([A-Za-z_][A-Za-z0-9_]*)\)"          )
shellRE    = re.compile(r"\$\(shell"                               )

#############################################################################

def findBlock(text, pos=0):
    """Given the input text (potentially multiline) and an optional pos marking
    the starting position, find an opening delimeter -- either (, [, {, single
    quote, or double quote -- and return a tuple of integers indicating the
    character indexes of the text block -- closed with ), ], }, single quote, or
    double quote, respectively -- while correctly handling nested blocks."""

    # Define delimeter strings
    quote1Delimeter = "'"
    quote2Delimeter = '"'
    openDelimeters  = "\(\[\{"
    closeDelimeters = "\)\]\}"

    # Define delimeter regular expressions
    quote1RE = re.compile("([" + quote1Delimeter + "])", re.M)
    quote2RE = re.compile("([" + quote2Delimeter + "])", re.M)
    openRE   = re.compile("([" + openDelimeters  +
                                 quote1Delimeter +
                                 quote2Delimeter + "])", re.M)
    anyRE    = re.compile("([" + openDelimeters  +
                                 quote1Delimeter +
                                 quote2Delimeter +
                                 closeDelimeters + "])", re.M)

    # Find the first opening delimeter
    matchObject = openRE.search(text, pos)
    if not matchObject: return (None, None)

    # Initialize the loop
    stack = [ matchObject.group() ]
    start = matchObject.start()
    pos   = start + 1

    # Find the end of the block
    while matchObject:

        # Determine the active delimeter regular expression
        if   stack[-1] == quote1Delimeter:
            activeRE = quote1RE
        elif stack[-1] == quote2Delimeter:
            activeRE = quote2RE
        else:
            activeRE = anyRE

        # Search for the next delimeter
        matchObject = activeRE.search(text, pos)
        if matchObject:
            delimeter = matchObject.group()
            pos       = matchObject.end()

            # Check for matched delimeters
            if (((stack[-1] == quote1Delimeter) and
                 (delimeter == quote1Delimeter)) or
                ((stack[-1] == quote2Delimeter) and
                 (delimeter == quote2Delimeter)) or
                ((stack[-1] == "("            ) and
                 (delimeter == ")"            )) or
                ((stack[-1] == "["            ) and
                 (delimeter == "]"            )) or
                ((stack[-1] == "{"            ) and
                 (delimeter == "}"            ))   ):
                stack.pop()                  # Remove the last element from the list
                if len(stack) == 0:
                    return (start, pos)

            # Process unmatched delimeter
            else:
                if (delimeter in openDelimeters  or
                    delimeter == quote1Delimeter or
                    delimeter == quote2Delimeter   ):
                    stack.append(delimeter)  # Add the delimeter to the stack
                else:
                    raise RuntimeError, "findBlock: mismatched delimeters: " + \
                          stack[-1] + " " + delimeter

    # We made it through all of text without finding the end of the block
    raise RuntimeError, "findBlock: open block: " + join(stack)

#############################################################################

def joinContinuationLines(lines):
    """Given lines, a list of strings, check for the continuation character
    ('\') at the end of each line.  If found, combine the appropriate strings
    into one, leaving blank lines in the list to avoid duplication."""
    for i in range(len(lines)-1):
        line = lines[i]
        match = continueRE.search(line)
        if match:
            lines[i+1] = match.group(1) + lines[i+1]
            lines[i  ] = ""

#############################################################################

def isNotBlankLine(s):
    """Return True if a string is not a blank line"""
    if blankRE.match(s):
        return False
    return True

#############################################################################

def parseMakefile(filename):
    """Open filename, read in the text and return a dictionary of make variable
    names and values.  If an include statement is found, this routine will be
    called recursively."""
    lines = open(filename,"r").readlines()     # Read in the lines of the Makefile
    lines = [s.split("#")[0] for s in lines]   # Remove all comments
    joinContinuationLines(lines)               # Remove continuation lines
    lines = filter(isNotBlankLine, lines)      # Remove all blank lines
    dict = { }
    for line in lines:
        # Process include statements
        match = includeRE.match(line)
        if match:
            files = evaluate(match.group(1),dict).split()
            for file in files:
                #print "Including", file
                try:
                    dict.update(parseMakefile(file))
                except IOError:
                    pass
            continue
        # Process assignment statements
        match = assignRE.match(line)
        if match:
            dict[match.group(1).strip()] = match.group(2).strip()
    return dict

#############################################################################

def evaluate(value, dict):
    """Evaluate the string 'value' by applying the following algorithm: if value
    contains substring '$(VARNAME)' and dict has key VARNAME, substitute
    dict[VARNAME] for the variable reference.  If VARNAME is not a key for the
    dictionary, substitute the null string.  If value contains substring
    '$(shell ...)', then call this routine recursively on the obtained command
    string.  If the resulting command string is unchanged, execute it as a shell
    command and substitutue the results.  Return the evaluated string."""

    # Initialize
    originalValue = value
    pos           = len(value)
    debug = "shell perl" in value
    #if debug: print "\n%s\n" % value

    # Evaluate $(VARNAME)
    match = makeVarRE.search(value)
    if match:
        subVarName = match.group(1)
        #if debug: print "\n%s\n" % subVarName
        start      = match.start(1)-2
        end        = match.end(1)  +1
        if subVarName in dict.keys():
            subValue = dict[subVarName]
        else:
            subValue = ""
        value = value[:start] + subValue + value[end:]
    else:

        # Evaluate $(shell ...)
        match = shellRE.search(value)
        if match:
            # The shellRE only matches the opening '$(shell'.  We need to find
            # the closing ')', accounting for internal parenthetical or quoted
            # blocks.
            (start,end) = findBlock(value,match.start())
            start      -= 1
            shellCmd    = value[start+8:end-1]
            newShellCmd = evaluate(shellCmd,dict)
            while newShellCmd != shellCmd:
                shellCmd    = newShellCmd
                newShellCmd = evaluate(shellCmd,dict)
            (status,subValue) = commands.getstatusoutput(shellCmd)
            if status:
                print >>sys.stderr, "WARNING: %s gives\n%s" % (shellCmd,
                                                               subValue)
                subValue = ""
            value = value[:start] + subValue + value[end:]

    # Are we done?
    if value == originalValue:
        return value
    else:
        return evaluate(value,dict)

#############################################################################

def makeSubstitutions(dict):
    """Loop over the items of a dictionary of variable names and string values.
    If the value contains the substring(s) '$(VARNAME)' and VARNAME is a key in
    the dictionary, then substitute the appropriate value.  If VARNAME is not a
    key in the dictionary, then substitute the null string.  For circular
    substitutions, substitute the null string."""
    for varName in dict:
        value = dict[varName]
        #if "EPETRAEXT_INCLUDES" == varName:
        #    print "\n%s =\n%s\n" % (varName, value)
        dict[varName] = evaluate(value,dict)
        #if "EPETRAEXT_INCLUDES" == varName:
        #    print "\n%s =\n%s\n" % (varName, dict[varName])
        #    sys.exit()

#############################################################################

def specialNormPath(path):
    """Apply os.path.normpath to argument path, but remove a leading option
    such as '-I' or '-L' before the call and add it back before the result is
    returned."""
    if len(path) <= 2: return path
    start = 0
    if path[0] == "-": start = 2
    return path[:start] + os.path.normpath(path[start:])

#############################################################################

def uniquifyList(list):
    """Remove duplicate items from a list, preserving original order."""
    i = 1
    while i < len(list):
        if list[i] in list[:i]:
            del list[i]
        else:
            i += 1

#############################################################################

def uniquifyString(s, delim=' '):
    """Split a string using the specified delimeter, apply specialNormPath to
    each string, uniquify the resulting list, and return a string that joins the
    unique list with the same delimeter."""
    list = s.split(delim)
    list = [specialNormPath(path.strip()) for path in list]
    uniquifyList(list)
    return delim.join(list)

#############################################################################

def uniquifyDict(dict):
    """Loop over each item in a dictionary of variable names and string values
    and uniquify the string."""
    for key in dict:
        dict[key] = uniquifyString(dict[key])

#############################################################################

def processMakefile(filename):
    """Open filename, read its contents and parse it for Makefile assignments,
    creating a dictionary of variable names and string values.  Substitute
    variable values when '$(...)' appears in a string value."""
    # We want to change directories to the directory that contains the
    # Makefile.  Then, when we make substitutions, any $(shell ...) expansions
    # will be done from the assumed directory.
    cwd = os.getcwd()
    (path,name) = os.path.split(filename)
    if path: os.chdir(path)

    # Parse and substitute
    dict = parseMakefile(name)
    makeSubstitutions(dict)

    # Change directory back to prevent confusion
    os.chdir(cwd)

    # Remove duplicates from the dictionary values and return the dictionary
    uniquifyDict(dict)
    return dict

#############################################################################

def main():
    """This is the routine that gets called if MakefileVariable.py is invoked
    from the shell.  Process any command-line options that may be given, take
    the first argument as a filename, process it to obtain a dictionary of
    variable name/value pairs, and output the results."""

    # Initialization
    (progDir,progName) = os.path.split(sys.argv[0])
    options      = "hmpv"
    long_options = ["help", "make", "python", "version"]
    outStyle     = "python"

    # Get the options and arguemnts from the command line
    (opts,args) = getopt(sys.argv[1:], options, long_options)

    # Loop over options and implement
    for flag in opts:
        if flag[0] in ("-h","--help"):
            print __doc__
            sys.exit()
        elif flag[0] in ("-m", "--make"):
            outStyle = "make"
        elif flag[0] in ("-p", "--python"):
            outStyle = "python"
        elif flag[0] in ("-v", "--version"):
            print progName, __version__, __date__
            sys.exit()
        else:
            print "Unrecognized flag:", flag[0]
            print __doc__
            sys.exit()

    # Process the filename
    dict = processMakefile(args[0])

    # Output the variable names and values
    if outStyle == "make":
        keys = dict.keys()
        keys.sort()
        for key in keys:
            print key, "=", dict[key]
    elif outStyle == "python":
        print dict

#############################################################################
# If called from the command line, call main()
#############################################################################

if __name__ == "__main__":
    main()
