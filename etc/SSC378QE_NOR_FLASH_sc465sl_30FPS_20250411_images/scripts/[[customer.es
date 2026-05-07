# <- this is for comment / total file size must be less than 4KB
sf probe 0
sf erase customer
tftp 0x21000000 customer.jffs2
sf write 0x21000000 customer ${filesize}
% <- this is end of file symbol
