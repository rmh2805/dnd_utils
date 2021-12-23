# DnD Utils Mapping Tool

Currently, the primary tool in this module is `makeMap`. While I do intend to 
further incorporate spritesets into the mapping util, at the moment the only 
loaded sprites are the tiles used to build up the maps.

This program requires ncurses 5 (instructions to do this on ubuntu can be found 
[here](https://askubuntu.com/questions/270381/how-do-i-install-ncurses-header-files))
and uses gnu make to automate the build process.

Once all required libraries are installed, simply run `make` in this directory 
to generate the `makeMap` and `makeSprite` executables, which can then be run 
from the command line with the command `./<executable name>`.

If you are initially unable to run the executable, you may need to enable 
execution priveleges on it, which can be done with the bash command 
`chmod u+x <executable name>`, and try again.

These programs contain help prompts in many of their modes which can be accessed
through the F1 key.
