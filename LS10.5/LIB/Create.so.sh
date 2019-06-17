rm *.o ; ar x ../libbaseLS10.5.a ; ld -shared -o /usr/lib/libbaseLS10.5.so *.o
rm *.o ; ar x ../libdbifLS10.5.a ; ld -shared -o /usr/lib/libdbifLS10.5.so *.o
rm *.o ; ar x ../libDSOXtraLS10.5.a ; ld -shared -o /usr/lib/libDSOXtraLS10.5.so *.o
rm *.o ; ar x ../libscrgenLS10.5.a ; ld -shared -o /usr/lib/libscrgenLS10.5.so *.o
