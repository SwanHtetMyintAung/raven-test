import json
import re
from pathlib import Path

from pykakasi import kakasi

# Change these paths if your files are in a different location.
INPUT_FILE = Path("assets/data/soumatome_n3_kanji_with_examples.json")
OUTPUT_FILE = Path("assets/data/soumatome_n3_kanji_with_hiragana.json")

converter = kakasi()

def to_hiragana(word: str) -> str:
    """
    Convert a complete Japanese vocabulary item to hiragana.
    Parenthetical kana annotations such as (な) are included in the reading.
    Placeholder symbols such as ～ and ○○ are retained.
    """
    if not isinstance(word, str):
        return ""

    # Keep common placeholder symbols, while converting the Japanese text.
    # pykakasi normally leaves non-Japanese symbols unchanged.
    converted = converter.convert(word)
    return "".join(part["hira"] for part in converted)


def main():
    with INPUT_FILE.open("r", encoding="utf-8") as f:
        data = json.load(f)

    examples_seen = 0
    readings_added = 0
    missing_words = []

    for week in data.get("weeks", []):
        for day in week.get("days", []):
            for kanji in day.get("kanjis", []):
                for example in kanji.get("examples", []):
                    examples_seen += 1
                    word = example.get("word")

                    if not isinstance(word, str) or not word.strip():
                        missing_words.append(word)
                        continue

                    example["hira"] = to_hiragana(word)
                    readings_added += 1

    with OUTPUT_FILE.open("w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)
        f.write("\n")

    print(f"Examples found: {examples_seen}")
    print(f"Hiragana readings added: {readings_added}")
    print(f"Missing/invalid words: {len(missing_words)}")
    print(f"Output: {OUTPUT_FILE.resolve()}")


if __name__ == "__main__":
    main()
