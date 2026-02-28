#!/usr/bin/env python3
from __future__ import annotations

import argparse
import re
from dataclasses import dataclass, field
from pathlib import Path


@dataclass
class Signal:
    name: str
    bit: int
    length: int
    order: str
    signed: bool
    factor: float
    offset: float


@dataclass
class Message:
    frame_id: int
    name: str
    dlc: int
    signals: list[Signal] = field(default_factory=list)


BO_RE = re.compile(r"^BO_\s+(\d+)\s+(\w+)\s*:\s*(\d+)\s+\w+")
SG_RE = re.compile(
    r"^SG_\s+(\w+)\s*:\s*(\d+)\|(\d+)@([01])([+-])\s*"
    r"\(([+-]?[\d.]+),([+-]?[\d.]+)\)\s*\[[^\]]+\]\s*\".*\""
)


def pascal(name: str) -> str:
    return "".join(p[:1].upper() + p[1:].lower() for p in re.split(r"[^a-zA-Z0-9]", name) if p)


def parse_dbc(path: Path) -> list[Message]:
    msgs: list[Message] = []
    cur: Message | None = None
    for raw in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        line = raw.strip()
        if not line:
            continue
        bo = BO_RE.match(line)
        if bo:
            cur = Message(int(bo.group(1)), bo.group(2), int(bo.group(3)))
            msgs.append(cur)
            continue
        sg = SG_RE.match(line)
        if sg and cur is not None:
            cur.signals.append(
                Signal(
                    name=sg.group(1),
                    bit=int(sg.group(2)),
                    length=int(sg.group(3)),
                    order="intel" if sg.group(4) == "1" else "motorola",
                    signed=sg.group(5) == "-",
                    factor=float(sg.group(6)),
                    offset=float(sg.group(7)),
                )
            )
    return msgs


def get_byte_info(sig: Signal) -> list[dict[str, int]]:
    bit = sig.bit
    left_len = sig.length
    byte_idx = bit // 8
    bit_start = bit % 8
    out: list[dict[str, int]] = []
    if sig.order == "motorola":
        while left_len > 0:
            width = min(bit_start + 1, left_len)
            out.append({"byte": byte_idx, "len": width, "start": bit_start - width + 1})
            left_len -= width
            byte_idx += 1
            bit_start = 7
    else:
        while left_len > 0:
            width = min(8 - bit_start, left_len)
            out.append({"byte": byte_idx, "len": width, "start": bit_start})
            left_len -= width
            byte_idx += 1
            bit_start = 0
        out.reverse()
    return out


def gen_header(ns: str, cls: str, msgs: list[Message]) -> str:
    lines = ["#pragma once", "", "#include <array>", "#include <cstddef>", "#include <cstdint>", "", f"namespace {ns} {{", "", f"class {cls} final {{", " public:"]
    for m in msgs:
        mcls = pascal(m.name)
        lines += [f"  struct {mcls} {{"]
        lines += [f"    double {s.name.lower()} = 0.0;" for s in m.signals]
        lines += [
            "  };",
            f"  static constexpr std::uint32_t ID_{mcls.upper()} = 0x{m.frame_id:X};",
            f"  static std::array<std::uint8_t, {m.dlc}> Encode{mcls}(const {mcls}& input);",
            f"  static {mcls} Decode{mcls}(const std::uint8_t* data, std::size_t len);",
            "",
        ]
    lines += ["};", "", f"}}  // namespace {ns}", ""]
    return "\n".join(lines)


def gen_encode_signal(sig: Signal, value_expr: str, idx: int) -> list[str]:
    infos = list(get_byte_info(sig))
    infos.reverse()
    xname = f"x_{idx}"
    lines = [f"  std::int32_t {xname} = static_cast<std::int32_t>(({value_expr} - {sig.offset}) / {sig.factor});"]
    for i, inf in enumerate(infos):
        mask = hex((1 << inf["len"]) - 1)
        lines += [
            f"  std::uint8_t t_{idx}_{i} = static_cast<std::uint8_t>({xname} & {mask});",
            f"  Byte(out.data() + {inf['byte']}).set_value(t_{idx}_{i}, {inf['start']}, {inf['len']});",
        ]
        if i != len(infos) - 1:
            lines += [f"  {xname} >>= {inf['len']};"]
    return lines


def gen_decode_signal(sig: Signal, out_expr: str, idx: int) -> list[str]:
    infos = get_byte_info(sig)
    xname = f"x_{idx}"
    lines = [f"  std::int32_t {xname} = 0;"]
    for i, inf in enumerate(infos):
        if i == 0:
            lines += [f"  {xname} = Byte(data + {inf['byte']}).get_byte({inf['start']}, {inf['len']});"]
        else:
            lines += [
                f"  {xname} = ({xname} << {inf['len']}) | Byte(data + {inf['byte']}).get_byte({inf['start']}, {inf['len']});"
            ]
    if sig.signed:
        shift = 32 - sig.length
        lines += [f"  {xname} <<= {shift};", f"  {xname} >>= {shift};"]
    lines += [f"  {out_expr} = static_cast<double>({xname}) * {sig.factor} + {sig.offset};"]
    return lines


def gen_source(ns: str, cls: str, header_name: str, msgs: list[Message]) -> str:
    lines = [
        f"#include \"{header_name}\"",
        "",
        "#include <stdexcept>",
        "",
        "#include \"can_codec/byte.h\"",
        "",
        f"namespace {ns} {{",
        "",
    ]

    for m in msgs:
        mcls = pascal(m.name)
        lines += [
            f"std::array<std::uint8_t, {m.dlc}> {cls}::Encode{mcls}(const {mcls}& input) {{",
            f"  std::array<std::uint8_t, {m.dlc}> out{{}};",
        ]
        for idx, s in enumerate(m.signals):
            lines += gen_encode_signal(s, f"input.{s.name.lower()}", idx)
        lines += ["  return out;", "}", ""]

        lines += [
            f"{cls}::{mcls} {cls}::Decode{mcls}(const std::uint8_t* data, std::size_t len) {{",
            f"  if (len < {m.dlc}) throw std::runtime_error(\"invalid frame length\");",
            f"  {mcls} out;",
        ]
        for idx, s in enumerate(m.signals):
            lines += gen_decode_signal(s, f"out.{s.name.lower()}", idx)
        lines += ["  return out;", "}", ""]

    lines += [f"}}  // namespace {ns}", ""]
    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate C++ CAN codec from DBC")
    parser.add_argument("--dbc", required=True)
    parser.add_argument("--out-dir", required=True)
    parser.add_argument("--namespace", default="standalone_can_codec")
    parser.add_argument("--class-name", default="GeneratedCanCodec")
    parser.add_argument("--base-name", default="generated_can_codec")
    args = parser.parse_args()

    msgs = parse_dbc(Path(args.dbc))
    out = Path(args.out_dir)
    out.mkdir(parents=True, exist_ok=True)

    hname = f"{args.base_name}.h"
    cname = f"{args.base_name}.cc"
    (out / hname).write_text(gen_header(args.namespace, args.class_name, msgs), encoding="utf-8")
    (out / cname).write_text(gen_source(args.namespace, args.class_name, hname, msgs), encoding="utf-8")


if __name__ == "__main__":
    main()
