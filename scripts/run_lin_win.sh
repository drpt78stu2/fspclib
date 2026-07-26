# Peter Taraba recommends MSYS2 on Windows
# https://www.msys2.org/
# just download and no need for admin account

export PATH="/ucrt64/bin/:$PATH"
#pacman -Syu
pacman -S --needed mingw-w64-ucrt-x86_64-gcc
pacman -S --needed mingw-w64-ucrt-x86_64-boost
pacman -S --needed neofetch

rm statsort_bench.exe

g++ -O3 ./benchmarks/statsort_bench.cpp -I./code -o statsort_bench.exe

echo "------ benchmark -----"
neofetch >> ./results/results.txt
./statsort_bench.exe >> ./results/results.txt

rm statsort_bench.exe
