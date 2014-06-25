# Copyright (c) 2014, Thomas Goyne <plorkyeran@aegisub.org>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# Aegisub Project http://www.aegisub.org/

# A script to strip all of the data we don't use out of ICU's data files
# Run from $ICU_ROOT/source/data

from __future__ import unicode_literals
import re
import os

# Remove stuff we don't use at all from the Makefile
def delete_matching(filename, strs):
    exprs = [re.compile(s) for s in strs]

    with open(filename) as f:
        lines = [line for line in f if not any(r.match(line.decode('utf-8')) for r in exprs)]

    with open(filename, 'w') as f:
        for line in lines:
            f.write(line)

REMOVE_SUBDIRS=['LOCSRCDIR', 'CURRSRCDIR', 'ZONESRCDIR', 'COLSRCDIR', 'RBNFSRCDIR', 'TRANSLITSRCDIR']
delete_matching('Makefile.in', ['^-include .*%s' % s for s in REMOVE_SUBDIRS])
delete_matching('Makefile.in', ['^CNV_FILES'])

with open('misc/misclocal.mk', 'w') as f:
    f.write('MISC_SOURCE = supplementalData.txt likelySubtags.txt icuver.txt icustd.txt metadata.txt')

# Remove data we don't need from the lang and region files
def parse_txt(filename):
    root = {}
    cur = root
    stack = [root]
    comment = False
    for line in open(filename):
        line = line.decode('utf-8')
        line = line.strip()
        if len(line) == 0:
            continue
        if '//' in line:
            continue
        if '/*' in line:
            comment = True
            continue
        if comment:
            if '*/' in line:
                comment = False
            continue

        if line == '}':
            stack.pop()
            cur = stack[-1]
            continue
        if line.endswith('{'):
            obj = {}
            cur[line[:-1]] = obj
            cur = obj
            stack.append(obj)
            continue

        m = re.match('(.*){"(.*)"}', line)
        if not m:
            print line
        else:
            cur[m.group(1)] = m.group(2)

    return root

def remove_sections(root):
    for child in root.itervalues():
        child.pop('Keys', None)
        child.pop('LanguagesShort', None)
        child.pop('Types', None)
        child.pop('Variants', None)
        child.pop('codePatterns', None)
        child.pop('localeDisplayPattern', None)
        child.pop('CountriesShort', None)
        child.pop('Scripts%stand-alone', None)

def remove_languages(root):
    for lang, child in root.iteritems():
        # We only care about a language's name in that language
        lang = lang.split('_')[0]
        trimmed = {}
        v = child.get('Languages', {}).get(lang)
        if v:
            trimmed[lang] = v
        child['Languages'] = trimmed

# Scripts which are actually used by stuff
SCRIPTS = ['Cyrl', 'Latn', 'Arab', 'Vaii', 'Hans', 'Hant']
def remove_scripts(root):
    for lang, child in root.iteritems():
        v = child.get('Scripts')
        if not v:
            continue

        trimmed = {}
        for script in SCRIPTS:
            if v.get(script):
                trimmed[script] = v[script]
        child['Scripts'] = trimmed

def write_dict(name, value, out, indent):
    if len(value) == 0:
        return

    child_indent = indent + '    '

    out.write(indent)
    out.write(name.encode('utf-8'))
    out.write('{\n')
    for k in sorted(value.keys()):
        v = value[k]
        if type(v) == dict:
            write_dict(k, v, out, child_indent)
        else:
            out.write(('%s%s{"%s"}\n' % (child_indent, k, v)).encode('utf-8'))
    out.write(indent)
    out.write('}\n')

def write_file(root, filename):
    with open(filename, 'w') as f:
        for k, v in root.iteritems():
            write_dict(k, v, f, '')

def minify_lang(filename):
    f = parse_txt(filename)
    remove_sections(f)
    remove_languages(f)
    remove_scripts(f)
    write_file(f, filename)

for name in os.listdir('lang'):
    if not name.endswith('.txt'):
        continue
    minify_lang('lang/' + name)

