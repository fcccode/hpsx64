g++ -w -fpermissive -static-libgcc -static-libstdc++ -I./src -I./src/execution -I../../../common -O3 -o testR3000APrint testR3000APrint.cpp ./src/R3000ADebugPrint.cpp -Wl,-subsystem,console
pause