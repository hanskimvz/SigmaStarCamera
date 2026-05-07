echo 4 > /proc/sys/kernel/printk

#kernel_mod_list
insmod /config/modules/5.10/nfs_ssc.ko
insmod /config/modules/5.10/libarc4.ko
insmod /config/modules/5.10/usb-common.ko
insmod /config/modules/5.10/usbcore.ko
insmod /config/modules/5.10/phy-sstar-u2phy.ko
insmod /config/modules/5.10/ehci-hcd.ko
insmod /config/modules/5.10/scsi_mod.ko
insmod /config/modules/5.10/usb-storage.ko
insmod /config/modules/5.10/md4.ko
insmod /config/modules/5.10/seqiv.ko
insmod /config/modules/5.10/libdes.ko
insmod /config/modules/5.10/cifs.ko
insmod /config/modules/5.10/nls_utf8.ko
insmod /config/modules/5.10/grace.ko
insmod /config/modules/5.10/sunrpc.ko
insmod /config/modules/5.10/lockd.ko
insmod /config/modules/5.10/nfs.ko
insmod /config/modules/5.10/nfsv2.ko
insmod /config/modules/5.10/nfsv3.ko
insmod /config/modules/5.10/mmc_core.ko
insmod /config/modules/5.10/mmc_block.ko
insmod /config/modules/5.10/kdrv_sdmmc.ko
insmod /config/modules/5.10/fat.ko
insmod /config/modules/5.10/msdos.ko
insmod /config/modules/5.10/vfat.ko
insmod /config/modules/5.10/ntfs.ko
insmod /config/modules/5.10/sd_mod.ko

#mi module
insmod /config/modules/5.10/mi_all.ko

#mi module mknod

#mi sensor
insmod /config/modules/5.10/sc465sl_MIPI.ko chmap=1
mdev -s
