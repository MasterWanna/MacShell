#!/bin/zsh

function rp() { python3 -c "import os,sys; print(os.path.realpath(sys.argv[1]))" "$1"; }

cd $(dirname $(dirname $(rp $0)))

file=./setup/.install
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

if [ -d ./scripts/utils/__pycache__ ]
then
    rm -rf ./scripts/utils/__pycache__
fi

mkdir bin
cp scripts/*/* ./bin

if [ -f ./bin/__init__.py ]
then
    rm ./bin/__init__.py
fi

ls bin > ./setup/.install
sudo cp ./bin/* /usr/local/bin

rm -r bin

echo Install into /usr/local/bin successfully
