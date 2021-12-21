#!/bin/zsh

function rp() { python -c "import os,sys; print(os.path.realpath(sys.argv[1]))" "$1"; }

cd $(dirname $(dirname $(rp $0)))

file=$PWD/setup/.install
if [ -f $file ]
then
    echo "Uninstalling ..."
    cd /usr/local/bin
    sudo rm -rf $(cat $file)
    cd -
    rm $file
    echo Uninstall from /usr/local/bin successfully
else
    echo Binary and Script are not installed
fi
