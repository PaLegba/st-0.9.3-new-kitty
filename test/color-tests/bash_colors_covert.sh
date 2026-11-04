#sed -E 's/<\/?div>/\r\n/g' bash_colors.html | h2md | sed -E 's/\\([_~*#"`“’\])/\1/g; s/\\([./])/\1/g' | sed -E 's/\\(])/\1/g' |sed -E 's/\\(\[)/\1/g' > clean_output.md

sed -E 's/<\/?div>/\r\n/g' bash_colors.html | h2md | \
sed -E 's/\\([_~*#"`“’])/\1/g; s/\\\././g; s/\\\//\//g; s/\\\]/]/g; s/\\\[/[/g' > clean_output.md


#cat bash_colors.html |h2md | sed -E 's/\\([_[~*#"`“’])/\1/g' > clean_output.md
#sed -E 's/<\/?div>/\n/g' bash_colors.html | h2md | sed -E -e 's/\\([_~*#"`“’])/\1/g' -e 's/\\([.\[\]\/])/\1/g' > clean_output.md
#sed -E 's/<\/?div>/\n/g' bash_colors.html | h2md | awk '{gsub(/\\([_~*#"`“’\.\[\]\/])/, "\\1"); print}' > clean_output.md