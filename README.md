# redirect-streams

Usage: redir <inp> <cmd> <out>

fork and exec "cmd", redirecting the input and output descriptors as necessary. Assume the values inp and out are filenames, read from and write to them respectively. If they are "-" leave them as stdin/stdout.

Note: cmd is ONE argument. For example "wc -l". You may need to split it into separate words (on " ") before execing it. You will need to properly handle (in code) the case where the command is not an absolute path: find the absolute path for the command!

No need to worry about spaces or quoted strings in its parameters. (It is not required to handle 'echo "foo bar"' as a command. That would/should/could get passed on as "echo", ""foo", "bar"".)