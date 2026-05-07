
touch /tmp/log_proc.txt
echo "Hello World !" 
rm -f /tmp/log_proc.txt
echo "************ Start Dump Module Proc Message ! ******************"  >> /tmp/log_proc.txt

echo " cat /proc/mi_modules/mi_sensor/mi_sensor0 "  >> /tmp/log_proc.txt
cat /proc/mi_modules/mi_sensor/mi_sensor0 >>  /tmp/log_proc.txt

echo " cat /proc/mi_modules/mi_vif/mi_vif0 "  >> /tmp/log_proc.txt
cat /proc/mi_modules/mi_vif/mi_vif0 >>  /tmp/log_proc.txt

echo " cat /proc/mi_modules/mi_isp/mi_isp0 "  >> /tmp/log_proc.txt
cat /proc/mi_modules/mi_isp/mi_isp0 >>  /tmp/log_proc.txt
echo " cat /proc/mi_modules/mi_isp/mi_isp1 "  >> /tmp/log_proc.txt
cat /proc/mi_modules/mi_isp/mi_isp1 >>  /tmp/log_proc.txt

echo " cat /proc/mi_modules/mi_scl/mi_scl0 "  >> /tmp/log_proc.txt
cat /proc/mi_modules/mi_scl/mi_scl0 >>  /tmp/log_proc.txt
echo " cat /proc/mi_modules/mi_scl/mi_scl1 "  >> /tmp/log_proc.txt
cat /proc/mi_modules/mi_scl/mi_scl1 >>  /tmp/log_proc.txt
echo " cat /proc/mi_modules/mi_scl/mi_scl0 "  >> /tmp/log_proc.txt
cat /proc/mi_modules/mi_scl/mi_scl4 >>  /tmp/log_proc.txt
echo " cat /proc/mi_modules/mi_scl/mi_scl1 "  >> /tmp/log_proc.txt
cat /proc/mi_modules/mi_scl/mi_scl5 >>  /tmp/log_proc.txt
echo " cat /proc/mi_modules/mi_scl/mi_scl3 "  >> /tmp/log_proc.txt
cat /proc/mi_modules/mi_scl/mi_scl3 >>  /tmp/log_proc.txt

echo " cat /proc/mi_modules/mi_venc/mi_venc0 "  >> /tmp/log_proc.txt
cat /proc/mi_modules/mi_venc/mi_venc0 >>  /tmp/log_proc.txt
echo " cat /proc/mi_modules/mi_venc/mi_venc1 "  >> /tmp/log_proc.txt
cat /proc/mi_modules/mi_venc/mi_venc1 >>  /tmp/log_proc.txt
echo " cat /proc/mi_modules/mi_venc/mi_venc8 "  >> /tmp/log_proc.txt
cat /proc/mi_modules/mi_venc/mi_venc8 >>  /tmp/log_proc.txt
echo " cat /proc/mi_modules/mi_venc/mi_venc9 "  >> /tmp/log_proc.txt
cat /proc/mi_modules/mi_venc/mi_venc9 >>  /tmp/log_proc.txt

echo " cat /proc/mi_modules/mi_rgn/mi_rgn0 "  >> /tmp/log_proc.txt
cat /proc/mi_modules/mi_rgn/mi_rgn0 >>  /tmp/log_proc.txt


echo " ************  Start Dump Isp Message ! ************** "  >>  /tmp/log_proc.txt

echo " cat /sys/class/mstar/ispmid0/ispmid_info "  >> /tmp/log_proc.txt
cat /sys/class/mstar/ispmid0/ispmid_info >> /tmp/log_proc.txt
echo " cat /sys/class/mstar/ispmid0/ispmid1_info "  >> /tmp/log_proc.txt
cat /sys/class/mstar/ispmid0/ispmid1_info  >> /tmp/log_proc.txt

echo " cat /sys/class/mstar/isp0/isp_ints  1"  >> /tmp/log_proc.txt
cat /sys/class/mstar/isp0/isp_ints  >> /tmp/log_proc.txt
echo " cat /sys/class/mstar/isp0/isp_ints  2 "  >> /tmp/log_proc.txt
cat /sys/class/mstar/isp0/isp_ints   >> /tmp/log_proc.txt
echo " cat /sys/class/mstar/isp0/isp_ints  3 "  >> /tmp/log_proc.txt
cat /sys/class/mstar/isp0/isp_ints  >> /tmp/log_proc.txt

echo " cat /sys/class/mstar/isp1/isp_ints  1"  >> /tmp/log_proc.txt
cat /sys/class/mstar/isp1/isp_ints  >> /tmp/log_proc.txt
echo " cat /sys/class/mstar/isp1/isp_ints  2 "  >> /tmp/log_proc.txt
cat /sys/class/mstar/isp1/isp_ints  >> /tmp/log_proc.txt
echo " cat /sys/class/mstar/isp1/isp_ints  3 "  >> /tmp/log_proc.txt
cat /sys/class/mstar/isp1/isp_ints  >> /tmp/log_proc.txt


echo "  cat /sys/class/mstar/ispscl0/ispscl_ints  1"  >> /tmp/log_proc.txt
cat /sys/class/mstar/ispscl0/ispscl_ints   >> /tmp/log_proc.txt
echo "  cat /sys/class/mstar/ispscl0/ispscl_ints  2 "  >> /tmp/log_proc.txt
cat /sys/class/mstar/ispscl0/ispscl_ints   >> /tmp/log_proc.txt
echo " cat /sys/class/mstar/ispscl0/ispscl_ints  3 "  >> /tmp/log_proc.txt
cat /sys/class/mstar/ispscl0/ispscl_ints   >> /tmp/log_proc.txt

echo " cat /sys/class/mstar/ispscl1/ispscl_ints  1"  >> /tmp/log_proc.txt
cat /sys/class/mstar/ispscl1/ispscl_ints   >> /tmp/log_proc.txt
echo " cat /sys/class/mstar/ispscl1/ispscl_ints  2 "  >> /tmp/log_proc.txt
cat /sys/class/mstar/ispscl1/ispscl_ints   >> /tmp/log_proc.txt
echo "cat /sys/class/mstar/ispscl1/ispscl_ints  3 "  >> /tmp/log_proc.txt
cat /sys/class/mstar/ispscl1/ispscl_ints   >> /tmp/log_proc.txt

echo " ************  Start Dump Reg Message ! ************** "  >>  /tmp/log_proc.txt
echo "   /customer/riu_r 1304 !  "  >>  /tmp/log_proc.txt
/customer/riu_r 1304 >> /tmp/log_proc.txt
echo "   /customer/riu_r 1306 !  "  >>  /tmp/log_proc.txt
/customer/riu_r 1306 >> /tmp/log_proc.txt
echo "   /customer/riu_r 1704 !  "  >>  /tmp/log_proc.txt
/customer/riu_r 1704 >> /tmp/log_proc.txt
echo "   /customer/riu_r 1706 !  "  >>  /tmp/log_proc.txt
/customer/riu_r 1706 >> /tmp/log_proc.txt

cp -f /tmp/log_proc.txt /mnt



