#!/bin/zsh

function rp() { python -c "import os,sys; print(os.path.realpath(sys.argv[1]))" "$1"; }

cd $(dirname $(dirname $(rp $0)))

if [ -f build ]
then
    echo Please check "build" file and remove it
    exit 1
fi

cmake -B build . > /dev/null
make -C build > /dev/null

ls bin > setup/.install
sudo cp bin/* /usr/local/bin

rm -r bin build

echo Install into /usr/local/bin successfully
