pipes: pipes.c redir.c redir.h
	gcc -o pipes pipes.c redir.c redir.h

clean:
	rm pipes
