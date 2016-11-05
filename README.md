JSONSTROLL(1)                    User Commands                   JSONSTROLL(1)

[![Build Status](http://b64.jenkins.knacki.info/job/jsonStroller/badge/icon)](http://b64.jenkins.knacki.info/job/jsonStroller/)

NAME
       jsonstroll  -  manual  page  for jsonstroll (jsonstroller suite) 1.0RC1
       generated on Nov  5 2016

SYNOPSIS
       jsonstroll [OPTIONS]
       jsonstroll [OPTIONS] FILENAME
       jsonstroll [OPTIONS] --INPUT
       jsonstroll --diff [OPTIONS] FILENAME FILENAME [FILENAME]

DESCRIPTION
       Read json input and print it using ncurse

       if not INPUT nor FILENAME, use standard input

       When output is redirected to a file or another command, output will  be
       prettyfied unless --compress

       FILENAME
              read input from filename instead of stdin

       INPUT  use this as input instead of stdin

       -W     consider continuing on non-blocking errors

       --ascii
              ignore unicode values

       --keep-order
              do not sort objects by key

       --color[=MODE]
              colorize  output,  MODE  can  be  never  or always (default when
              ommited)

       --compress
              if output is redirected, strip unnecessaries characters

       --diff-cmd COMMAND ;
              Execute command as external diff. All following argument will be
              passed  as  parameter  to  COMMAND until ';' is encountered. The
              '{}' argument will be replaced by the file path of  the  pretti‐
              fied input

       -v, -version
              display version information

       -h, --help
              show this message and exit

EXAMPLES
       jsonstroll f.json       Output f.json's content

       jsonstroll   --diff   fileA.json   fileB.json  compare  fileA.json  and
       fileB.json

       jsonstroll --diff-cmd vimdiff {} ';' fileA.json fileB.json      compare
       fileA.json and fileB.json

AUTHOR
       Written by isundil <isundill@gmail.com>.

REPORTING BUGS
       Report bugs to <isundill@gmail.com>

COPYRIGHT
       Copyright  ©  2016  Free Software Foundation, Inc.  License GPLv3+: GNU
       GPL version 3 or later <http://gnu.org/licenses/gpl.html>.
       This is free software: you are free  to  change  and  redistribute  it.
       There is NO WARRANTY, to the extent permitted by law.

SEE ALSO
       The  full  documentation for jsonstroll is maintained as a Texinfo man‐
       ual.  If the info and jsonstroll programs  are  properly  installed  at
       your site, the command

              info jsonstroll

       should give you access to the complete manual.



jsonstroll (jsonstroller suite) on Nov  5 2016  JSONSTROLL(1)
