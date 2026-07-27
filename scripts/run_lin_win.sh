# Peter Taraba recommends MSYS2 on Windows
# https://www.msys2.org/
# just download and no need for admin account

export PATH="/ucrt64/bin/:$PATH"
#sudo pacman -Syu

#windows
sudo pacman -S --needed mingw-w64-ucrt-x86_64-gcc
sudo pacman -S --needed mingw-w64-ucrt-x86_64-boost

#linux
sudo pacman -Syu --needed gcc
sudo pacman -S --needed boost
sudo pacman -S --needed neofetch

rm statsort_bench

g++ -O3 ./benchmarks/statsort_bench.cpp -I./code -o statsort_bench

echo "------ benchmark -----"
fastfetch >> ./results/results_pc.txt

for i in {1..30}
do
    ./statsort_bench >> ./results/results.txt
done

rm statsort_bench
