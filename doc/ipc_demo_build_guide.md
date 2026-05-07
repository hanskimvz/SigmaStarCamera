# IPC Demo 빌드 가이드 및 동작 설명

## 1. IPC Demo란?

IPC Demo는 SigmaStar Infinity6C 칩셋 기반의 IP 카메라 레퍼런스 애플리케이션 모음입니다. 이 데모들을 보드에 올리면 카메라 센서로부터 영상을 입력받아 처리하고 인코딩하는 등의 동작을 테스트할 수 있습니다.

### 1.1. 빌드 결과물의 역할

빌드하면 `out/` 디렉토리에 ARM용 실행 바이너리와 커널 모듈(`.ko`)이 생성됩니다. 이것들은 호스트 PC(x86)에서는 실행할 수 없고, 반드시 SigmaStar 보드에 전송하여 실행해야 합니다.

### 1.2. 각 모듈의 역할

| 카테고리 | 바이너리 | 설명 |
|----------|----------|------|
| vif | vif | Video Input Front-end. 센서에서 영상 데이터를 받아오는 입구 |
| isp | isp | Image Signal Processor. 센서 RAW 데이터를 처리 (노이즈 제거, 색보정 등) |
| scl | scl | Scaler. 영상 해상도 변환 (리사이즈) |
| ldc | ldc | Lens Distortion Correction. 렌즈 왜곡 보정 |
| venc | venc | Video Encoder. H.264/H.265 영상 인코딩 |
| ai/ao | ai, ao | Audio Input/Output. 마이크 입력 및 스피커 출력 |
| dla | dla_classify, dla_detect 등 | Deep Learning Accelerator. AI 추론 (객체 분류/검출) |
| HW | gpio, i2c, uart, watchdog 등 | 하드웨어 주변장치 제어 테스트 |

### 1.3. 보드에서의 동작 흐름

```text
센서(SC430AI 등)
    ↓
  [VIF] 영상 입력
    ↓
  [ISP] 이미지 처리
    ↓
  [SCL] 해상도 조절
    ↓
  [VENC] H.264/H.265 인코딩
    ↓
  파일 저장 또는 네트워크 스트리밍
```

## 2. 보드에 다운로드하여 실행하는 방법

### 2.1. 사전 조건

SDK 펌웨어(`make image`)가 보드에 이미 플래싱되어 있어야 합니다. 펌웨어에는 커널, MI 모듈 드라이버, 라이브러리 등이 포함되어 있습니다.

### 2.2. 보드로 파일 전송

```bash
# 보드 IP가 192.168.1.10이라고 가정
# NFS 마운트 방식
mount -t nfs -o nolock 192.168.1.100:/ext/SigmaStarCamera/ipc_demo/maruko/out /mnt/nfs

# 또는 TFTP 방식
tftp -g -r vif 192.168.1.100

# 또는 SCP 방식 (보드에 SSH가 있는 경우)
scp /ext/SigmaStarCamera/ipc_demo/maruko/out/* root@192.168.1.10:/tmp/
```

### 2.3. 보드에서 실행

```bash
# 보드의 시리얼 콘솔에서
cd /tmp   # 또는 전송한 경로

# MI 모듈이 로드되어 있는지 확인
lsmod

# VIF 데모 실행 예시
chmod +x vif
./vif

# VENC 데모 실행 예시
./venc
```

## 3. IPC Demo 빌드 환경 설정

### 3.1. auto_release.sh 수정 사항

원본 ipc_demo는 uclibc 9.1.0 기반으로 작성되어 있으나, SDK가 glibc 11.1.0으로 빌드되었으므로 일치시켜야 합니다.

**수정할 변수들:**

```bash
# SDK 경로 (절대경로 필수)
MI_PROJECT=/ext/SigmaStarCamera/Maruko-ILS00_TINY_V1.5.0/project
KERNEL_DIR=/ext/SigmaStarCamera/Maruko-ILS00_TINY_V1.5.0/kernel

# 툴체인 PATH
export PATH=/tools/bin/:$PATH

# 크로스 컴파일러
declare -x ARCH="arm"
declare -x CROSS_COMPILE="arm-linux-gnueabihf-sigmastar-11.1.0-"

# 툴체인 종류 및 버전 (glibc로 변경)
TOOLCHAIN=glibc
TOOLCHAIN_VERSION=11.1.0
```

**주의:** `TOOLCHAIN`과 `TOOLCHAIN_VERSION`이 파일 내에 중복 선언되지 않도록 확인합니다. 뒤쪽에 uclibc/9.1.0 설정이 남아 있으면 덮어쓰므로, 반드시 주석 처리합니다.

```bash
#TOOLCHAIN=uclibc
#TOOLCHAIN_VERSION=9.1.0
```

### 3.2. CMakeLists.txt 크로스 컴파일러 수정

`demo_CMakeLists.txt`와 `CMakeLists.txt` 모두 수정합니다. (`auto_release.sh` 실행 시 `demo_CMakeLists.txt`가 `CMakeLists.txt`로 복사되므로 둘 다 수정해야 합니다.)

