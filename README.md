# fss-sat

Prerequisites

1. Clone this repo
2. Add INET to your workspace

How to run it on your local device:

1. `cd` to `omnetpp-x.y/path/to/fss-sat/src` on your omnetpp **SHELL**
2. Then run this command

```
opp_makemake -f --deep -I/c/omnetpp-6.1/path/to/your/inet/src -L/c/omnetpp-6.1/path/to/your/inet/src -lINET
```

3. Then run this command

```
make MODE=release cleanall
```

4. Then run this command

```
make MODE=release all
```

5. Open omnetpp IDE and go to `fss-sat/simulations` and then run `omnetpp.ini`
