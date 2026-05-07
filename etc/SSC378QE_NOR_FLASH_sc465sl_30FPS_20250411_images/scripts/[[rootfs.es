# <- this is for comment / total file size must be less than 4KB
sf probe 0
sf erase rootfs
tftp 0x21000000 rootfs.sqfs
sf write 0x21000000 rootfs ${filesize}
% <- this is end of file symbol
