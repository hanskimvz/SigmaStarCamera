# ref_board vs dev_board MD5 비교 보고서

**생성 기준:** `compare_ref_dev_md5.py` 실행 결과 (동일 상대 경로 파일의 MD5 쌍 비교)  
**ref 경로:** `test/ref_board`  
**dev 경로:** `test/dev_board`

## 1. 요약

| 구분 | 개수 |
|------|------|
| 전체 경로(행) | 169 |
| **일치** | 40 |
| **불일치** | 56 |
| **ref에만 존재** | 41 |
| **dev에만 존재** | 32 |

**해석:** `mi_all.ko`, `sc465sl_MIPI.ko`는 양쪽 동일이지만, **`/config/lib`의 ISP·MI·algo·IQ Server 라이브러리**와 **`isp_api.xml`**, **`iqfile*.bin` 조합**이 ref와 dev에서 크게 다릅니다. 이는 IQ Tool / `LoadIQFile` / AE·AWB·ISP IQ API 실패와 직결될 수 있는 구간입니다.

---

## 2. IQ · ISP · 튜닝 우선 점검 (불일치·단측 포함)

| 파일이름 | 위치 | ref md5sum | dev md5sum | 비고 |
|----------|------|------------|------------|------|
| isp_api.xml | config/iqfile/isp_api.xml | `ec6825cd0160489fc150d8319afecd76` | `ab3de24d4e44062765db4136cbf10e95` | **불일치** |
| iqfile0.bin | config/iqfile/iqfile0.bin | `5192b45f6ef2dbeaa8e46bf40fd9747a` | `1c66306962858c4912baee168b395f14` | **불일치** |
| iqfile1.bin | config/iqfile/iqfile1.bin | `5192b45f6ef2dbeaa8e46bf40fd9747a` | `1c66306962858c4912baee168b395f14` | **불일치** |
| iqfile2.bin | config/iqfile/iqfile2.bin | `5192b45f6ef2dbeaa8e46bf40fd9747a` | `1c66306962858c4912baee168b395f14` | **불일치** |
| iqfile3.bin | config/iqfile/iqfile3.bin | `5192b45f6ef2dbeaa8e46bf40fd9747a` | `1c66306962858c4912baee168b395f14` | **불일치** |
| V129.bin | config/iqfile/V129.bin | `1c66306962858c4912baee168b395f14` | (없음) | ref에만 |
| imx307_iqfile.bin | config/iqfile/imx307_iqfile.bin | `5192b45f6ef2dbeaa8e46bf40fd9747a` | (없음) | ref에만 |
| sc465sl_iqfile.bin | config/iqfile/sc465sl_iqfile.bin | (없음) | `1c66306962858c4912baee168b395f14` | dev에만 |
| libcus3a.so | config/lib/libcus3a.so | `404c4912a669370eeaeb47f492b0a509` | `f1243fef1e47d66480f8214ff9b651e1` | **불일치** |
| libispalgo.so | config/lib/libispalgo.so | `a7c9a8f94799e841434f31f3d5dc2d0b` | `121e60907f3a93f92da4e5af52ca4739` | **불일치** |
| libmi_iqserver.so | config/lib/libmi_iqserver.so | `75a861e2b44fe5b9603340c3b2427cc9` | `59b1a87ab50249bd2df3283390319b32` | **불일치** |
| libmi_isp.so | config/lib/libmi_isp.so | `d3744808bbbf580e682c644bf9c57485` | `9bc3bf6d5e89ae1421a8ae637cd1487a` | **불일치** |

> dev 쪽 `iqfile0~3` 해시가 ref의 `V129.bin`과 동일해 보이는 것은 **플랫폼/센서별 iq 슬롯에 다른 바이너리를 넣은 패키지 차이**를 시사합니다. **IQ 클라이언트와 `isp_api.xml`은 세트**이므로 ref와 맞출 때는 라이브러리 + XML + 해당 `iqfile` 계통을 함께 맞추는 것이 안전합니다.

---

## 3. 불일치 전체 (56건)

