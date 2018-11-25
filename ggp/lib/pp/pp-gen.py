#!/usr/bin/python3

import argparse
import enum
import lark
import re
import sys

@enum.unique
class BlockType(enum.Enum):
    UNSET = 0
    NORMAL = 1
    REPEATED = 2

@enum.unique
class PostProcessTemplateType(enum.Enum):
    REPLACE = 0
    SKIP = 1

class PostProcessTemplates:
    def __init__(self):
        self._d = {
            '$ppla': {
                'type': PostProcessTemplateType.REPLACE,
                'replacement': '<',
            },
            '$ppra': {
                'type': PostProcessTemplateType.REPLACE,
                'replacement': '>',
            },
            '$ppskip': {
                'type': PostProcessTemplateType.SKIP,
            }
        }

    def has(self, key):
        return key in self._d

    def get(self, key):
        if key not in self._d:
            fail(f"unknown post process template {key}")
        return self._d[key]

class Range:
    def __init__(self, first, last, step):
        self.first = first
        self.last = last
        self.step = step

    def loop(self):
        return range(self.first, self.last + (self.step // abs(self.step)), self.step)

    def next_is_in_range(self, old_value):
        if self.step > 0:
            return old_value + self.step <= self.last
        else:
            return old_value + self.step >= self.last

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

arith_grammar_snippet = """
    sum: product
        | sum "+" product -> add
        | sum "-" product -> sub
    product: atom
        | product "*" atom -> mul
        | product "/" atom -> div
        | product "%" atom -> mod
    atom: SIGNED_INT -> num
         | "(" sum ")" -> sum
    %import common.SIGNED_INT
"""

logic_grammar_snippet = """
    or: and
        | or "or" and -> or
    and: cmp
        | and "and" cmp -> and
    cmp: bool_rec
        | sum "lt" sum -> lt
        | sum "le" sum -> le
        | sum "ge" sum -> ge
        | sum "gt" sum -> gt
        | sum "eq" sum -> eq
        | sum "ne" sum -> ne
    bool_rec: "(" or ")" -> group
        | "!" "(" or ")" -> neg
        | "true"         -> true
        | "false"        -> false
"""

grammar_epilog = """
    %import common.WS_INLINE
    %ignore WS_INLINE
"""

arith_grammar_prolog = """
    start: sum
"""

logic_grammar_prolog = """
    start: or
"""

arith_grammar = f"{arith_grammar_prolog}{arith_grammar_snippet}{grammar_epilog}"
logic_grammar = f"{logic_grammar_prolog}{logic_grammar_snippet}{arith_grammar_snippet}{grammar_epilog}"

class ExprParser:
    def __init__(self):
        self._lp = lark.Lark(logic_grammar)
        self._ap = lark.Lark(arith_grammar)

    def evaluate_logic_expr(self, s):
        try:
            tree = self._lp.parse(s)
            if tree.data != 'start' or len(tree.children) > 1:
                fail(f"unexpected parse result: '{tree.data}' with {len(tree.children)} children")
            return self._parse_logic_tree(tree.children[0])
        except Exception as u:
            fail(f"{u}")

    def evaluate_arith_expr(self, s):
        try:
            tree = self._ap.parse(s)
            if tree.data != 'start' or len(tree.children) > 1:
                fail(f"unexpected parse result: '{tree.data}' with {len(tree.children)} children")
            return self._parse_arith_tree(tree.children[0])
        except Exception as u:
            fail(f"{u}")

    def _parse_logic_tree(self, tree):
        if self._do_logic_fallthrough(tree):
            return self._parse_logic_tree(tree.children[0])
        if tree.data == 'or':
            if self._parse_logic_tree(tree.children[0]):
                return True
            return self._parse_logic_tree(tree.children[1])
        if tree.data == 'and':
            if not self._parse_logic_tree(tree.children[0]):
                return False
            return self._parse_logic_tree(tree.children[1])
        if tree.data == 'lt':
            return self._parse_arith_tree(tree.children[0]) < self._parse_arith_tree(tree.children[1])
        if tree.data == 'le':
            return self._parse_arith_tree(tree.children[0]) <= self._parse_arith_tree(tree.children[1])
        if tree.data == 'gt':
            return self._parse_arith_tree(tree.children[0]) > self._parse_arith_tree(tree.children[1])
        if tree.data == 'ge':
            return self._parse_arith_tree(tree.children[0]) >= self._parse_arith_tree(tree.children[1])
        if tree.data == 'eq':
            return self._parse_arith_tree(tree.children[0]) == self._parse_arith_tree(tree.children[1])
        if tree.data == 'ne':
            return self._parse_arith_tree(tree.children[0]) != self._parse_arith_tree(tree.children[1])
        if tree.data == 'neg':
            return not self._parse_logic_tree(tree.children[0])
        if tree.data == 'true':
            return True
        if tree.data == 'false':
            return False
        fail(f"unhandled logic tree '{tree.data}'")

    def _do_logic_fallthrough(self, tree):
        if len(tree.children) != 1:
            return False
        if tree.data not in {'or', 'and', 'cmp', 'group'}:
            return False
        return True

    def _parse_arith_tree(self, tree):
        if self._do_arith_fallthrough(tree):
            return self._parse_arith_tree(tree.children[0])
        if tree.data == 'add':
            return self._parse_arith_tree(tree.children[0]) + self._parse_arith_tree(tree.children[1])
        if tree.data == 'sub':
            return self._parse_arith_tree(tree.children[0]) - self._parse_arith_tree(tree.children[1])
        if tree.data == 'mul':
            return self._parse_arith_tree(tree.children[0]) * self._parse_arith_tree(tree.children[1])
        if tree.data == 'div':
            return self._parse_arith_tree(tree.children[0]) / self._parse_arith_tree(tree.children[1])
        if tree.data == 'mod':
            return self._parse_arith_tree(tree.children[0]) % self._parse_arith_tree(tree.children[1])
        if tree.data == 'num':
            token = tree.children[0]
            return int(token.value)
        fail(f"unhandled arithmetic tree '{tree.data}'")

    def _do_arith_fallthrough(self, tree):
        if len(tree.children) != 1:
            return False
        if tree.data not in {'sum', 'product'}:
            return False
        return True

class NormalTemplates:
    def __init__(self, templates, ranges):
        self._t = templates
        self._r = ranges

    def get(self, key):
        # <$iX<$$ip[prefix]><$$is[suffix]><$$ic[separator]>>
        m = re.fullmatch(r"\$i(\d+)<\$\$ip\[(.*?)\]><\$\$is\[(.*?)\]><\$\$ic\[(.*?)\]>", key)
        if m is not None:
            range_idx = int(m.group(1))
            prefix = m.group(2)
            suffix = m.group(3)
            separator = m.group(4)
            items = []
            for i in self._r[range_idx].loop():
                items.append(f"{prefix}{i}{suffix}")
            return separator.join(items)
        # <$b[expr]<$$bt[replacement]><$$bf[replacement]>>
        m = re.fullmatch(r"\$b\[(.+?)\]<\$\$bt\[(.*?)\]><\$\$bf\[(.*?)\]>", key)
        if m is not None:
            expr = m.group(1)
            replacement_on_true = m.group(2)
            replacement_on_false = m.group(3)
            if g.expr_parser.evaluate_logic_expr(expr):
                return replacement_on_true
            return replacement_on_false
        # <$a[expr]>
        m = re.fullmatch(r"\$a\[(.+?)\]", key)
        if m is not None:
            expr = m.group(1)
            return str(g.expr_parser.evaluate_arith_expr(expr))
        return self._t.get(key)

class RepeatedTemplates:
    def __init__(self, templates, current_range_values):
        self._t = templates
        self._v = current_range_values

    def get(self, key):
        # <$rvX>
        m = re.fullmatch(r"\$rv(\d+)", key)
        if m is not None:
            range_idx = int(m.group(1))
            return f"{self._v[range_idx]}"
        return self._t.get(key)

class BlockState:
    def __init__(self):
        self.block_type = BlockType.UNSET
        self.block = []
        self.repeat_ranges = set()
        self.ranges_required = 0

class Globals:
    def __init__(self):
        self.post_process_templates = PostProcessTemplates()
        self.templates = Templates()
        self.ranges = []
        self.lines = []
        self.line_num = 0
        self.inpath = ''
        self.block_state = BlockState()
        self.expr_parser = ExprParser()

g = Globals()

def fail(message):
    print(f"{g.inpath}:{g.line_num}: {message}", file=sys.stderr)
    sys.exit(1)

def get_next_template(ppdefine):
    templates=[]
    for c in ppdefine:
        if c == '<':
            templates.append('')
        elif c == '>':
            if len(templates) == 0:
                fail("closing templates without starting one, should not happen, should be caught earlier, I think")
            elif g.post_process_templates.has(templates[-1]) or templates[-1].startswith('$$'):
                template = templates.pop()
                if len(templates) > 0:
                    templates[-1] = f"{templates[-1]}<{template}>"
            else:
                return templates[-1]
        elif len(templates) > 0:
            templates[-1] = templates[-1] + c
    if len(templates) > 0:
        fail("unclosed templates, should not happen, should be caught earlier, I think")
    return None

def post_process_ppdefine(ppdefine, pptemplates):
    # re.findall should be fine, what is left in ppdefine is
    # non-overlapping post-processing templates, if any
    templates = set(re.findall(r"<(.*?)>", ppdefine))
    for template in templates:
        value = pptemplates.get(template)
        if value['type'] == PostProcessTemplateType.REPLACE:
            ppdefine = template.replace(f"<{template}>", value['replacement'])
        elif value['type'] == PostProcessTemplateType.SKIP:
            return ''
    return ppdefine

def process_ppdefines(ppdefines, templates):
    for ppdefine in ppdefines:
        template = get_next_template(ppdefine)
        while template is not None:
            ppdefine = ppdefine.replace(f"<{template}>", templates.get(template), 1)
            template = get_next_template(ppdefine)
        ppdefine = post_process_ppdefine(ppdefine, g.post_process_templates)
        if ppdefine != '':
            g.lines.append(f"#define {ppdefine}")

def process_normal_ppdefines():
    templates = NormalTemplates(g.templates, g.ranges)
    process_ppdefines(g.block_state.block, templates)

def get_next_repeat_values(repeat_values):
    if repeat_values is None:
        initial_values = []
        for idx in range(0, len(g.ranges)):
            if idx in g.block_state.repeat_ranges:
                initial_values.append(g.ranges[idx].first)
            else:
                initial_values.append(None)
        return initial_values
    for idx in reversed(range(0, len(repeat_values))):
        if repeat_values[idx] is None:
            continue
        if g.ranges[idx].next_is_in_range(repeat_values[idx]):
            repeat_values[idx] += g.ranges[idx].step
            return repeat_values
        repeat_values[idx] = g.ranges[idx].first
    return None

def process_repeated_ppdefines():
    normal_templates = NormalTemplates(g.templates, g.ranges)
    repeat_values = get_next_repeat_values(None)
    while repeat_values is not None:
        templates = RepeatedTemplates(normal_templates, repeat_values)
        process_ppdefines(g.block_state.block, templates)
        repeat_values = get_next_repeat_values(repeat_values)

def process_block():
    if g.block_state.block_type == BlockType.UNSET:
        pass
    elif g.block_state.block_type == BlockType.NORMAL:
        process_normal_ppdefines()
    elif g.block_state.block_type == BlockType.REPEATED:
        process_repeated_ppdefines()
    g.block_state = BlockState()

def append_line(line):
    g.line_num = g.line_num - 1
    process_block()
    g.line_num = g.line_num + 1
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

def ensure_type(ppdefine_type, wanted_type):
    if ppdefine_type == BlockType.UNSET:
        return wanted_type
    if ppdefine_type != wanted_type:
        fail(f"can't mix templates of type {wanted_type.name} with templates of type {ppdefine_type.name} in a single ppdefine")
    return wanted_type

def get_templates_in_line(line):
    in_progress = []
    templates = []
    for c in line:
        if c == '<':
            in_progress.append('')
        elif c == '>':
            if len(in_progress) == 0:
                fail(f"closing template without opening one, maybe you wanted to use <$ppra> to get a literal '>'?")
            templates.append(in_progress.pop())
        elif len(in_progress) > 0:
            in_progress[-1] = f"{in_progress[-1]}{c}"
    if len(in_progress) > 0:
        fail(f"unclosed templates, maybe you wanted to use <$ppla> to get a literal '<'?")
    return templates

def handle_ppdefine(ppdefine):
    ppdefine_type = BlockType.UNSET
    templates = get_templates_in_line(ppdefine)
    ranges_required = 0
    repeat_ranges = set()
    for template in templates:
        if template == '':
            fail(r"can't use empty template name")
        m = re.fullmatch(r"\$rv(\d+)", template)
        if m is not None:
            range_idx = int(m.group(1))
            ppdefine_type = ensure_type(ppdefine_type, BlockType.REPEATED)
            ranges_required = max(ranges_required, range_idx + 1)
            repeat_ranges.add(range_idx)
            continue
    if ppdefine_type == BlockType.UNSET:
        if len(templates) == 0:
            fail(f"pointless #ppdefine, there are no templates, just use #define")
        ppdefine_type = BlockType.NORMAL
    elif ppdefine_type == BlockType.REPEATED:
        if len(g.block_state.repeat_ranges) == 0:
            g.block_state.repeat_ranges = repeat_ranges
        if g.block_state.repeat_ranges != repeat_ranges:
            fail(f"can't mix repeat ppdefined with different repeat ranges ({g.block_state.repeat_ranges} vs {repeat_ranges})")
    if g.block_state.block_type == BlockType.UNSET:
        g.block_state.block_type = ppdefine_type
    if ppdefine_type != g.block_state.block_type:
        fail(f"can't mix ppdefines of type {ppdefine_type.name} with ppdefines of type {g.block_state.block_type.name} in a single block")
    g.block_state.ranges_required = max(g.block_state.ranges_required, ranges_required)
    if g.block_state.ranges_required > len(g.ranges):
        fail("not enough ranges specified through pppushrange clauses")
    g.block_state.block.append(ppdefine)

def handle_pppushrange(pppushrange):
    if g.block_state.block_type != BlockType.UNSET:
        fail(f"can't mix ppdefines of type {g.block_state.block_type} with pppushrange")
    m = re.fullmatch(r"(\d+)\s+(\d+)(?:\s+([-+]?\d+))?", pppushrange)
    if m is None:
        fail(f"invalid pppushrange clause, should be <NUM> <NUM> [<SIGNED NUM>]")
    first = int(m.group(1))
    last = int(m.group(2))
    if first == last:
        fail(f"invalid range <{first}, {last}>")
    step = m.group(3)
    if step is None:
        if first > last:
            step = -1
        else:
            step = 1
    if first > last and step >= 0:
        fail("invalid step ({step}) for range <{first}, {last}>, should be lower than zero")
    if first < last and step <= 0:
        fail("invalid step ({step}) for range <{first}, {last}>, should be greater than zero")
    g.ranges.append(Range(first, last, step))

def handle_pppoprange(pppoprange):
    if g.block_state.block_type != BlockType.UNSET:
        fail(f"can't mix ppdefines of type {g.block_state.block_type} with pppoprange")
    to_pop = 1
    if pppoprange is not None:
        m = re.fullmatch(r"(\d+)", pppoprange)
        if m is None:
            fail(f"invalid pppoprange clause, should be [<NUM>]")
        to_pop = int(m.group(1))
        if to_pop == 0:
            fail(f"number of ranges to pop must be greater than zero")
    if len(g.ranges) < to_pop:
        fail(f"not enough ranges to pop ({len(g.ranges)} vs {to_pop})")
    del g.ranges[-to_pop:]

def handle_pptemplate(pptemplate):
    if g.block_state.block_type != BlockType.UNSET:
        fail(f"can't mix ppdefines of type {g.block_state.block_type} with pptemplate")
    m = re.fullmatch(r"(\w+)\s+(.+)", rest)
    if m is None:
        fail(f"invalid pptemplate clause, should be <TEMPLATE> <REPLACEMENT>")
    d = m.group(1)
    val = m.group(2)
    g.templates.put(d, val)

def handle_ppignore(ppignore):
    pass

def handle_hash(clause, rest):
    if clause == 'ppdefine':
        handle_ppdefine(rest)
    elif clause == 'pppushrange':
        handle_pppushrange(rest)
    elif clause == 'pppoprange':
        handle_pppoprange(rest)
    elif clause == 'pptemplate':
        handle_pptemplate(rest)
    elif clause == 'ppignore':
        handle_ppignore(rest)
    else:
        if g.block_state.block_type != BlockType.UNSET:
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
        g.line_num = num + 1
        line = line.rstrip('\n')
        if not line:
            handle_empty_line()
            continue
        m = re.match(r"\s*//", line)
        if m is not None:
            append_line(line)
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
