#!/usr/bin/env python3
"""
ref_board / dev_board (또는 지정한 두 루트) 아래 모든 파일을 순회해 MD5를 비교합니다.

출력 컬럼: 파일이름, 위치(상대경로), ref md5, dev md5, 비고

사용 예:
  python compare_ref_dev_md5.py
  python compare_ref_dev_md5.py --ref 5.10_ref --dev 5.10_dev
  python compare_ref_dev_md5.py -o report.csv
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import sys
from pathlib import Path


def md5_file(path: Path, chunk: int = 1 << 20) -> str:
    h = hashlib.md5()
    with path.open("rb") as f:
        while True:
            b = f.read(chunk)
            if not b:
                break
            h.update(b)
    return h.hexdigest()


def collect_files(root: Path) -> dict[str, Path]:
    """상대 POSIX 경로(str) -> 절대 Path"""
    root = root.resolve()
    out: dict[str, Path] = {}
    if not root.is_dir():
        return out
    for p in root.rglob("*"):
        if p.is_file():
            rel = p.relative_to(root).as_posix()
            out[rel] = p
    return out


def remark(ref_h: str | None, dev_h: str | None) -> str:
    if ref_h is None and dev_h is None:
        return "둘 다 없음(비정상)"
    if ref_h is None:
        return "dev에만 존재"
    if dev_h is None:
        return "ref에만 존재"
    if ref_h == dev_h:
        return "일치"
    return "불일치"


def main() -> int:
    script_dir = Path(__file__).resolve().parent
    ap = argparse.ArgumentParser(description="ref/dev 보드 폴더 트리 MD5 비교")
    ap.add_argument(
        "--ref",
        type=Path,
        default=script_dir / "ref_board",
        help="ref 측 루트 디렉터리 (기본: test/ref_board)",
    )
    ap.add_argument(
        "--dev",
        type=Path,
        default=script_dir / "dev_board",
        help="dev 측 루트 디렉터리 (기본: test/dev_board)",
    )
    ap.add_argument(
        "-o",
        "--output",
        type=Path,
        default=None,
        help="CSV 저장 경로 (미지정 시 stdout만)",
    )
    args = ap.parse_args()

    ref_root = args.ref.expanduser().resolve()
    dev_root = args.dev.expanduser().resolve()

    ref_map = collect_files(ref_root)
    dev_map = collect_files(dev_root)
    all_rel = sorted(set(ref_map.keys()) | set(dev_map.keys()))

    rows: list[tuple[str, str, str, str, str]] = []
    for rel in all_rel:
        rp = ref_map.get(rel)
        dp = dev_map.get(rel)
        ref_h = md5_file(rp) if rp else None
        dev_h = md5_file(dp) if dp else None
        name = Path(rel).name
        rows.append(
            (
                name,
                rel,
                ref_h or "",
                dev_h or "",
                remark(ref_h, dev_h),
            )
        )

    fieldnames = ["파일이름", "위치", "ref md5sum", "dev md5sum", "비고"]
    out_lines = csv.writer(sys.stdout, lineterminator="\n")
    out_lines.writerow(fieldnames)
    for r in rows:
        out_lines.writerow(r)

    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        with args.output.open("w", newline="", encoding="utf-8-sig") as f:
            w = csv.writer(f, lineterminator="\n")
            w.writerow(fieldnames)
            w.writerows(rows)

    # 요약을 stderr로 (stdout은 순수 CSV로 파이프하기 좋게)
    mismatch = sum(1 for r in rows if r[4] == "불일치")
    only_ref = sum(1 for r in rows if r[4] == "ref에만 존재")
    only_dev = sum(1 for r in rows if r[4] == "dev에만 존재")
    print(f"# ref: {ref_root}", file=sys.stderr)
    print(f"# dev: {dev_root}", file=sys.stderr)
    print(f"# 파일 수(전체 경로 기준): {len(rows)}", file=sys.stderr)
    print(f"# 일치: {sum(1 for r in rows if r[4] == '일치')}", file=sys.stderr)
    print(f"# 불일치: {mismatch}", file=sys.stderr)
    print(f"# ref에만: {only_ref}", file=sys.stderr)
    print(f"# dev에만: {only_dev}", file=sys.stderr)
    if args.output:
        print(f"# CSV 저장: {args.output.resolve()}", file=sys.stderr)

    return 0


if __name__ == "__main__":
    sys.exit(main())
