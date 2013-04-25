simpleHTTPserver
================

My IT215 Systems Software project code. A simple HTTP webserver.

#Description#

This is a small and simple network application to work as an HTTP server. A very small subset of HTTP protocol is implemented.
The major focus is on multithreading (using pthreads), socket programming, and file handling.

Building and using
------------------
To build the server, just invoke gcc with the pthread library flag.
```bash
gcc -lpthread http_server.c
```

and then start the server by calling
```bash
./a.out
```

The server has started. Fire up your browser and point it towards http://127.0.0.1:8181 and see it in action.

#important note#
Right now all the logs are generated on the fly on the console only. The server is stopped by the kill signal ie ctrl-c
