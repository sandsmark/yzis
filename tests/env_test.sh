
BUILD_DIR=/home/tim/yzis/yzis.git/build

if [ -n "$CYGWIN" ]; then
	export TMP="d:/tmp"
	export TEMP="d:/tmp"
fi

if [ -n "$CYGWIN" ];
then
	LIBYZIS_DLL=$BUILD_DIR/libyzis/libyzis.dll
	QYZIS_EXE=$BUILD_DIR/qyzis/qyzis.exe
	LIBYZISRUNNER_EXE=$BUILD_DIR/libyzisrunner/libyzisrunner.exe
	UNITTEST_EXE=$BUILD_DIR/tests/unittest/yzis_unittest.exe
else
	LIBYZIS_DLL=$BUILD_DIR/libyzis/libyzis.so
	QYZIS_EXE=$BUILD_DIR/qyzis/qyzis
	NYZIS_EXE=$BUILD_DIR/nyzis/nyzis
	LIBYZISRUNNER_EXE=$BUILD_DIR/libyzisrunner/libyzisrunner
	UNITTEST_EXE=$BUILD_DIR/tests/unittest/yzis_unittest
fi

