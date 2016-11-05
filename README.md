# JsonStroller


[![Build Status](http://b64.jenkins.knacki.info/job/jsonStroller/badge/icon)](http://b64.jenkins.knacki.info/job/jsonStroller/)

## NAME
jsonstroll  -  manual  page  for jsonstroll (jsonstroller suite) 1.2 generated on Nov  5 2016

## SYNOPSIS
    jsonstroll [OPTIONS]
    jsonstroll [OPTIONS] FILENAME
    jsonstroll [OPTIONS] -- INPUT
    jsonstroll --diff [OPTIONS] FILENAME FILENAME

## DESCRIPTION
Read json input and print it using ncurse

if not INPUT nor FILENAME, use standard input

When output is redirected to a file or another command, output will  be
prettyfied unless --compress

``FILENAME``
- read input from filename instead of stdin

``INPUT``
- use this as input instead of stdin

``-W``
- consider continuing on non-blocking errors

``--ascii``
- ignore unicode values

``--keep-order``
- do not sort objects by key

``--color[=MODE]``
- colorize  output,  MODE  can  be  never  or always (default when ommited)

``--compress``
- if output is redirected, strip unnecessaries characters

``--diff-cmd COMMAND ;``
- Execute command as external diff. All following argument will be passed  as  parameter  to  COMMAND until ';' is encountered. The '{}' argument will be replaced by the file path of  the  prettified input

``-v``, ``-version``
- display version information

``-h``, ``--help``
- show this message and exit

## EXAMPLES
```sh
       jsonstroll f.json
          # Output f.json's content

       jsonstroll   --diff   fileA.json fileB.json
          # compare  fileA.json  and fileB.json

       jsonstroll --diff-cmd vimdiff {} \; fileA.json fileB.json
          # compare fileA.json and fileB.json
```

## AUTHOR
Written by ``isundil`` <isundill@gmail.com>.

## REPORTING BUGS
Report bugs to <isundill@gmail.com>

## COPYRIGHT
See [LICENSE](LICENSE)

jsonstroll (jsonstroller suite) on Nov  5 2016  JSONSTROLL(1)
