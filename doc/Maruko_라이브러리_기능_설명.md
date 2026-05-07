# Maruko SDK 라이브러리·커널 모듈 기능 설명

## 출처

- 원본 CSV: `Maruko库功能说明.csv`  (예: `TOOLS_SDK_202030831/Maruko-ILS00_TINY_V1.2.0/reference(excel)/Library description/`)
- 본 문서의 **기능 설명**은 동일 자료의 레퍼런스 표·공개 요약을 바탕으로 정리했습니다.
- 원본 CSV 파일의 한글/중국어 본문은 저장 인코딩 문제로 자동 추출이 불안정할 수 있어, **파일명 목록은 CSV와 동일**하게 두고 설명은 정제본만 표에 넣었습니다.

---

## 사용자 공간 라이브러리 (`.so`)

| 파일 | 원문 요약(중국어) | 한국어 설명 |
| --- | --- | --- |
| `libADAS_LINUX.so` | 智能辅助驾驶 | 지능형 보조 주행(ADAS) |
| `libAEC_LINUX.so` | 回声消除 | 음향 에코 제거(AEC) |
| `libAED_LINUX.so` | 异常检测 | 이상(이벤트) 검출 |
| `libAPC_LINUX.so` | 音频算法库 | 오디오 알고리즘 라이브러리 |
| `libBF_2MIC_LINUX.so` | Beamforming, 波束成形 | 빔포밍(2채널 마이크) |
| `libBF_4MIC_LINUX.so` | Beamforming, 波束成形 | 빔포밍(4채널 마이크) |
| `libMD_LINUX.so` | 基于IVE算子库，提供运动侦测的算法模块 | IVE 연산 기반 움직임 검출(MD) |
| `libOD_LINUX.so` | 基于IVE算子库，提供遮挡侦测的算法模块 | IVE 연산 기반 가림(occlusion) 검출(OD) |
| `libSRC_LINUX.so` | 重采样 | 오디오 리샘플링 |
| `libSSL_2MIC_LINUX.so` | 声源定位 | 음원 위치 추정(SSL, 2마이크) |
| `libSSL_4MIC_LINUX.so` | 声源定位 | 음원 위치 추정(SSL, 4마이크) |
| `libVG_LINUX.so` | 基于IVE算子库，提供入侵报警策略的算法模块 | IVE 연산 기반 침입 경보(VIG) 전략 알고리즘 |
| `libcam_fs_wrapper.so` | 摄像头文件系统 | 카메라 파일시스템 래퍼 |
| `libcam_os_wrapper.so` | 封装通用系统接口，供前端 cam(Oshm) 调用 | 공통 OS API 래퍼(프론트엔드 cam/Oshm 연동) |
| `libcus3a.so` | 为客户提供3A相关接口 | 고객용 3A(AE/AWB/AF 등) 관련 인터페이스 |
| `libfbc_decode.so` | MI layer 10bit 解码；IQ tool dfview 图形显示调用 | MI 레이어 10bit 디코딩(IQ 도구 dfview 등에서 사용) |
| `libg711.so` | G711 编解码 | G.711 코덱 |
| `libg726.so` | G726 编解码 | G.726 코덱 |
| `libispalgo.so` | ISP 控制库算法库 | ISP 제어·알고리즘 라이브러리 |
| `libmi_ai.so` | 音频输入(Audio Input)；采集、发送及音频算法处理 | 오디오 입력(AI): 디바이스 연동·전송·음성 알고리즘 처리 |
| `libmi_ao.so` | 音频输出(Audio Output)；播放及音频算法处理 | 오디오 출력(AO): 재생·음성 알고리즘 처리 |
| `libmi_common.so` | MI 通用功能模块 | MI 공통 모듈 |
| `libmi_disp.so` | 视频显示单元；图层合成与 HDMI/VGA/MIPI/TTL 输出 | 디스플레이(DISP): 레이어 합성·HDMI/VGA/MIPI/TTL 출력 |
| `libmi_dummy.so` | Kernel 层测试 mi_sys | 커널 측 mi_sys 테스트용 더미 모듈 |
| `libmi_fb.so` | Linux Framebuffer 显示；缓冲区与 Alpha/ColorKey 等 | 프레임버퍼(FB): 버퍼 관리·Alpha·ColorKey·영역 설정 |
| `libmi_gfx.so` | GFX 图形引擎硬件加速；绘制、缩放、旋转、Alpha、Color Key 等 | GFX: UI 그래픽 가속(그리기·스케일·회전·알파·컬러키 등) |
| `libmi_gyro.so` | 陀螺仪功能 | 자이로스코프 |
| `libmi_hdmi.so` | 高清多媒体接口 | HDMI 입출력 |
| `libmi_ipu.so` | MI IPU：AI 智能控制单元相关功能 | MI IPU: AI 연산 제어 |
| `libmi_iqserver.so` | IQ 服务器测试模块 | IQ 서버 테스트 모듈 |
| `libmi_isp.so` | 图像信号处理；HDR、3D/2D 降噪、3A、WDR 等 | ISP: HDR·3D/2D NR·3A·WDR 등 영상 신호 처리 |
| `libmi_ive.so` | 视频智能分析算子引擎基础支持 | 영상 지능 분석(IVE) 기본 연산 |
| `libmi_jpd.so` | JPD 硬解码；拍照、对讲等场景 | JPD 하드웨어 디코드(캡처·인터컴 등) |
| `libmi_ldc.so` | 镜头畸变校正 | 렌즈 왜곡 보정(LDC) |
| `libmi_mipitx.so` | MIPI TX | MIPI TX 송신 |
| `libmi_panel.so` | 点屏 / Panel | 패널 구동 |
| `libmi_pspi.so` | PSPI 与 sensor；对接 PSPI/panel | PSPI·센서·패널 연동 |
| `libmi_rgn.so` | 视频 OSD 叠加 | 영상 OSD(RGN) 오버레이 |
| `libmi_scl.so` | 缩放、裁剪、输出端口；mirror/flip 等 | 스케일·크롭·출력 포트(SCL) |
| `libmi_sed.so` | 智能相关扩展 | 지능형 관련 확장(SE 등 문맥에 따름) |
| `libmi_sensor.so` | Sensor 驱动抽象 | 센서(SENSOR) 모듈 |
| `libmi_shadow.so` | Shadow 相关 | Shadow 레지스터/디버그 보조 |
| `libmi_sys.so` | MI 系统核心；内存、BIND、公共调用 | MI 시스템 코어(mi_sys) |
| `libmi_vdec.so` | 视频解码相关 | 비디오 디코더(VDEC) |
| `libmi_vdf.so` | 封装 MD、OD、VIG 等算法 | MD·OD·VIG 등 VDF 알고리즘 묶음 |
| `libmi_vdisp.so` | Virtual Display：多路 YUV/RGB 拼接整幅画面 | 가상 디스플레이: 다채널 YUV/RGB 합성 |
| `libmi_venc.so` | 视频编码；H.264/H.265 参数配置与查询 | 비디오 인코더(VENC): H.264/H.265 설정·조회 |
| `libmi_vif.so` | 视频输入(VIF)；采集与控制解析 | 비디오 입력(VIF) |
| `ld-uClibc-1.0.31.so` | — | *(CSV에 설명 필드 없음 또는 미정리)* |
| `libatomic.so` | — | *(CSV에 설명 필드 없음 또는 미정리)* |
| `libgcc_s.so` | — | *(CSV에 설명 필드 없음 또는 미정리)* |
| `libncurses.so` | — | *(CSV에 설명 필드 없음 또는 미정리)* |
| `libstdc++.so` | — | *(CSV에 설명 필드 없음 또는 미정리)* |
| `libthread_db-1.0.31.so` | — | *(CSV에 설명 필드 없음 또는 미정리)* |
| `libuClibc-1.0.31.so` | — | *(CSV에 설명 필드 없음 또는 미정리)* |

