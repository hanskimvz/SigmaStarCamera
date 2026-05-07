## SigmaStar 카메라 SDK 개발 환경 세팅 가이드

### 환경 정보

| 항목 | 내용 |
|------|------|
| SDK | Maruko-ILS00_TINY_V1.5.0 |
| 칩셋 | i6c (Infinity6C / Maruko) QFN88 |
| 보드 | SSC027A-S01A |
| 커널 | Linux 5.10.61 |
| U-Boot | 201501 |
| 툴체인 | arm-linux-gnueabihf-sigmastar-11.1.0 (glibc) |
| 플래시 | 16M NOR, squashfs |
| 호스트 OS | Ubuntu (Python 3.8 기반) |

### 1. 필수 패키지 설치

```bash
# 기본 빌드 도구
apt-get install build-essential libncurses5-dev bison flex libssl-dev bc \
    u-boot-tools mtd-utils device-tree-compiler git texinfo gawk wget \
    unzip rsync cpio

# SigmaStar SDK 추가 의존 패키지
apt-get install libtinfo5          # clang-format/clang-tidy 의존 라이브러리
apt-get install python-is-python3  # python 명령어 심볼릭 링크 (Python 2 → 3 호환)
```

`libtinfo5` 패키지를 찾을 수 없는 경우 (최신 Ubuntu):

```bash
ln -s /usr/lib/x86_64-linux-gnu/libtinfo.so.6 /usr/lib/x86_64-linux-gnu/libtinfo.so.5
```

### 2. 크로스 컴파일러 PATH 등록

#### 2-1. 설치된 툴체인 위치 찾기

```bash
sudo find / -name "arm-linux-gnueabihf-gcc" 2>/dev/null
```

#### 2-2. PATH 영구 등록

```bash
# ~/.bashrc 끝에 추가 (경로는 실제 설치 위치에 맞게 수정)
echo 'export PATH=/tools/bin:$PATH' >> ~/.bashrc
source ~/.bashrc
```

#### 2-3. 등록 확인

```bash
which arm-linux-gnueabihf-gcc
arm-linux-gnueabihf-gcc --version
```

### 3. Python 2 → Python 3 호환성 패치

이 SDK는 Python 2 기반으로 작성되어 있어 Python 3 환경에서 커널 빌드 시 여러 에러가 발생합니다. 아래 두 파일을 수정해야 합니다.

#### 3-1. kernel/scripts/ms_builtin_dtb_update.py

**에러 증상:**

```
UnicodeDecodeError: 'utf-8' codec can't decode byte 0xd0 in position 0
TypeError: a bytes-like object is required, not 'str'
```

**수정 내용:**

```bash
# 20번째 줄: 문자열 리터럴을 바이트 리터럴로 변경
sed -i "s/name='#MS_DTB#'/name=b'#MS_DTB#'/" kernel/scripts/ms_builtin_dtb_update.py

# 22번째 줄: DTB 파일을 바이너리 모드로 열기
sed -i "s/open(sys.argv\[2\])/open(sys.argv[2],'rb')/" kernel/scripts/ms_builtin_dtb_update.py
```

**수정 전후 비교:**

```python
# 수정 전 (Python 2)
name='#MS_DTB#'
dtb_file=open(sys.argv[2])

# 수정 후 (Python 3)
name=b'#MS_DTB#'
dtb_file=open(sys.argv[2],"rb")
```

#### 3-2. kernel/scripts/ms_bin_option_update_int.py

**에러 증상:**

```
NameError: name 'long' is not defined
TypeError: a bytes-like object is required, not 'str'
```

**수정 내용:**

```bash
# 25번째 줄: long() → int() (Python 3에서 long 타입 제거됨)
sed -i 's/=long(/=int(/' kernel/scripts/ms_bin_option_update_int.py

# 21번째 줄: 문자열을 바이트로 인코딩
sed -i "s/name=sys.argv\[2\]/name=sys.argv[2].encode()/" kernel/scripts/ms_bin_option_update_int.py
```

**수정 전후 비교:**

```python
# 수정 전 (Python 2)
name=sys.argv[2]
value=long(sys.argv[3])

# 수정 후 (Python 3)
name=sys.argv[2].encode()
value=int(sys.argv[3])
```

#### 3-3. 수정 확인

```bash
grep -n "name=" kernel/scripts/ms_builtin_dtb_update.py
grep -n "name=\|long\|int(" kernel/scripts/ms_bin_option_update_int.py
```

### 4. 빌드 절차

#### 4-1. defconfig 적용

```bash
cd /ext/SigmaStarCamera/Maruko-ILS00_TINY_V1.5.0/project
make ipc_i6c.nor.glibc-11.1.0-squashfs.027a.64.qfn88_defconfig
```

#### 4-2. 전체 이미지 빌드

```bash
make image -j$(nproc)
```

#### 4-3. 부분 빌드 (문제 해결 후 이어서 빌드할 때)

```bash
# 커널만 다시 빌드
make linux-kernel

# 이미지 패킹만 다시 실행
make image_install
```

### 5. 빌드 시 무시해도 되는 경고

| 메시지 | 원인 | 영향 |
|--------|------|------|
| fatal: not a git repository | SDK 디렉토리가 git 저장소가 아님 | 없음 (버전 문자열만 비어 있음) |
| /usr/bin/strings: 'gitInformation.txt': No such file | git 정보 파일 부재 | 없음 |
| Image-fpga DTB: IndexError: list index out of range | FPGA 보드용 DTB 인자 없음 | 없음 (실제 보드에서는 불필요) |
| warning: '%s' directive writing likely 7 or more bytes | kconfig 빌드 시 컴파일러 경고 | 없음 |
| NOP: command not found | Makefile 스크립트 오타 | 없음 |

### 6. 빌드 성공 확인

```bash
# 커널 이미지 생성 확인
ls -la kernel/arch/arm/boot/uImage*

# 최종 펌웨어 이미지 확인
ls -la project/image/output/images/
```

정상 빌드 시 `image/output/images/` 디렉토리에 `IPL.bin`, `u-boot.bin`, `kernel`, `rootfs.sqfs`, `miservice.sqfs`, `customer.jffs2` 등의 파일이 생성됩니다.

### 7. 문제 해결 요약

| 순서 | 에러 | 해결 방법 |
|------|------|-----------|
| 1 | libtinfo.so.5: cannot open shared object file | `apt-get install libtinfo5` |
| 2 | python: command not found | `apt-get install python-is-python3` |
| 3 | NameError: name 'long' is not defined | `long()` → `int()` 변경 |
| 4 | UnicodeDecodeError: 'utf-8' codec can't decode | `open()` → `open(..., "rb")` 변경 |
| 5 | TypeError: a bytes-like object is required | 문자열 → 바이트 리터럴(`b'...'` / `.encode()`) 변경 |
