# pinbus

## how to setup ubuntu

```
sudo apt-get source linux-image-$(uname -r)
cd linux-3.2.0/
cp /boot/config-3.2.0-25-generic .config
cp /lib/modules/3.2.0-25-generic/build/Module.symvers .
vi Makefile (EXTRAVERSION anpassen)
make clean
make oldconfig
make prepare
make prepare0
make scripts

cd /module/dir/in/homeverzeichnis
make
```

## build dkms
checkout this dir to /usr/src/pinbus-0.23
sudo dkms add -m pinbus -v 0.23
sudo dkms build -m pinbus -v 0.23


## build debian dkms package
sudo dkms mkdsc -m pinbus -v 0.23 --source-only
sudo dkms mkdeb -m pinbus -v 0.23 --source-only

cp  /var/lib/dkms/pinbus/0.23/deb/pinbus-dkms_0.23_all.deb .
sudo dkms remove pinbus/0.23 --all
sudo rm -rf /var/lib/dkms/pinbus/