# gather information about which language+region combinations actually exist,
# so that we can drop all others
def gather_regions():
    langs = {
        'af': ['ZA'],
        'am': ['ET'],
        'ar': ['AE', 'BH', 'DZ', 'EG', 'IQ', 'JO', 'KW', 'LB', 'LY', 'MA', 'OM', 'QA', 'SA', 'SY', 'TN', 'YE'],
        'arn': ['CL'],
        'as': ['IN'],
        'az': ['AZ', 'AZ'],
        'ba': ['RU'],
        'be': ['BY'],
        'bg': ['BG'],
        'bn': ['BD', 'IN'],
        'bo': ['CN'],
        'br': ['FR'],
        'bs': ['BA', 'BA'],
        'ca': ['ES'],
        'co': ['FR'],
        'cs': ['CZ'],
        'cy': ['GB'],
        'da': ['DK'],
        'de': ['AT', 'CH', 'DE', 'LI', 'LU'],
        'div': ['MV'],
        'el': ['GR'],
        'en': ['029', 'AU', 'BZ', 'CA', 'GB', 'IE', 'IN', 'JM', 'MY', 'NZ', 'PH', 'SG', 'TT', 'US', 'ZA', 'ZW'],
        'es': ['AR', 'BO', 'CL', 'CO', 'CR', 'DO', 'EC', 'ES', 'GT', 'HN', 'MX', 'NI', 'PA', 'PE', 'PR', 'PY', 'SV', 'US', 'UY', 'VE'],
        'et': ['EE'],
        'eu': ['ES'],
        'fa': ['IR'],
        'fi': ['FI'],
        'fil': ['PH'],
        'fo': ['FO'],
        'fr': ['BE', 'CA', 'CH', 'FR', 'LU', 'MC'],
        'fy': ['NL'],
        'ga': ['IE'],
        'gl': ['ES'],
        'gsw': ['FR'],
        'gu': ['IN'],
        'ha': ['NG'],
        'he': ['IL'],
        'hi': ['IN'],
        'hr': ['BA', 'HR'],
        'hu': ['HU'],
        'hy': ['AM'],
        'id': ['ID'],
        'ig': ['NG'],
        'ii': ['CN'],
        'is': ['IS'],
        'it': ['CH', 'IT'],
        'iu': ['CA', 'CA'],
        'ja': ['JP'],
        'ka': ['GE'],
        'kk': ['KZ'],
        'kl': ['GL'],
        'km': ['KH'],
        'kn': ['IN'],
        'ko': ['KR'],
        'kok': ['IN'],
        'ky': ['KG'],
        'lb': ['LU'],
        'lo': ['LA'],
        'lt': ['LT'],
        'lv': ['LV'],
        'mi': ['NZ'],
        'mk': ['MK'],
        'ml': ['IN'],
        'mn': ['CN', 'MN'],
        'moh': ['CA'],
        'mr': ['IN'],
        'ms': ['BN', 'MY'],
        'mt': ['MT'],
        'nb': ['NO'],
        'ne': ['NP'],
        'nl': ['BE', 'NL'],
        'nn': ['NO'],
        'nso': ['ZA'],
        'oc': ['FR'],
        'or': ['IN'],
        'pa': ['IN'],
        'pl': ['PL'],
        'prs': ['AF'],
        'ps': ['AF'],
        'pt': ['BR', 'PT'],
        'qut': ['GT'],
        'quz': ['BO', 'EC', 'PE'],
        'rm': ['CH'],
        'ro': ['RO'],
        'ru': ['RU'],
        'rw': ['RW'],
        'sa': ['IN'],
        'sah': ['RU'],
        'se': ['FI', 'NO', 'SE'],
        'si': ['LK'],
        'sk': ['SK'],
        'sl': ['SI'],
        'sma': ['NO', 'SE'],
        'smj': ['NO', 'SE'],
        'smn': ['FI'],
        'sms': ['FI'],
        'sq': ['AL'],
        'sr': ['BA', 'BA', 'SP', 'YU'],
        'sv': ['FI', 'SE'],
        'sw': ['KE', 'TZ'],
        'syr': ['SY'],
        'ta': ['IN'],
        'te': ['IN'],
        'tg': ['TJ'],
        'th': ['TH'],
        'tk': ['TM'],
        'tn': ['ZA'],
        'tr': ['TR'],
        'tt': ['RU'],
        'tzm': ['DZ'],
        'ug': ['CN'],
        'uk': ['UA'],
        'ur': ['PK'],
        'uz': ['UZ', 'UZ'],
        'vi': ['VN'],
        'wee': ['DE'],
        'wen': ['DE'],
        'wo': ['SN'],
        'xh': ['ZA'],
        'yo': ['NG'],
        'zh': ['CN', 'HK', 'MO', 'SG', 'TW'],
        'zu': ['ZA']
    }
    for name in os.listdir('region'):
        if not name.endswith('.txt'): continue
        parts = name[:-4].split('_')
        if len(parts) == 1: continue
        if not parts[0] in langs:
            langs[parts[0]] = []
        langs[parts[0]].extend(parts[1:])
    return langs

REGIONS = gather_regions()
def remove_countries(root):
    for lang, child in root.iteritems():
        v = child.get('Countries', {})
        if not v: continue

        # We only care about the names for regions in the languages used in
        # those regions
        lang = lang.split('_')[0]
        regions = REGIONS.get(lang)
        if not regions:
            del child['Countries']
            continue

        trimmed = {}
        for region in regions:
            name = v.get(region)
            if name:
                trimmed[region] = name
        child['Countries'] = trimmed

def minify_region(filename):
    f = parse_txt(filename)
    remove_sections(f)
    remove_countries(f)
    write_file(f, filename)

for name in os.listdir('region'):
    if not name.endswith('.txt'):
        continue
    minify_region('region/' + name)

