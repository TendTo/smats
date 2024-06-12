#!/bin/bash

# Unused
readonly regex_match_math_double='^\$\$$'
readonly regex_substitute_math_double='\\f$$'

readonly regex_match_math_split='\$(.)\$'
readonly regex_substitute_math_split='$ \1 $'

readonly regex_match_math='(([^$])\$|^\$([^$]))'
readonly regex_substitute_math='\2\\f$\3'

# KNOWN LIMITATION: breaks if the character ` is used in the mermaid code
readonly regex_match_mermaid='```mermaid\n([^\`]*)```'
readonly regex_substitute_mermaid='<pre class='mermaid'>\n\1<\/pre>'

cat "${1}" \
| sed -E \
    -e "s/$regex_match_math_split/$regex_substitute_math_split/g" \
    -e "s/$regex_match_math/$regex_substitute_math/g" \
| sed -E -z  \
    -e "s/$regex_match_mermaid/$regex_substitute_mermaid/g"
