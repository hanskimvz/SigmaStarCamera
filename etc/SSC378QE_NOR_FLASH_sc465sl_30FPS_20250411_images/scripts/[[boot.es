# <- this is for comment / total file size must be less than 4KB
tftp 0x21000000 boot.bin
sf probe 0
sf erase BOOT
sf write 0x21000000 BOOT ${filesize}
% <- this is end of file symbol
