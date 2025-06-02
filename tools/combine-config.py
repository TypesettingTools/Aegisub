#!/usr/bin/env python3
import json
import argparse


def override(base, over, diff=False):
    for key in over:
        if isinstance(over[key], dict):
            assert diff or key in base

            override(base[key], over[key], diff=diff)
            if diff and not base[key]:
                del base[key]
        else:
            if diff:
                if key in base and base[key] == over[key]:
                    del base[key]
            else:
                base[key] = over[key]


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Merge default_foo.json files")
    parser.add_argument("base")
    parser.add_argument("overrides")
    parser.add_argument("out")
    parser.add_argument("--diff", action="store_true")
    parser.add_argument("--pretty", action="store_true")

    args = parser.parse_args()

    with open(args.base, encoding="utf-8") as bf:
        base = json.load(bf)

    with open(args.overrides, encoding="utf-8") as of:
        over = json.load(of)

    override(base, over, diff=args.diff)

    with open(args.out, "w", encoding="utf-8") as of:
        json.dump(base, of, ensure_ascii=False, indent=4 if args.pretty else None)