## 커널 모듈 (`.ko`)

| 파일 | 원문 요약(중국어) | 한국어 설명 |
| --- | --- | --- |
| `mi_ai.ko` | MI 音频输入内核模块 | MI 오디오 입력(AI) 커널 모듈 |
| `mi_aio.ko` | MI 音频一体化内核模块 | MI 오디오 통합(AIO) 커널 모듈 |
| `mi_ao.ko` | MI 音频输出内核模块 | MI 오디오 출력(AO) 커널 모듈 |
| `mi_common.ko` | MI 公共内核模块 | MI 공통 커널 모듈 |
| `mi_disp.ko` | MI 显示内核模块 | MI 디스플레이(DISP) 커널 모듈 |
| `mi_fb.ko` | MI Framebuffer 内核模块 | MI 프레임버퍼(FB) 커널 모듈 |
| `mi_ipu.ko` | MI IPU 内核模块 | MI IPU 커널 모듈 |
| `mi_isp.ko` | MI ISP 内核模块 | MI ISP 커널 모듈 |
| `mi_ldc.ko` | MI LDC 内核模块 | MI 렌즈 왜곡 보정(LDC) 커널 모듈 |
| `mi_panel.ko` | MI Panel 内核模块 | MI 패널 커널 모듈 |
| `mi_rgn.ko` | MI RGN/OSD 内核模块 | MI RGN(OSD) 커널 모듈 |
| `mi_scl.ko` | MI SCL 内核模块 | MI 스케일러(SCL) 커널 모듈 |
| `mi_sensor.ko` | MI Sensor 内核模块 | MI 센서 커널 모듈 |
| `mi_shadow.ko` | MI Shadow 内核模块 | MI Shadow 커널 모듈 |
| `mi_sys.ko` | MI 系统内核模块 | MI 시스템(mi_sys) 커널 모듈 |
| `mi_vcodec.ko` | MI 视频编解码内核模块 | MI 비디오 코덱(VCodec) 커널 모듈 |
| `mi_venc.ko` | MI 视频编码内核模块 | MI 비디오 인코더(VENC) 커널 모듈 |
| `mi_vif.ko` | MI 视频输入内核模块 | MI 비디오 입력(VIF) 커널 모듈 |
| `sc430ai_MIPI.ko` | SC430AI Sensor 驱动 | SC430AI MIPI 센서 드라이버 |
| `cifs.ko` | CIFS/Samba 文件系统 | CIFS(삼바) 파일시스템 |
| `fat.ko` | FAT 文件系统 | FAT 파일시스템 |
| `vfat.ko` | VFAT；U 盘常用 | VFAT(USB 저장소 등) |
| `grace.ko` | NFS | NFS grace(락 관련) |
| `kdrv_sdmmc.ko` | SD 卡驱动 | SD/MMC 호스트 드라이버 |
| `libarc4.ko` | RC4 | RC4 암호 |
| `libdes.ko` | DES | DES 암호 |
| `lockd.ko` | NFS lock manager | NFS lockd |
| `md4.ko` | MD4 摘要 | MD4 해시 알고리즘 모듈 |
| `mmc_block.ko` | MMC 块设备 | MMC 블록 레이어 |
| `mmc_core.ko` | MMC 核心 | MMC 코어 |
| `msdos.ko` | MS-DOS FS | MS-DOS 파일시스템 |
| `nfs.ko` | NFS | NFS 클라이언트/서버 스택 일부 |
| `nfs_ssc.ko` | NFS SSC 相关 | NFS SSC(SigmaStar Custom) 관련 |
| `nfsv2.ko` | NFSv2 | NFS 버전 2 |
| `nfsv3.ko` | NFSv3 | NFS 버전 3 |
| `nls_utf8.ko` | UTF-8 NLS | UTF-8 문자셋(NLS) |
| `ntfs.ko` | NTFS | NTFS 파일시스템 |
| `scsi_mod.ko` | SCSI 子系统 | SCSI 코어 |
| `sd_mod.ko` | SCSI 磁盘 | SCSI 디스크(sd) |
| `seqiv.ko` | seqiv AEAD | 커널 crypto seqiv(일련번호 IV) |
| `sunrpc.ko` | SUN RPC | NFS 등에서 쓰는 SUN-RPC |
| `phy-sstar-u2phy.ko` | USB2 PHY | SigmaStar USB2 PHY 드라이버 |
| `usb-common.ko` | USB 公共层 | USB 공통 레이어 |
| `usb-storage.ko` | USB 大容量存储 | USB 매스 스토리지 |
| `usbcore.ko` | USB 核心 | USB 코어 |
| `ehci-hcd.ko` | USB EHCI 主机控制器 | USB 2.0 EHCI HCD |

## 기타 항목

CSV에 포함된 그 외 이름:

- `ld-uClibc.so.0`
- `ld-uClibc.so.1`
- `libatomic.so.1`
- `libatomic.so.1.2.0`
- `libc.so.0`
- `libc.so.1`
- `libc.so.6`
- `libgcc_s.so.1`
- `libncurses.so.6`
- `libncurses.so.6.1`
- `libstdc++.so.6`
- `libstdc++.so.6.0.26`
- `libthread_db.so.1`

---

## 부록: `libVG` vs `libVIG`

일부 문서에는 침입 경보 알고리즘 라이브러리가 `libVIG_LINUX.so` 로 표기되고, 배포 CSV에는 `libVG_LINUX.so` 로만 올라 있는 경우가 있습니다. 빌드 산출물 이름을 기준으로 맞추면 됩니다.
