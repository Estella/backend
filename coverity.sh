#!/bin/sh

make clean
cov-build --dir cov-int make
tar czvf backend.tar.gz cov-int

curl --form token=kypOtMw402gcM_P2-IdpPg \
      --form email=aaron@aaronbedra.com \
      --form file=@backend.tar.gz \
      --form version=$1 \
      --form description=$1 \
      https://scan.coverity.com/builds?project=repsheet%2Fbackend
