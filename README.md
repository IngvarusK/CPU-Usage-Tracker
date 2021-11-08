# CPU-Usage-Tracker
Recruitment task 

Additional <ncurses.h> library. Istall with 'sudo apt-get install libncurses5-dev'
Using ncurses result is memory leak (rather reference to existing, still used memory). Problem described down below.
https://stackoverflow.com/questions/32410125/valgrind-shows-memory-leaks-from-ncurses-commands-after-using-appropriate-free

No memory leaks in connection with variables used in program.
