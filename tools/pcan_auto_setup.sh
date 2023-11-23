#!/bin/bash
wget -c https://www.peak-system.com/fileadmin/media/linux/files/peak-linux-driver-8.14.0.tar.gz
tar -xzf peak-linux-driver-8.14.0.tar.gz
cd peak-linux-driver-8.14.0/
make clean
sudo apt install libpopt-dev
make
sudo make install
cd ../../
wget -c https://www.peak-system.com/quick/BasicLinux
tar -xzf BasicLinux
cd PCAN-Basic_Linux-4.6.0/
cd libpcanbasic/
make clean
make
sudo make install
wget 'https://forum.peak-system.com/download/file.php?id=1177&sid=2a05c3512c5efd58957ab9d71f4e2614' --no-check-certificate
tar -xzf 'file.php?id=1177&sid=2a05c3512c5efd58957ab9d71f4e2614'
sudo dpkg -i pcanview-ncurses_0.8.7-5_amd64-trusty.deb
sudo apt-get install libncurses5
pcanview
