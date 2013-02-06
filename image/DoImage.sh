#!/bin/sh
. ../define/PATH
. ../define/FUNCTION_SCRIPT
CHAR_TO_ASCII()
{
if [ "$1" ]; then
ascii=32
for i in " " "!" "\"" "#" "\$" "%" "&" "'" "(" ")" "*" "+" "," "-" "." "/" "0" "1" "2" "3" "4" "5" "6" "7" "8" "9" ":" ";" "<" "=" ">" "?" "@"\
"A" "B" "C" "D" "E" "F" "G" "H" "I" "J" "K" "L" "M" "N" "O" "P" "Q" "R" "S" "T" "U" "V" "W" "X" "Y" "Z" "[" "\\" "]" "^" "_" "\`"\
"a" "b" "c" "d" "e" "f" "g" "h" "i" "j" "k" "l" "m" "n" "o" "p" "q" "r" "s" "t" "u" "v" "w" "x" "y" "z" "{" "|" "}" "~"; do
if [ "$1" = "$i" ]; then
echo "$ascii"
break;
fi
ascii=`expr $ascii + 1`
done
else
echo 0
fi
}
DEC_TO_HEX()
{
temp=$1
result=""
while [ "$temp" ] && [ $temp -gt 0 ]; do
mod=`expr $temp - $temp / 16 \* 16`
for i in "0" "1" "2" "3" "4" "5" "6" "7" "8" "9" "A" "B" "C" "D" "E" "F"; do
if [ $mod -eq 0 ]; then
break;
else
mod=`expr $mod - 1`
fi
done
result=${i}${result}
temp=`expr $temp / 16`
done
if [ ! "$result" ]; then
result=0
fi
if [ "$2" ]; then
while [ `expr length $result` -lt $2 ]; do
result="0"${result}
done
fi
echo $result
}
PAD_FILE_WITH_A_BYTE()
{
if [ "$1" ] && [ "$2" ]; then
filename=$1
byte_to_pad=$2
filesize=`du -b $filename | cut -f 1 -d "	"`
objcopy -I binary -O binary --gap-fill=$byte_to_pad --pad-to=`expr $filesize + 1` ${filename} ${filename}_temp
rm -rf ${filename}
mv ${filename}_temp ${filename}
fi
}
if [ "$_MODE_" = "Customer" ]; then
objcopy -I binary -O binary --gap-fill=0 --pad-to=0xFFFF0 linux linux-pad
../AP/goahead-2.1.1/LINUX/cvimg root appimg appimg.bin 0x80500000 0xC0000
mv appimg.bin appimg
elif [ "$_KERNEL_VERSION_26_" = "y" ];then
objcopy -I binary -O binary --gap-fill=0 --pad-to=0xFFFF0 linux linux-pad
../AP/goahead-2.1.1/LINUX/cvimg root appimg appimg.bin 0x80500000 0xC0000
mv appimg.bin appimg
else
objcopy -I binary -O binary --gap-fill=0 --pad-to=0x9FFF0 linux linux-pad
fi
cat linux-pad appimg > linux-org
rm -f linux-pad
LANG=eng
if [ "$_MODEL_" = "BR6258GN" ] || [ "$_MODEL_" = "BR6458GN" ]; then
if [ "$_MODE_" = "Customer" ]; then
BOOT_FILE="boot-${_FLASHTYPE_}-${_MEMSIZE_}M-${_MEMBUS_}BIT-${_MEMTYPE_}-6258GN-Silex"
else
BOOT_FILE="boot-${_FLASHTYPE_}-${_MEMSIZE_}M-${_MEMBUS_}BIT-${_MEMTYPE_}-6258GN"
fi
elif [ "$_MODEL_" = "BR6268GN" ]; then
BOOT_FILE="boot-${_FLASHTYPE_}-${_MEMSIZE_}M-${_MEMBUS_}BIT-${_MEMTYPE_}-6X68GN"
elif [ "$_MODEL_" = "BR6428HPN" ]; then
BOOT_FILE="boot-${_FLASHTYPE_}-${_MEMSIZE_}M-${_MEMBUS_}BIT-${_MEMTYPE_}-6X28HPN"
else
if [ "$_PLATFORM_" = "RTL8196C_1200" ] || [ "$_PLATFORM_" = "RTL8196C_2500" ] || [ "$_PLATFORM_" = "RTL8196C_2300" ] || [ "$_PLATFORM_" = "RTL8196C_2400" ]; then
BOOT_FILE="boot-${_FLASHTYPE_}-${_MEMSIZE_}M-${_MEMBUS_}BIT-${_MEMTYPE_}-96C"
elif [ "$_PLATFORM_" = "RTL8198_2300" ] || [ "$_PLATFORM_" = "RTL8198_2400" ] || [ "$_PLATFORM_" = "RTL8198_2500" ]; then
if [ "$_MODE_" = "Customer" ] || [ "$_MODE_" = "Customer" ]; then
BOOT_FILE="boot-${_FLASHTYPE_}-${_MEMSIZE_}M-${_MEMBUS_}BIT-${_MEMTYPE_}-98-Jensen"
else
BOOT_FILE="boot-${_FLASHTYPE_}-${_MEMSIZE_}M-${_MEMBUS_}BIT-${_MEMTYPE_}-98"
fi
else
BOOT_FILE="boot-${_FLASHTYPE_}-${_MEMSIZE_}M-${_MEMBUS_}BIT-${_MEMTYPE_}"
fi
fi
if [ ! -f ${BOOT_FILE} ]; then
echo "boot code not found!!"
else
objcopy -I binary -O binary --gap-fill=0 --pad-to=0x10000 ${BOOT_FILE} boot-pad
../AP/goahead-2.1.1/LINUX/cvimg linux linux-org linux.bin 0x80500000 0x10000
if [ "$_KERNEL_VERSION_26_" = "y" -o "$_FLASH4M_" = "y" ];then
objcopy -I binary -O binary --gap-fill=0 --pad-to=0x3f0000 linux.bin linux.bin-pad
else
objcopy -I binary -O binary --gap-fill=0 --pad-to=0x1f0000 linux.bin linux.bin-pad
fi
cat boot-pad linux.bin-pad > Factory_temp.bin
rm -rf boot-pad linux.bin-pad config-pad.bin
if [ $? != 0 ]; then
exit 1
fi
chmod 777 Factory_temp.bin
chmod 777 linux.bin
mkdir -p ./${_MODEL_}/${_MODE_}/Tester
mkdir -p ./${_MODEL_}/${_MODE_}/Factory
if [ "$_LANGUAGE_PACK_" ]; then
if [ "`du -b linux.bin | cut -f 1 -d \"	\"`" ]; then
image_size=`du -b linux.bin | cut -f 1 -d "	"`
else
image_size=0
fi
if [ ! -d "${GOAHEADDIR}/web/language_pack" ]; then
language_number=0
language_file_list=""
else
language_number=`ls ${GOAHEADDIR}/web/language_pack/*.txt | wc -l`
language_file_list=`temp=\`pwd\`; cd ${GOAHEADDIR}/web/language_pack; ls *.txt | cut -f 1 -d .; cd \$temp`
fi
echo $file_size $language_number $language_file_list
temp=`pwd`
cd ${GOAHEADDIR}/web/language_pack
for i in $language_file_list; do
rm -rf ${i}.txt.gz
cp ${i}.txt ${i}.txt1
gzip -9 ${i}.txt
mv ${i}.txt1 ${i}.txt
done
cd $temp
PAD_FILE_WITH_A_BYTE linux.bin $language_number
for i in $language_file_list; do
PAD_FILE_WITH_A_BYTE linux.bin 0x`DEC_TO_HEX \`CHAR_TO_ASCII "\\\`echo $i | cut -b1\\\`"\``
PAD_FILE_WITH_A_BYTE linux.bin 0x`DEC_TO_HEX \`CHAR_TO_ASCII "\\\`echo $i | cut -b2\\\`"\``
size=`du -b ${GOAHEADDIR}/web/language_pack/${i}.txt.gz | cut -f 1 -d "	"`
size=`DEC_TO_HEX $size 8`
for j in 1 3 5 7; do
k=`expr $j + 1`
PAD_FILE_WITH_A_BYTE linux.bin 0x`echo $size | cut -b $j-$k`
done
done
for i in $language_file_list; do
cat ${GOAHEADDIR}/web/language_pack/$i.txt.gz >> linux.bin
done
fi
if [ "$_Israel_" = "y" ]; then
mv -f Factory_temp.bin ./${_MODEL_}/${_MODE_}/Factory/${_MODEL_}_${_MODE_}_PIKOK_${_VERSION_}_mp.bin
cp -f linux.bin        ./${_MODEL_}/${_MODE_}/Tester/${_MODEL_}_${_MODE_}_PIKOK_${_VERSION_}_upg.bin
ls -l ./${_MODEL_}/${_MODE_}/Tester/${_MODEL_}_${_MODE_}_PIKOK_${_VERSION_}_upg.bin ./${_MODEL_}/${_MODE_}/Factory/${_MODEL_}_${_MODE_}_PIKOK_${_VERSION_}_mp.bin
else
mv -f Factory_temp.bin ./${_MODEL_}/${_MODE_}/Factory/${_MODEL_}_${_MODE_}_${_VERSION_}_mp.bin
cp -f linux.bin        ./${_MODEL_}/${_MODE_}/Tester/${_MODEL_}_${_MODE_}_${_VERSION_}_upg.bin
ls -l ./${_MODEL_}/${_MODE_}/Tester/${_MODEL_}_${_MODE_}_${_VERSION_}_upg.bin ./${_MODEL_}/${_MODE_}/Factory/${_MODEL_}_${_MODE_}_${_VERSION_}_mp.bin
fi
fi
