2019-08-03
This directory contains headers for writing relocatable "apps".
These "apps" are small, because they re-use common I/O functions that are already present by the "OS".
To create an app, you can use the Python script "rel2app.py" to post-process your compiled program to an "app".

You can think of it as a simplified libc that is provided by the loader ("api").
The loader takes the given "app" binary, copies it to any offset in RAM and re-writes the addresses inside the code to match the new locations (relocation).

If we were extremely cool, the "api" would match CP/M's BIOS functions ;-)
