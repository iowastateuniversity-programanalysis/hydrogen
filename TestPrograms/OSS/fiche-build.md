clang -c -O0 -Xclang -disable-O0-optnone -g -emit-llvm -S main.c -o main.bc
clang -c -O0 -Xclang -disable-O0-optnone -g -emit-llvm -S fiche.c -o fiche.bc
llvm-link -S fiche.bc main.bc -o prog.bc