| 파일이름 | 위치 | ref md5sum | dev md5sum |
|----------|------|------------|------------|
| config.json | config/config.json | `dd69669fe2f0ff929e2bbbef413bea21` | `617de74b6b2e0d362ce7c6814dee2bd8` |
| iqfile0.bin | config/iqfile/iqfile0.bin | `5192b45f6ef2dbeaa8e46bf40fd9747a` | `1c66306962858c4912baee168b395f14` |
| iqfile1.bin | config/iqfile/iqfile1.bin | `5192b45f6ef2dbeaa8e46bf40fd9747a` | `1c66306962858c4912baee168b395f14` |
| iqfile2.bin | config/iqfile/iqfile2.bin | `5192b45f6ef2dbeaa8e46bf40fd9747a` | `1c66306962858c4912baee168b395f14` |
| iqfile3.bin | config/iqfile/iqfile3.bin | `5192b45f6ef2dbeaa8e46bf40fd9747a` | `1c66306962858c4912baee168b395f14` |
| isp_api.xml | config/iqfile/isp_api.xml | `ec6825cd0160489fc150d8319afecd76` | `ab3de24d4e44062765db4136cbf10e95` |
| libcam_fs_wrapper.so | config/lib/libcam_fs_wrapper.so | `2552f8e2be2afa55173f481fe1884583` | `d313fbb38019bfd4b07990d471c073b7` |
| libcam_os_wrapper.so | config/lib/libcam_os_wrapper.so | `d9dda361b1f084fab154c9c06a7dd0d6` | `723c6fe7dfa0cfe6a979e2a7088e1c57` |
| libcus3a.so | config/lib/libcus3a.so | `404c4912a669370eeaeb47f492b0a509` | `f1243fef1e47d66480f8214ff9b651e1` |
| libispalgo.so | config/lib/libispalgo.so | `a7c9a8f94799e841434f31f3d5dc2d0b` | `121e60907f3a93f92da4e5af52ca4739` |
| libmi_ai.so | config/lib/libmi_ai.so | `d426c2ab639812d7c73584b78b7db386` | `1657612b1bc53cfcc3e5ff0aaa112192` |
| libmi_ao.so | config/lib/libmi_ao.so | `cb0df760e1514dc905dfe7979e53b316` | `fcad55635578109afc10273d85bfd871` |
| libmi_common.so | config/lib/libmi_common.so | `fdbfc84daa879b2a1eaf905cbfc1777b` | `01eec49b744a64da58fa4c88a7d6ba9f` |
| libmi_ipu.so | config/lib/libmi_ipu.so | `d477013c1a32f89f60755be78506b468` | `436b3f9912e4d536a150dca6ab918a05` |
| libmi_iqserver.so | config/lib/libmi_iqserver.so | `75a861e2b44fe5b9603340c3b2427cc9` | `59b1a87ab50249bd2df3283390319b32` |
| libmi_isp.so | config/lib/libmi_isp.so | `d3744808bbbf580e682c644bf9c57485` | `9bc3bf6d5e89ae1421a8ae637cd1487a` |
| libmi_ive.so | config/lib/libmi_ive.so | `9c48373912d984ef18a951ce38681e62` | `d608368e2348855adab63667fc1d359c` |
| libmi_ldc.so | config/lib/libmi_ldc.so | `4648ce174ac53ff933ab597c798ab7d2` | `caddc3c560f4471f4563f19e93b147fd` |
| libmi_rgn.so | config/lib/libmi_rgn.so | `ccde9f0ce5a47f9a8c732a6e0d68b42b` | `fda9eec51d2f3c98d1f2ea4fb5fb0ff5` |
| libmi_scl.so | config/lib/libmi_scl.so | `0b5858a17043769c67cf3d5ff0abdb63` | `f05e664fce2885f0e135f3ee5304a6f2` |
| libmi_sensor.so | config/lib/libmi_sensor.so | `de7aeb8836d10dcc15540358a02b0e37` | `6a16ab7a19e3d1e2b11eb2047a2b6982` |
| libmi_shadow.so | config/lib/libmi_shadow.so | `7bc0b451784ed2a4ce5ee51a70b34dad` | `533dddc3c27c2653a3767e30618fe823` |
| libmi_sys.so | config/lib/libmi_sys.so | `1bbc5a4d8a290f1bd541130fc05ab960` | `5835e2d7d840f095cbc7036c74b8cd92` |
| libmi_vdf.so | config/lib/libmi_vdf.so | `ff0b73c3097bb67387c81a16d057a4e3` | `8589d2d15288f7faddf6c21042eeeb0b` |
| libmi_venc.so | config/lib/libmi_venc.so | `35f0b52eb3888aaa5c46442111578084` | `2461c47acf228a8a62fc6ceb21bdde50` |
| libmi_vif.so | config/lib/libmi_vif.so | `2a65967d25d6346ea856ed723eafc0b4` | `7d38d9bf396086a7b1f63b19b7c675de` |
| cifs.ko | config/modules/5.10/cifs.ko | `c4cf40e68f6aaadce77078cc3dd35eaa` | `9f7600b3206724dfef7c7eb87977c877` |
| ehci-hcd.ko | config/modules/5.10/ehci-hcd.ko | `84facf1c01214bfb786b303404ceb1b9` | `039398a9568cf4a95fb436f946de54a2` |
| fat.ko | config/modules/5.10/fat.ko | `b7bbd7f8266deb65c5e31876202a222e` | `9b96fbe9f1ddd898b3d526aa15fab5df` |
| grace.ko | config/modules/5.10/grace.ko | `7a27b97944fd9dfb31d9928b2c91b738` | `43b3c620c2b6cd16195223d948804663` |
| imx415_MIPI.ko | config/modules/5.10/imx415_MIPI.ko | `78403f4c057cc4c1ee7057be9af5b2cc` | `498a85beae85db6cf02887c63ad34194` |
| kdrv_sdmmc.ko | config/modules/5.10/kdrv_sdmmc.ko | `d3bcdc24c42981e7b92adce78c767af8` | `0818c63c2bf689c0e587c1d14d959fed` |
| libarc4.ko | config/modules/5.10/libarc4.ko | `9535c67b39b82fcb4e8ffe9cad411b80` | `a319428b4f5d6cbc41088d2a251af359` |
| libdes.ko | config/modules/5.10/libdes.ko | `97c491820ed38385a5cd3ef118b297a8` | `cca4b5739d7aacccf4543856f9f2a51b` |
| lockd.ko | config/modules/5.10/lockd.ko | `5074bc3bca1372481339d779b9ddf5be` | `ea01169a744c0df5a7951e218d78a8df` |
| md4.ko | config/modules/5.10/md4.ko | `94fd0c3681a397a664bea7daf24963cd` | `cff27e3d4338a0a56950b3efd14f24cd` |
| mmc_block.ko | config/modules/5.10/mmc_block.ko | `b39ca32f4990ef366245aae35dec9cbc` | `1a4faa33758df23895981724ce3d7223` |
| mmc_core.ko | config/modules/5.10/mmc_core.ko | `35b17858ff7afe950f8acda7e9ca5456` | `9d2df9f94f3f613f6a750f9d908f040d` |
| msdos.ko | config/modules/5.10/msdos.ko | `e93cbcc7be063c7fe9cc15fe91830065` | `530bc2d1f3e67a2c4fc374c89cccb6bf` |
| nfs.ko | config/modules/5.10/nfs.ko | `d1b9dc5a2cb05bbf8a1a727f76b776de` | `6e21da3f00c9d570fe8fe2f53d2f4907` |
| nfs_ssc.ko | config/modules/5.10/nfs_ssc.ko | `9d0e7dca1a4a646a57cffc03432f5861` | `072b21592e880deb50e0df6a7e4dad51` |
| nfsv2.ko | config/modules/5.10/nfsv2.ko | `d40ea6669501f53fc44d982dc0ca7c66` | `b0ef6f5f4a2d8127903c9d2d67a4fb74` |
| nfsv3.ko | config/modules/5.10/nfsv3.ko | `ef4be03242672e4b4f1928d4e61eb284` | `55ec859636fed0d074a0d1a13ddcaf8a` |
| nls_utf8.ko | config/modules/5.10/nls_utf8.ko | `8774e61536d8e059e1179d572bce334c` | `7171612812e85834d18143bcd468f6d9` |
| ntfs.ko | config/modules/5.10/ntfs.ko | `7616f8495486f69a198b0b24c5110d9c` | `e72a2701505fdb2ad035bbab6f8fe53a` |
| phy-sstar-u2phy.ko | config/modules/5.10/phy-sstar-u2phy.ko | `6b0fbdc11ec792c363069049385901c1` | `9f39099983b7068cb461a57215144e9a` |
| scsi_mod.ko | config/modules/5.10/scsi_mod.ko | `01171c31709ba4eb642cafdc911db681` | `c9321e0da0e26025c568a0e22de5afc2` |
| sd_mod.ko | config/modules/5.10/sd_mod.ko | `878a9d4cdd356521032d0ac56dccaa9d` | `7d61e290e9e68f4e1920648bfd46f695` |
| seqiv.ko | config/modules/5.10/seqiv.ko | `360ec33a6d6b77b981503d2347ac3332` | `0b7386c629b3606e7a0ae3fe5028353f` |
| sunrpc.ko | config/modules/5.10/sunrpc.ko | `ee8211eb46259d08e130501151c3f99c` | `caef997e7503621f963549036e68951f` |
| usb-common.ko | config/modules/5.10/usb-common.ko | `b5ad057e3314776c885b2a49155031e0` | `66ea5ba02559b5a6eddb68f9508d4934` |
| usb-storage.ko | config/modules/5.10/usb-storage.ko | `24b230c289c9e28dc26a866c14dc0cdb` | `d2217709af9bf92b91d4053ddd47b2f1` |
| usbcore.ko | config/modules/5.10/usbcore.ko | `1676585f3eba25a5962a8770248007be` | `0e768393fd76aa57c7dbc3a543adf6db` |
| vfat.ko | config/modules/5.10/vfat.ko | `e0b31af81a9497b9ad46e8b18d961c45` | `90e3ebd9ff08938679e09d6dee46b877` |
| config.json | customer/config.json | `dd69669fe2f0ff929e2bbbef413bea21` | `617de74b6b2e0d362ce7c6814dee2bd8` |
| demo.sh | customer/demo.sh | `7d692b3504d62da41055c86bbba23c30` | `7139f8e79055adad9889612bd8bbc46f` |

