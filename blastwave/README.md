to build the libTFHIC.so library run

`make`

then run root and load libTFHIC.so along with any dependend root macro, e.g.:

```
.L libTFHIC.so
.L test.cpp
test()
```

if modifying blastwave core scrs, to rebuild run from blastwave folder:

```
make clean
make
```
