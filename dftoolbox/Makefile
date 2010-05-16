DESTDIR = 
PREFIX  = "/usr/local"
DATADIR = "${PREFIX}/share/dftoolbox"
BINDIR  = "${PREFIX}/bin"

build/feuerkraft:
	scons

clean:
	scons -c
	rm -rf .sconf_temp/
	rm -f .sconsign.dblite

install: install-exec install-data

install-exec: dftoolbox
	install -D dftoolbox "${DESTDIR}${BINDIR}/dftoolbox"

install-data:
	cd icons/; \
	find -type f \( \
	-name "*.gif" \) \
	-exec install -D {} ${DESTDIR}${DATADIR}/{} \;

.PHONY : clean install install-exec install-data

# EOF #
