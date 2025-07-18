in blastwave subfolder, run make to build the libTFHIC.so library
then, from blastwave subfolder, run root and load libTFHIC.so along with any dependend root macro, e.g.:

  .L libTFHIC.so
  .L test.cpp
  test()

if modifying blastwave core scrs, to rebuild run from blastwave subfolder:

  make clean
  make
