#!/bin/bash     
svn co -r 6932 https://svn.ntop.org/svn/ntop/trunk/ntopng/
cd ntopng
svn co -r 6932 https://svn.ntop.org/svn/ntop/trunk/nDPI
cd ..
mv ntopng/ ntopng-1.1/
tar -cvzf ntopng-1.1.tar.gz ntopng-1.1/
openssl rmd160 ntopng-1.1.tar.gz
openssl sha256 ntopng-1.1.tar.gz
