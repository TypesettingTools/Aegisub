#!/bin/sh

# Copyright (c) 2012, Thomas Goyne <plorkyeran@aegisub.org>
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

for aff in src/dictionaries/*.aff; do
	lang=$(echo $aff | cut -d/ -f3 | cut -d. -f1)

	case $lang in
		ca_ES) langname="Catalan" ;;
		cs_CZ) langname="Czech" ;;
		da_DK) langname="Danish" ;;
		de_AT) langname="Austrian German" ;;
		de_CH) langname="Swiss German" ;;
		de_DE) langname="German" ;;
		el_GR) langname="Greek" ;;
		en_GB) langname="British English" ;;
		en_US) langname="American English" ;;
		es_ES) langname="Spanish" ;;
		eu) langname="Basque" ;;
		fr_FR) langname="French" ;;
		hu_HU) langname="Hungarian" ;;
		it_IT) langname="Italian" ;;
		ms_MY) langname="Malay" ;;
		nl_NL) langname="Dutch" ;;
		pl_PL) langname="Polish" ;;
		pt_BR) langname="Brazilian Portuguese" ;;
		pt_PT) langname="Portuguese" ;;
		ru_RU) langname="Russian" ;;
		sk_SK) langname="Slovak" ;;
		sl_SI) langname="Slovenian" ;;
		sr_SR) langname="Serbian (Cyrillic and Latin)" ;;
		sv_SE) langname="Swedish" ;;
		sw_TZ) langname="Swahili (Tanzania)" ;;
		th_TH) langname="Thai" ;;
		vi_VN) langname="Vietnamese" ;;
		*) echo "Unknown language $lang"; exit 1 ;;
	esac

	printf '#define LANGCODE "%s"\n' "${lang}" > temp_dict.iss
	printf '#define LANGNAME "%s"\n' "${langname}" >> temp_dict.iss
	if [[ ! -f "src/dictionaries/th_${lang}.idx" ]]; then
		printf '#define NOTHES\n' >> temp_dict.iss
	fi

	printf '#include "dictionaries/fragment_stddict.iss"\n' >> temp_dict.iss

	src/iscc.sh temp_dict.iss
	rm temp_dict.iss
done
