#!/usr/bin/env python

import pandas
import os
import sys
from multimetric import __main__ as m_main

directories = {
    'rpc': ['base64.c', 'compression.c', 'encryption.c', 'network.c', 'rpc.c', 'sort.c', 'structures.h'],
    'secrets': ['crypto.c', 'main.c', 'sort.c', 'structures.h'],
}

interests = {
    'loc': 'Lines of code',
    'operators_sum': 'Operators count',
    'operators_uniq': 'Distinct operators',
    'operands_sum': 'Operands count',
    'operands_uniq': 'Distinct operands',
    'halstead_volume': 'Halstead Volume',
    'halstead_difficulty': 'Halstead Difficulty',
    'halstead_effort': 'Halstead Effort',
    'halstead_timerequired': 'Halstead Time Required',
    'cyclomatic_complexity': 'McCabe Cyclomatic complexity',
}

metrics = {}

for dir, files in directories.items():
    if not os.path.isdir(dir):
        print(f'Directory "{dir}" does not exist')
        continue
    sys.argv = [
        'multimetric',
        *list(map(lambda x: dir+'/'+x, files))
    ]
    args = m_main.parse_args()
    result = m_main.run(args)
    res_interests = dict(filter(lambda item: item[0] in interests.keys(), result['overall'].items()))
    metrics[dir] = res_interests

df = pandas.DataFrame(metrics)
df.index = df.index.map(interests)
df_rounded = df.round(0)
print(df_rounded.to_latex())
print(df.to_markdown())