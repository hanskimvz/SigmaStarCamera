# <- this is for comment / total file size must be less than 4KB
tftp 0x21000000 boot/flash_list.nri
sf probe 0x21000000
setenv mtdids nor0=nor0
setenv mtdparts ' mtdparts=nor0:0x4F000(BOOT),0x1000(ENV),0x2C0000(KERNEL),0x2F0000(rootfs),0x350000(miservice),0x650000(customer)
saveenv
% <- this is end of file symbol
