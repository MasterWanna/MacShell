#!/bin/zsh

function rp() { python3 -c "import os,sys; print(os.path.realpath(sys.argv[1]))" "$1"; }

cd $(dirname $(dirname $(rp $0)))

if [ -f build ]
then
    echo Please check \"build\" file and remove it
    exit 1
fi

file=$PWD/setup/.install
if [[ -f $file ]]
then   
    if [[ ! "$(getopt y "$@")" =~ ^.*[Yy].*$ ]]
    then 
        read -k 1 "opt?Binary and Script are already installed. Uninstall them first. 
Do you want to continue? Y/[N]: "
        if [[ ! "$opt" == "
" ]]
        then
            echo
        fi
        if [[ ! "$opt" =~ ^[Yy]$ ]]
        then
            echo Abort.
            exit 1
        fi
    fi
    ./setup/uninstall.sh
fi

echo "Installing ..."

cmake -B build -Wno-dev . > /dev/null
make -C build > /dev/null

ls bin > setup/.install
sudo cp bin/* /usr/local/bin

rm -r bin build

echo Install into /usr/local/bin successfully
