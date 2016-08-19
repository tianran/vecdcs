#/bin/sh -e

wget -O Eigen-3.2.9.tar.bz2 http://bitbucket.org/eigen/eigen/get/3.2.9.tar.bz2
tar xfj Eigen-3.2.9.tar.bz2
mv eigen-eigen-dc6cfdf9bcec Eigen-3.2.9
rm Eigen-3.2.9.tar.bz2
