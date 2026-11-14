import argparse
from pathlib import Path


def escape_c(data: bytes) -> str:
    result = []

    for b in data:
        if b == 34:
            result.append(r'\"')
        elif b == 92:
            result.append(r'\\')
        elif b == 10:
            result.append(r'\n')
        elif b == 13:
            result.append(r'\r')
        elif b == 9:
            result.append(r'\t')
        elif 32 <= b <= 126:
            result.append(chr(b))
        else:
            result.append(f'\\{b:03o}')

    return ''.join(result)


def main():
    parser = argparse.ArgumentParser(
        description="Embed XML into a C header"
    )

    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)

    args = parser.parse_args()

    source = args.source
    output = args.output

    if not source.exists():
        raise FileNotFoundError(
            f"XML file not found: {source}"
        )

    data = source.read_bytes()
    output.parent.mkdir(parents=True, exist_ok=True)

    lines = data.splitlines(keepends=True)

    with output.open("w", encoding="ascii", newline="\n") as f:
        f.write("#pragma once\n")
        f.write("#include <stddef.h>\n\n")

        f.write("static const char hello_xml[] =\n")

        if not lines:
            f.write('    ""\n')
        else:
            for line in lines:
                f.write(f'    "{escape_c(line)}"\n')

        f.write(";\n\n")
        f.write("static const size_t hello_xml_size = ")
        f.write("sizeof(hello_xml) - 1;\n")

    print(f"Successfully embedded {len(data)} bytes.")
    print(f"Output: {output}")


if __name__ == "__main__":
    main()