---

## 4. 일치 (40건) — 발췌

MI·ISP와 인접한 항목만 예시로 적습니다. 나머지는 스크립트 재실행 CSV로 확인할 수 있습니다.

| 위치 | 비고 |
|------|------|
| `config/modules/5.10/mi_all.ko` | 동일 |
| `config/modules/5.10/sc465sl_MIPI.ko` | 동일 |
| `config/lib/AEC_LICENSE_*.txt`, `APC_LICENSE_*.txt` | 동일 |
| `config/lib/libADAS_LINUX.so` ~ 일부 알고·오디오 라이브러리 | 동일 (출력 원본 참고) |
| `config/modparam.json` | 동일 |
| `config/venc_fw/chagall.bin` | 동일 |
| `config/dla/ipu_lfw.bin` | 동일 (빈 파일 MD5) |

---

## 5. ref에만 존재 (41건) — 요약

| 영역 | 내용 |
|------|------|
| 루트 | `bootlog.txt`, `config.tar`, `customer.tar`, `etc.tar` |
| iqfile | `V129.bin`, `imx307_iqfile.bin` |
| modules | `cfg80211.ko`, `firmware_class.ko`, `imx307_MIPI_36M*.ko`, `imx485_MIPI.ko`, `imx664/678`, `sc233/sc500ai/sc835hai` 등 |
| customer/mi_demo | `prog_rtsp_det`, `imx485_api.bin`, 각종 `*_param_snr0.ini`, OSD·모델 바이너리 등 **통째로 ref 측에만 편중** |
| init.d | `rcS`, `udhcpc.script` |

