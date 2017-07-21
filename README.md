# xmpp_xep0047

XMPP XEP0047

### Pre-Install: ###

- libstrophe 0.9.0 and after, Ref. [strophe](https://github.com/strophe/libstrophe).

>```
sudo apt-get install autoconf
sudo apt-get install libtool
sudo apt-get install libexpat-dev
wget https://github.com/strophe/libstrophe/archive/0.9.1.tar.gz
tar zxvf 0.9.1.tar.gz
cd libstrophe-0.9.1/
./bootstrap.sh
./configure
make
make install
sudo make install
```

### Build: ###

- build libraries and example

>```
make
```

### Example: ###

- run example, you can run two main_ibb to establish session and send message to each other.

>```
./main_ibb -s localhost -j user@localhost -w 1234
```
