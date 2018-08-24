TAG_FILE = cppreference.xml
TAG_URL = https://en.cppreference.com/w/File:cppreference-doxygen-web.tag.xml

doc/: Doxyfile $(TAG_FILE) README.md src/*
	doxygen

upload-doc:
	make doc
	ghp-import -n doc/html
	git push -f https://github.com/tov/ge211.git gh-pages

$(TAG_FILE):
	curl "$(TAG_URL)" > $@
