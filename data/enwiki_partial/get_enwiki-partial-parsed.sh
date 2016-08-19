#/bin/sh -e

wget --no-check-certificate -O enwiki-partial-parsed.tar.bz2 'https://drive.google.com/uc?id=0B_-oZIbBJszXMG9abkVVU3lLZ2c&export=download'
tar xfj enwiki-partial-parsed.tar.bz2
rm enwiki-partial-parsed.tar.bz2