---

## 6. dev에만 존재 (32건) — 요약

| 영역 | 내용 |
|------|------|
| 루트 | `config_dev.tar`, `customer_dev.tar` |
| iqfile | `sc465sl_iqfile.bin` |
| modules | `imx307_MIPI.ko`, `sc230ai_MIPI.ko` |
| customer | `demo.sh`는 양쪽 있으나 MD5 불일치; `i2c_read_write`, `io_check_maruko`, `LCM/*.ini`, `misc/*`, `mixer/*`, `riu_r/w` 등 **UI·믹서·점검 도구** 위주 |

---

## 7. 재생성 방법

```bash
cd SigmaStarCamera/test
python3 compare_ref_dev_md5.py -o md5_compare_$(date +%Y%m%d).csv
```

CSV와 동일한 데이터를 바탕으로 본 문서를 갱신하면 됩니다.

---

## 8. 권장 액션 (IQ 이슈 기준)

1. **ref에서 동작 확인된 조합**으로 dev의 `/config/lib` 중 **`libmi_isp` / `libmi_iqserver` / `libcus3a` / `libispalgo` / `libmi_common` / `libmi_sys`** 등 실행 경로에 걸리는 `.so`를 통일하거나, 반대로 전부 dev SDK 한 벌로 재정렬합니다.  
2. **`config/iqfile/isp_api.xml`**과 **`iqfile*.bin` / 센서별 iq 바이너리**를 ref와 동일 세트로 맞춥니다.  
3. **`mi_all.ko` / `sc465sl_MIPI.ko`는 이미 동일**이므로, 남는 차이는 위 사용자 공간·iqfile 쪽 정렬이 우선입니다.
