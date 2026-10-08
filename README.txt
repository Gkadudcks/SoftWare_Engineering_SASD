RVC Library Modules - independent sensor reads + EOF handling

The sensors.txt file has one row per tick, four 0/1 values per row:
    front left right dust

Each sensor interface opens sensors.txt independently, reads through the
current tick row, returns only its sensor's value, then closes the file.
After each of the four distinct sensor functions has run once, tick increases.

When the next read reaches EOF, the sensor function closes the file and
terminates the program normally with exit(EXIT_SUCCESS). This preserves the
original bool sensor API (which cannot represent an EOF separately).
No validation of 0/1 values or column count is performed. Assumes the file
exists and each row contains four valid 0/1 values.

Only the Library Modules section was edited in the original rvc.c. Other TODO
modules and original main.c / rvc.h are unchanged. The separate test_sensors.c
invokes sensor functions and demonstrates automatic termination at EOF.

Build / run (from this folder):
    gcc -std=c11 -Wall -Wextra rvc.c test_sensors.c -o test_sensors
    ./test_sensors
