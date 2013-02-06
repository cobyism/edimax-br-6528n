#!/bin/sh
set -e
if [ $# = 1 ]; then
	case "$1" in
		"clean")
			MAKECLEAR=1
			MAKING=0
			;;
		"make")
			MAKECLEAR=0
			MAKING=1
			;;
        "ap")
            echo "AP dosn't need this!!"
			exit 0
            ;;
        "ga")
            echo "GA dosn't need this!!"
			exit 0
            ;;
		"config")
			MAKECLEAR=1
			MAKING=1
			TMP_DIR=`pwd`
			cd ../libpcap-0.7.2
			make clean
			cd $TMP_DIR
			;;
						
    esac
fi

if [ "$MAKING" = "1" ]; then
	if [ -f ../libpcap-0.7.2/libpcap.a ]; then
		echo "libpcap ok"
	else
		TMP_DIR=`pwd`
		cd ../libpcap-0.7.2
		make
		cd $TMP_DIR
	fi
fi


cd pppd/plugins/rp-pppoe

if [ "$MAKECLEAR" = "1" ]; then
	make clean
	rm -f *.a
	if [ $? != 0 ]; then
   		exit 1
	fi
fi
if [ "$MAKING" = "1" ]; then
    make clean
    rm -f *.a
    make
    if [ $? != 0 ]; then
        exit 1
    fi
fi
														

cd ../../..




cd pppd

if [ "$MAKECLEAR" = "1" ]; then
    make clean
    if [ $? != 0 ]; then
        exit 1
    fi							
fi
if [ "$MAKING" = "1" ]; then
    make clean
    make
    if [ $? != 0 ]; then
        exit 1
    fi						
fi

cd ..

cd chat

if [ "$MAKECLEAR" = "1" ]; then
    make clean
    if [ $? != 0 ]; then
        exit 1
    fi							
fi
if [ "$MAKING" = "1" ]; then
    make clean
    make
    if [ $? != 0 ]; then
        exit 1
    fi						
fi

cd ..
