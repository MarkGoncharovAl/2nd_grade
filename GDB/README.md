# gdb commands for debugging 

1. ```start``` till main
2. ```Ctrl+X+A``` - Graphical interface
3. ```n``` - go futher (not deep into)
4. ```p <smth>``` - print array/variable and so on
5. ```b <somewhere>``` - make a breakpoint
6. ```r``` - start again from the beginning
7. ```p *a@5``` - print array that named a 
8. ```p``` can be used with all kinds of expressions
9. ```info breakpoints``` - print info
10. ```delete <num>``` - deleting breakpoint <num>
11. ```c``` - go till next breakpoint
12. ```command <num>``` - creating command that will happen when you're going to <num> breakpoint. Must be ended with ```end```
13. ```set pagination off``` - for not spamming the screen
    
### What is happening during debugging:

1. In order to not check every string -> debugger is editing your binary code
2. 