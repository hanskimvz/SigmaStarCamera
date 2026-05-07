echo 4 > /proc/sys/kernel/printk

KO_PATH=/opt/app/ko/5.10
#kernel_mod_list
insmod $KO_PATH/nfs_ssc.ko
#insmod $KO_PATH/libarc4.ko
#insmod $KO_PATH/usb-common.ko
#insmod $KO_PATH/usbcore.ko
insmod $KO_PATH/phy-sstar-u2phy.ko
#insmod $KO_PATH/ehci-hcd.ko
insmod $KO_PATH/scsi_mod.ko
insmod $KO_PATH/usb-storage.ko
#insmod $KO_PATH/md4.ko
#insmod $KO_PATH/seqiv.ko
#insmod $KO_PATH/libdes.ko
#insmod $KO_PATH/cifs.ko
insmod $KO_PATH/nls_utf8.ko
insmod $KO_PATH/grace.ko
insmod $KO_PATH/sunrpc.ko
insmod $KO_PATH/lockd.ko
insmod $KO_PATH/nfs.ko
insmod $KO_PATH/nfsv2.ko
insmod $KO_PATH/nfsv3.ko
insmod $KO_PATH/mmc_core.ko
insmod $KO_PATH/mmc_block.ko
insmod $KO_PATH/kdrv_sdmmc.ko
insmod $KO_PATH/fat.ko
#insmod $KO_PATH/msdos.ko
insmod $KO_PATH/vfat.ko
insmod $KO_PATH/exfat.ko
#insmod $KO_PATH/ntfs.ko
#insmod $KO_PATH/sd_mod.ko

#mi module
insmod $KO_PATH/mi_all.ko

#mi module mknod

echo hvsp1 down fc25G1.00_SWLUT.txt fc25G1.00_SWLUT.txt > /sys/class/mstar/mscl/hvsp

#wifi module
insmod $KO_PATH/firmware_class.ko
insmod $KO_PATH/cfg80211.ko
insmod $KO_PATH/8188fu.ko

#mi sensor
insmod $KO_PATH/sc450ai_MIPI.ko chmap=1
mdev -s

##udhcpc -s /etc/init.d/udhcpc.script &
##telnetd

echo 800000 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq
echo 800000 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq

echo 320000000 > /sys/class/mstar/isp0/isp_clk

echo 384000000 > /sys/devices/virtual/mstar/venc/ven_clock
echo 320000000 > /sys/devices/virtual/mstar/venc/ven_clock_2nd
echo 320000000 > /sys/devices/virtual/mstar/venc/ven_clock_axi
echo 480000000 > /sys/devices/virtual/mstar/venc/ven_clock_scdn

echo 320000000 > /sys/devices/virtual/mstar/mscl/clk
echo 384000000 > /sys/class/mstar/jpe/jpe_clock
echo 400 > /sys/dla/freq
echo 1 > /proc/mi_modules/mi_sensor/debug_level
echo 1 > /proc/mi_modules/mi_venc/debug_level
echo 1 > /proc/mi_modules/mi_sys/debug_level
echo 1 > /proc/mi_modules/mi_vif/debug_level
echo 1 > /proc/mi_modules/mi_isp/debug_level
echo 1 > /proc/mi_modules/mi_scl/debug_level
echo 1 > /proc/mi_modules/mi_ai/debug_level
echo 1 > /proc/mi_modules/mi_ao/debug_level
#$KO_PATH/riu_w 1038 6a 0c0c

#echo 1 > /sys/module/mi/parameters/drv_venc_wrapper.SCDN_MODE
echo scltable 0 fc33G1.00_SWLUT.txt fc33G1.00_SWLUT.txt 3/1 > /sys/class/mstar/venc/venc_param_ctrl
echo scltable 1 fc25G1.00_SWLUT.txt fc25G1.00_SWLUT.txt 4/1 > /sys/class/mstar/venc/venc_param_ctrl
