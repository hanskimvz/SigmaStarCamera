# <- this is for comment / total file size must be less than 4KB
tftp 0x21000000 kernel
sf probe 0
sf erase KERNEL
sf write 0x21000000 KERNEL ${filesize}
% <- this is end of file symbol
