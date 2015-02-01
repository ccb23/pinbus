# pinbus

## howto setup ubuntu build environment ( just using current kernel headers )

```
sudo apt-get install linux-headers-`uname -r`
cd /pinbus/dir/
make
```

## build dkms
```
checkout this dir to /usr/src/pinbus-0.23
sudo dkms add -m pinbus -v 0.23
sudo dkms build -m pinbus -v 0.23
```

## build debian dkms package

```
sudo dkms mkdsc -m pinbus -v 0.23 --source-only
sudo dkms mkdeb -m pinbus -v 0.23 --source-only

cp  /var/lib/dkms/pinbus/0.23/deb/pinbus-dkms_0.23_all.deb .
sudo dkms remove pinbus/0.23 --all
sudo rm -rf /var/lib/dkms/pinbus/
```

## update rpi system

```
sudo apt-get update && sudo apt-get dist-upgrade
sudo rpi-update
```

## install dkms package on raspberrypi

```
sudo apt-get install gcc-4.7 cpp-4.7
wget http://www.niksula.hut.fi/~mhiienka/Rpi/linux-headers-rpi/linux-headers-3.12.33%2b_3.12.33%2b-2_armhf.deb
sudo dpkg -i linux-headers-3.12.33+_3.12.33+-2_armhf.deb
sudo dpkg -i pinbus-dkms_0.23_all.deb
```

## howto setup ubuntu build environment ( with whole kernel tree )

```
sudo apt-get source linux-image-$(uname -r)
cd linux-3.2.0/
cp /boot/config-3.2.0-25-generic .config
cp /lib/modules/3.2.0-25-generic/build/Module.symvers .
vi Makefile (set correct EXTRAVERSION)
make clean
make oldconfig
make prepare
make prepare0
make scripts

cd /pinbus/dir/
make
```

## howto autostart service on raspberry pi

```
git clone https://github.com/ccb23/pinbus.git /usr/src/pinbus
echo pinbus >> /etc/modules
echo 'nohup /usr/src/pinbus/tools/rc.pinbus &' >> /etc/rc.local

```
