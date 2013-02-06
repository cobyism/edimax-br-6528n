

obj-y           += 
obj-m           :=
obj-n           :=
obj-            :=

O_TARGET	:= flash.o

export-objs	:= 
list-multi	:= 

mod-subdirs	:= 
subdir-y	:=
subdir-m	:= 

subdir-$(CONFIG_FLASH_NOR) += nor
#subdir-$(CONFIG_FLASH_NAND) += nand

obj-$(CONFIG_FLASH_NOR)	+= nor/nor.o
#obj-$(CONFIG_FLASH_NAND) += nand/nand.o



include $(TOPDIR)/Rules.make




