# net-speeder
A program to speed up single thread download upon long delay and unstable network.

## Install
```bash
apt-get update
apt-get install libnet1 libnet1-dev libpcap0.8 libpcap0.8-dev --no-install-recommends

git clone https://github.com/pexcn/netspeeder.git
cd netspeeder
make
make install
cd ..
```

## Usage
```bash
# speed up all packets
nice -n -5 netspeeder eth0 "ip" &> /dev/null &

# speed up port 53 and 80
nice -n -5 netspeeder eth0 "src port 53 || tcp src port 80" &> /dev/null &
```
