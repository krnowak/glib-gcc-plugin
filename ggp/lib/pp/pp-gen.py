#!/usr/bin/python3

import argparse
import enum
import re
import sys

@enum.unique
class BlockType(enum.Enum):
    UNSET = 0
    NORMAL = 1
    RANGED = 2
    ITERATIVE = 3

class Templates:
    def __init__(self):
        self._d = {}

    def get(self, key):
        if key not in self._d:
            fail(f"unknown template {key}")
        return self._d[key]

    def put(self, key, value):
        if key in self._d:
            fail(f"redefined template {key}")
        self._d[key] = value

class RangedTemplates:
    def __init__(self, templates, i):
        self._t = templates
        self._i = f"{i}"
        self._d = f"{i - 1}"

    def get(self, key):
        if key == '$':
            return self._i
        if key == '$-':
            return self._d
        return self._t.get(key)

class IterativeTemplates:
    def __init__(self, templates):
        self._t = templates

    def get(self, key):
        m = re.fullmatch(r"\$#(\d+):(\d+)(?::([^:]*)(?::([^:]*)(?::([^:]*)))?)?", key)
        if m is not None:
            from_ = int(m.group(1))
            to = int(m.group(2))
            prefix = m.group(3)
            if prefix is None:
                prefix = ''
            suffix = m.group(4)
            if suffix is None:
                suffix = None
            separator = m.group(5)
            if separator is None:
                separator = ''
            direction = 1
            if from_ > to:
                direction = -1
            items = []
            for i in range(from_, to + direction, direction):
                items.append(f"{prefix}{i}{suffix}")
            val = separator.join(items)
            return val
        return self._t.get(key)

class Globals:
    def __init__(self):
        self.templates = Templates()
        self.range = [0, -1]
        self.block_type = BlockType.UNSET
        self.block = []
        self.lines = []
        self.num = 0
        self.inpath = ''

g = Globals()

def get_templates_in_line(line):
    return re.findall(r"<(.*?)>", line)

def fail(message):
    print(f"{g.inpath}:{g.num}: {message}", file=sys.stderr)
    sys.exit(1)

def process_ppdefines(ppdefines, templates):
    for ppdefine in ppdefines:
        line_templates = set(get_templates_in_line(ppdefine))
        for template in line_templates:
            ppdefine = re.sub(re.escape(f"<{template}>"), templates.get(template), ppdefine)
        g.lines.append(f"#define {ppdefine}")

def process_normal_ppdefines():
    process_ppdefines(g.block, g.templates)

def process_ranged_ppdefines():
    for i in range(g.range[0], g.range[1] + 1):
        templates = RangedTemplates(g.templates, i)
        process_ppdefines(g.block, templates)

def process_iterative_ppdefines():
    templates = IterativeTemplates(g.templates)
    process_ppdefines(g.block, templates)

def process_block():
    if g.block_type == BlockType.UNSET:
        pass
    elif g.block_type == BlockType.NORMAL:
        process_normal_ppdefines()
    elif g.block_type == BlockType.RANGED:
        process_ranged_ppdefines()
    elif g.block_type == BlockType.ITERATIVE:
        process_iterative_ppdefines()
    g.block.clear()
    g.block_type = BlockType.UNSET

def append_line(line):
    g.num = g.num - 1
    process_block()
    g.num = g.num + 1
    g.lines.append(line)

def handle_empty_line():
    append_line('')

def handle_pp_gen_def_command(rest):
    m = re.fullmatch(r"<(\w+)>\s+(\w+)", rest)
    if m is None:
        fail(f"invalid DEF command: {rest}")
    d = m.group(1)
    if d == '$' or d == '$-':
        fail(f"'{d}' is a reserved name")
    val = m.group(2)
    g.templates.put(d, val)

def handle_pp_gen_range_command(rest):
    m = re.fullmatch(r"(\d+)\s+(\d+)", rest)
    if m is None:
        fail(f"invalid RANGE command: {rest}")
    f = int(m.group(1))
    t = int(m.group(2))
    if f > t:
        fail(f"invalid range in RANGE: {rest}")
    g.range[0] = f
    g.range[1] = t

def handle_pp_gen_command(command, rest):
    if command == 'DEF':
        handle_pp_gen_def_command(rest)
    elif command == 'RANGE':
        handle_pp_gen_range_command(rest)
    elif command == 'IGNORE':
        pass
    else:
        fail(f"unknown pp gen command {command}")

def handle_comment(comment):
    m = re.fullmatch(r"PP_GEN:\s+(\w+)\s+(.*)", comment)
    if m is not None:
        command = m.group(1)
        rest = m.group(2)
        handle_pp_gen_command(command, rest)
    else:
        append_line(f"// {comment}")

def ensure_type(ppdefine_type, wanted_type):
    if ppdefine_type == BlockType.UNSET:
        return wanted_type
    if ppdefine_type != wanted_type:
        fail(f"can't mix templates of type {wanted_type.name} with templates of type {ppdefine_type.name} in a single ppdefine")
    return wanted_type

def handle_ppdefine(ppdefine):
    ppdefine_type = BlockType.UNSET
    templates = get_templates_in_line(ppdefine)
    for template in templates:
        if template == '':
            fail(r"can't use empty template name")
        if template == '$':
            ppdefine_type = ensure_type(ppdefine_type, BlockType.RANGED)
            found_range_template = True
            continue
        if template == '$-':
            ppdefine_type = ensure_type(ppdefine_type, BlockType.RANGED)
            found_dec_range_template = True
            continue
        if template.startswith('$#'):
            ppdefine_type = ensure_type(ppdefine_type, BlockType.ITERATIVE)
            continue
    if ppdefine_type == BlockType.UNSET:
        if len(template) == 0:
            fail(f"pointless #ppdefine, there are not templates, just use #define")
        ppdefine_type = BlockType.NORMAL
    elif ppdefine_type == BlockType.RANGED:
        if not found_range_template and found_dec_range_template:
            fail(f"ppdefine does not make sense, uses only <-> without <>")
    if g.block_type == BlockType.UNSET:
        g.block_type = ppdefine_type
    if ppdefine_type != g.block_type:
        fail(f"can't mix ppdefines of type {ppdefine_type.name} with ppdefines of type {g.block_type.name} in a single block")
    g.block.append(ppdefine)

def handle_hash(clause, rest):
    if clause == 'ppdefine':
        handle_ppdefine(rest)
    else:
        if g.block_type != BlockType.UNSET:
            fail(f"can't mix preprocessor clauses with ppdefines")
        if rest is None:
            append_line(f"#{clause}")
        else:
            append_line(f"#{clause} {rest}")

parser = argparse.ArgumentParser()
parser.add_argument('input', help='input file')
parser.add_argument('output', help='output file')
args = parser.parse_args()
g.inpath = args.input
outpath = args.output
with open(g.inpath) as f:
    for num, line in enumerate(f):
        g.num = num + 1
        line = line.rstrip('\n')
        if not line:
            handle_empty_line()
            continue
        m = re.fullmatch(r"//\s+(.*)", line)
        if m is not None:
            comment = m.group(1)
            handle_comment(comment)
            continue
        m = re.fullmatch(r"#\s*(\w+)(?:\s+(.*))?", line)
        if m is not None:
            clause = m.group(1)
            rest = m.group(2)
            handle_hash(clause, rest)
            continue
        fail(f"unhandled line, not a preprocessor directive, not a comment, not an empty line")
    process_block()

with open(outpath, 'w') as f:
    print('// GENERATED FILE, DO NOT EDIT!', file=f)
    print('', file=f)
    for line in g.lines:
        print(line, file=f)
