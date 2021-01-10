# Election Algorithm in a Ring

The program `ring_voting` provides a Ring with Workers which implement the Chang and Roberts Algorithm for Elections.

One can use the program with the command line which has the following options:

Usage: `ring_voting [OPTIONS] [size]`
```
Positionals:
  size UINT:POSITIVE          Positive Number of Workers in the Ring
                                This value needs to be provided either by CLI or config file

Options:
  -h,--help                   Print this help message and exit
  -c,--config TEXT:FILE       Toml config file from which to read
                                Configurations can be overriden with the CLI
                                Values which do not adhere to restrictions are replaced with default values
  -n,--number-of-elections UINT
                              Number of Elections after which to finish
                                Default 0 is infinit
  --sleep UINT                Sleeptime after each Election in milliseconds
                                Default is a sleeptime of 5 seconds
  --worker-sleep UINT         Sleeptime of each worker after finishing an operation in milliseconds
                                Default is a sleeptime of 500 milliseconds
  --log                       Enables logging
                                Default logging level is INFO
                                Default logging output is the console
  --log-file TEXT             Sets the file as log output and enables logging
  --log-date                  Logs the date additionally to the time, when logging to a file
  --log-level ENUM            Sets the visible logging level and enables logging
                                0 ... TRACE
                                1 ... DEBUG
                                2 ... INFO
                                3 ... WARN
                                4 ... ERROR
                                5 ... CRITICAL
  --no-config-log             Abstain from logging the used config as a DEBUG message
```
