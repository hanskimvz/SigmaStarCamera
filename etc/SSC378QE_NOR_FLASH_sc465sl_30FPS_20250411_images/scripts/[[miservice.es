# <- this is for comment / total file size must be less than 4KB
sf probe 0
sf erase miservice
tftp 0x21000000 miservice.sqfs
sf write 0x21000000 miservice ${filesize}
% <- this is end of file symbol
