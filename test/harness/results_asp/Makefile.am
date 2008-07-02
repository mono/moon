
MCS = gmcs
RUNTIME = mono
MCSFLAGS = -debug+ -t:library -out:moonlight.dll -r:System.Web.dll -r:System.Xml.dll -r:System.Data.dll -r:Mono.Data.SqliteClient.dll

CLEANFILES = moonlight.dll

results_asp_sources = 		\
	Global.asax.cs \
	ShowTest.aspx.cs \
	ShowTest.aspx.designer.cs \
	TestHarness.aspx.cs \
	TestHarness.aspx.designer.cs

all: moonlight.dll

moonlight.dll: $(results_asp_sources)
	$(MCS) $(MCSFLAGS) $(results_asp_sources)
	mkdir bin
	mv moonlight.dll bin
	

setup-test-dir: all
	mkdir -p $(TEST_DIR)
	cp moonlight.dll $(TEST_DIR)/.
	cp -a images $(TEST_DIR)/.
	cp *.aspx $(TEST_DIR)/.

cleanup-test-dir:
	rm -rf bin
