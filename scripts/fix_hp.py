files = [
    r'D:\Desktop\Git\AI_MAZE\maze_raylib\Interface.c',
    r'D:\Desktop\Git\AI_MAZE\maze_raylib\event.c',
]

for path in files:
    with open(path, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    new_lines = []
    count = 0
    for line in lines:
        stripped = line.rstrip('\n\r')
        if stripped.strip() == 'Hp = shortstep * 2;':
            indent = stripped[:len(stripped) - len(stripped.lstrip())]
            new_lines.append(indent + 'if (shortstep <= 0) shortstep = 50;\n')
            count += 1
        new_lines.append(line)
    with open(path, 'w', encoding='utf-8') as f:
        f.writelines(new_lines)
    print(f'{path}: replaced {count} occurrences')
