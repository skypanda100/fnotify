target = fnotify
obj = fnotify.o notify.o watch.o config.o

cflags = -Wall -std=c99

$(target) : $(obj)
	gcc -o $(target) $(cflags) $(obj)

fnotify.o : def.h config.h notify.h watch.h
	gcc -c $(cflags) fnotify.c

notify.o : def.h
	gcc -c $(cflags) notify.c

watch.o : def.h notify.h
	gcc -c $(cflags) watch.c

config.o : def.h
	gcc -c $(cflags) config.c

.PHONY : clean
clean :
	rm -f $(target) $(obj)
