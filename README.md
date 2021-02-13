# netspeeder

A program to speed up single thread download upon long delay and unstable network.

## Install
```bash
apt-get update
apt-get install --no-install-recommends -y libnet1 libnet1-dev libpcap0.8 libpcap0.8-dev

git clone https://github.com/pexcn/netspeeder.git
cd netspeeder
make
make install
cd ..
```

## Usage
```bash
# speed up all ip packets
nice -n -5 netspeeder eth0 "ip" &> /dev/null &

# speed up port 53 outbound packets and tcp port 80 outbound packets
nice -n -5 netspeeder eth0 "src port 53 || tcp src port 80" &> /dev/null &

# fix error: "err msg: [libnet_write_raw_ipv4(): -1 bytes written (Message too long)"
ethtool -K eth0 tso off
```
