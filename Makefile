TAG_FILE = doxygen/cppreference.xml
TAG_URL = http://upload.cppreference.com/mwiki/images/f/f8/cppreference-doxygen-web.tag.xml

DOXY = doxygen/DoxygenLayout.xml doxygen/FRONTMATTER.md $(TAGFILE)
SRCS = $(wildcard include/*) $(wildcard src/*)

doc: doxygen/Doxyfile $(DOXY) $(SRCS)
	doxygen $<

doxygen/FRONTMATTER.md: README.md
	sed -E '/^\[.*\]: *$$/,/^ *$$/d;s/\[|\]//g' < $< > $@

upload-doc:
	make doc
	ghp-import -n doc/html
	git push -f https://github.com/tov/ge211.git gh-pages

$(TAG_FILE):
	curl "$(TAG_URL)" > $@
