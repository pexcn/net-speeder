# net-speeder
A program to speed up single thread download upon long delay and unstable network.  
net-speeder is a double packet sender.  

## Build

```bash
# only for debian
apt-get update
apt-get install libnet1 libnet1-dev libpcap0.8 libpcap0.8-dev --no-install-recommends

git clone https://github.com/pexcn/netspeeder.git
pushd netspeeder
make
make install
popd
rm -r netspeeder
```

## Usage

```bash
# Example
# all packet speed up
nice -n -5 netspeeder venet0 "ip" > /dev/null 2>&1&

# send data speed up for port 22 and 53
nice -n -5 netspeeder venet0 "src port 22 || src port 53" > /dev/null 2>&1&
```
