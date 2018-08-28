TAG_FILE = cppreference.xml
TAG_URL = http://upload.cppreference.com/mwiki/images/f/f8/cppreference-doxygen-web.tag.xml

doc/: Doxyfile $(TAG_FILE) FRONTMATTER.md src/*
	doxygen

FRONTMATTER.md: README.md
	sed -E '/^\[.*\]: *$$/,/^ *$$/d;s/\[|\]//g' < $< > $@

upload-doc:
	make doc
	ghp-import -n doc/html
	git push -f https://github.com/tov/ge211.git gh-pages

$(TAG_FILE):
	curl "$(TAG_URL)" > $@
