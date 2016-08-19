#/bin/sh -e

wget --no-check-certificate -O enwiki-model.tar.bz2 'https://drive.google.com/uc?id=0B_-oZIbBJszXemZXbzZBdk9ZV1E&export=download'
tar xfj enwiki-model.tar.bz2
rm enwiki-model.tar.bz2