```bash
sed -i 's/arm-sigmastar-linux-uclibcgnueabihf-gcc/arm-linux-gnueabihf-sigmastar-11.1.0-gcc/' demo_CMakeLists.txt
sed -i 's/arm-sigmastar-linux-uclibcgnueabihf-g++/arm-linux-gnueabihf-sigmastar-11.1.0-g++/' demo_CMakeLists.txt
```

### 3.3. DLA 모듈 OpenCV 라이브러리 변경

DLA 모듈이 uclibc용 OpenCV를 참조하고 있으므로 glibc 11.1.0 버전으로 교체합니다.

```bash
# glibc용 opencv 복사
cp -rf /ext/SigmaStarCamera/maruko_v2.0.1_mi_demo/mi_demo/prebuild_libs/opencv/glibc_static_lib_11.1.0 \
    /ext/SigmaStarCamera/ipc_demo/maruko/libs/third-party/opencv/

# dla.cmake 수정
sed -i 's|uclibc_static_lib_9.1.0|glibc_static_lib_11.1.0|g' source/dla/dla.cmake
```

### 3.4. source.cmake 링크 라이브러리 추가

카메라 모듈(vif, isp, scl, ldc, venc)에서 `libmi_ipu.so`와 `libcam_fs_wrapper.so`가 필요합니다.

**`source/source.cmake` 수정 전:**

```cmake
target_link_libraries(${TARGET_NAME} libmi_ldc.so libmi_vif.so libmi_common.so libcam_os_wrapper.so libmi_sys.so libmi_isp.so libmi_scl.so libmi_venc.so libmi_sensor.so libcus3a.so libispalgo.so libm.so -pthread)
```

**수정 후:**

```cmake
target_link_libraries(${TARGET_NAME} libmi_ldc.so libmi_vif.so libmi_common.so libcam_os_wrapper.so libcam_fs_wrapper.so libmi_sys.so libmi_isp.so libmi_ipu.so libmi_scl.so libmi_venc.so libmi_sensor.so libcus3a.so libispalgo.so libm.so -pthread)
```

**sed 명령으로 처리:**

```bash
sed -i 's/libcam_os_wrapper.so/libcam_os_wrapper.so libcam_fs_wrapper.so/' source/source.cmake
sed -i 's/libmi_isp.so libmi_scl.so/libmi_isp.so libmi_ipu.so libmi_scl.so/' source/source.cmake
```

## 4. 빌드 실행

```bash
cd /ext/SigmaStarCamera/ipc_demo/maruko

# 이전 빌드 캐시 삭제
rm -rf build

# 빌드 실행
./auto_release.sh
```

### 4.1. 빌드 결과 확인

```bash
ls out/
```

정상 빌드 시 생성되는 파일:

| 파일 | 유형 |
|------|------|
| vif, isp, scl, ldc, venc | 카메라 파이프라인 데모 바이너리 |
| ai, ao | 오디오 데모 바이너리 |
| audio_alg_AEC_demo, audio_alg_APC_demo 등 | 오디오 알고리즘 데모 |
| audio_g711_codec_demo, audio_g726_codec_demo | 오디오 코덱 데모 |
| dla_classify, dla_detect, dla_simulator 등 | DLA 추론 데모 |
| gpio_demo, i2c_demo, uart_demo 등 | HW 제어 데모 |
| gpio_ko_demo.ko, timer_ko_demo.ko | 커널 모듈 |

### 4.2. 무시 가능한 경고

| 메시지 | 설명 |
|--------|------|
| CMake Warning: No project() command | CMakeLists.txt에 project() 미선언. 동작에 무영향 |
| The dependency target "common_static" does not exist | timer_demo의 의존성 문제. timer_demo만 링크 시 영향 |
| The C compiler identification is GNU 9.4.0 | CMake 초기화 시 호스트 컴파일러 감지. 실제 빌드는 크로스 컴파일러 사용 |
| 각종 -Wunused-variable, -Wsign-compare | 소스코드의 경고. 동작에 무영향 |

## 5. 수정 파일 요약

| 파일 | 수정 내용 |
|------|-----------|
| auto_release.sh | `MI_PROJECT`, `KERNEL_DIR` 경로 설정, `TOOLCHAIN=glibc`, `TOOLCHAIN_VERSION=11.1.0`, `CROSS_COMPILE` 변경, 중복 uclibc 설정 주석 처리 |
| demo_CMakeLists.txt | `CMAKE_C_COMPILER`, `CMAKE_CXX_COMPILER`를 glibc 11.1.0 툴체인으로 변경 |
| source/source.cmake | `target_link_libraries`에 `libmi_ipu.so`, `libcam_fs_wrapper.so` 추가 |
| source/dla/dla.cmake | OpenCV 경로를 `uclibc_static_lib_9.1.0` → `glibc_static_lib_11.1.0`으로 변경 |
