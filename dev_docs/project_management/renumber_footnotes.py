#!/usr/bin/env python3
"""
脚注を本文初出順に振り直すスクリプト
形式: 3桁ゼロパディング、10刻み (010, 020, 030, ...)
"""

import re
import sys
from collections import OrderedDict


def renumber_footnotes(content: str) -> str:
    # 本文中の脚注参照を初出順に収集 (定義行 [^xxx]: を除く)
    ref_pattern = re.compile(r'\[\^([^\]]+)\](?!:)')
    
    old_to_new = OrderedDict()
    
    for match in ref_pattern.finditer(content):
        old_num = match.group(1)
        if old_num not in old_to_new:
            new_num = f"{(len(old_to_new) + 1) * 10:03d}"
            old_to_new[old_num] = new_num
    
    # 一時形式に変換 (衝突回避)
    result = content
    for old_num in old_to_new:
        result = re.sub(
            rf'\[\^{re.escape(old_num)}\]',
            f'[^__TMP_{old_num}__]',
            result
        )
    
    # 新番号に変換
    for old_num, new_num in old_to_new.items():
        result = re.sub(
            rf'\[\^__TMP_{re.escape(old_num)}__\]',
            f'[^{new_num}]',
            result
        )
    
    # 巻末の脚注定義を番号順にソート
    lines = result.split('\n')
    
    # 脚注定義ブロックを特定
    footnote_start = None
    footnote_blocks = []  # [(start_line, end_line, footnote_num, content_lines)]
    current_block = None
    
    for i, line in enumerate(lines):
        def_match = re.match(r'^\[\^(\d{3})\]:', line)
        if def_match:
            if current_block:
                current_block[1] = i - 1
                footnote_blocks.append(current_block)
            footnote_num = def_match.group(1)
            current_block = [i, i, footnote_num, [line]]
            if footnote_start is None:
                footnote_start = i
        elif current_block and (line.startswith('    ') or line.strip() == ''):
            current_block[3].append(line)
            current_block[1] = i
        elif current_block:
            footnote_blocks.append(current_block)
            current_block = None
    
    if current_block:
        footnote_blocks.append(current_block)
    
    if footnote_blocks:
        # 番号順にソート
        footnote_blocks.sort(key=lambda x: int(x[2]))
        
        # 再構築
        pre_footnotes = lines[:footnote_start]
        sorted_footnotes = []
        for block in footnote_blocks:
            sorted_footnotes.extend(block[3])
        
        result = '\n'.join(pre_footnotes + sorted_footnotes)
    
    return result


def main():
    if len(sys.argv) < 2:
        print("Usage: python renumber_footnotes.py <input.md> [output.md]")
        print("  output.md を省略すると標準出力に出力")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else None
    
    with open(input_file, 'r', encoding='utf-8') as f:
        content = f.read()
    
    result = renumber_footnotes(content)
    
    if output_file:
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(result)
        print(f"Done: {output_file}")
    else:
        print(result)


if __name__ == '__main__':
    main()
