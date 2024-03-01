#!/usr/bin/env sh

IFS=$'\n'; for v in $(grep -A1 '\^^^' el_option.in | grep '[A-Z]'); do
	n=$(echo $v | awk '{print $1}' | tr -d '`');
	l=$(echo $n | tr '[:upper:]' '[:lower:]');
	args=$(echo $v | cut -f2 -d\( | cut -f1 -d\))
	def=$(echo $v | cut -f2 -d\[ | cut -f1 -d\] | sed 's/default: //' | tr -d '*')
	ref=manuals/options/el_option.3:$l

	echo "* - :ref:\`$n <$ref>\`"
	echo "  - \`\`$args\`\`"
	echo "  - \`\`$def\`\`"
	#printf '| %-75s | %-45s | %-15s |\n' ":ref:\`$n <$ref>\`" "$args" "$def"
done
