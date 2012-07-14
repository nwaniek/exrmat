MEX = /opt/matlab/R2011a/bin/mex

readexr:
	$(MEX) readexr.cpp -lIlmImf -lImath -lIlmThread -I/usr/include/OpenEXR -L/usr/local/lib

clean:
	@rm -f readexr.mexa64
