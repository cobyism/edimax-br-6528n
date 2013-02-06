#!/bin/sh
. ../../target.def
#Include Function Defined List.(Kyle)
. ../../define/PATH
. ../../define/FUNCTION_SCRIPT

rm -rf ${ROMFSDIR}/etc/stupid-ftpd
rm -rf ${ROMFSDIR}/etc.tmp/stupid-ftpd
rm -f ${ROMFSDIR}/etc/linuxigd/gatedesc-Customer.xml.def
rm -f ${ROMFSDIR}/etc/linuxigd/gatedesc-general.xml.def
rm -f ${ROMFSDIR}/etc/linuxigd/gatedesc-taiCustomer.xml.def


if [ "${_MODE_}" = "Customer" ]; then
	
	if [ "$_DBAND_" = "y" ]; then	
		rm -rf ${ROMFSDIR}/web/file/banner_l.gif
		rm -rf ${ROMFSDIR}/web/file/banner_m.gif
		rm -rf ${ROMFSDIR}/web/file/banner_r.gif
	else
		rm -rf ${ROMFSDIR}/web/file/db_banner_l.gif
		rm -rf ${ROMFSDIR}/web/file/db_banner_m.gif
		rm -rf ${ROMFSDIR}/web/file/db_banner_r.gif
	fi
fi