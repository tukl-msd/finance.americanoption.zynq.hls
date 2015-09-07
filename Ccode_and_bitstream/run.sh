sudo chown root:admin /dev/xdevcfg; sudo chmod 660 /dev/xdevcfg
#cat ./design_1_wrapper.bit.bin > /dev/xdevcfg
sudo chmod 660 /dev/mem
g++ -g -Wall mt19937ar.cpp ls_golden.cpp main.cpp -o main.exe
sudo ./main.exe

