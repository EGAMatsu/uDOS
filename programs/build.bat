del source.zip
del include.zip
del jcl.zip

del all.zip
del alljcl.jcl
del output.txt

zip -9 -X -ll -j source.zip *.txt *.c
zip -9 -X -ll -j include.zip *.h
zip -9 -X -ll -j jcl.zip *.jcl COPYING
zip -9 -X all *.zip

del alljcl.jcl
type build.jcl >>alljcl.jcl
pause
call c:\pdos\mvs380\runmvs alljcl.jcl output.txt all.zip
