
MCS = gmcs
RUNTIME = mono
MCSFLAGS = -debug+ -t:library -out:moonlight.dll -r:System.Web.dll -r:System.Xml.dll -r:System.Data.dll -r:Mono.Data.SqliteClient.dll

CLEANFILES = moonlight.dll
TEST_DIR=../../test-run-data

results_asp_sources = 		\
	Global.asax.cs \
	ShowTest.aspx.cs \
	ShowTest.aspx.designer.cs \
	TestHarness.aspx.cs \
	TestHarness.aspx.designer.cs

all: moonlight.dll

moonlight.dll: $(results_asp_sources)
	$(MCS) $(MCSFLAGS) $(results_asp_sources)
	mkdir -p $(TEST_DIR)/{bin,images}
	mv -v moonlight.dll $(TEST_DIR)/bin
	cp -v images/*png $(TEST_DIR)/images
	cp -v *.as*x $(TEST_DIR)
	cp -v *.css $(TEST_DIR)
	sqlite $(TEST_DIR)/moonTestSuite.db < schema.sql || echo "Database already initialized"

setup-test-dir: all
	mkdir -p $(TEST_DIR)
	cp moonlight.dll $(TEST_DIR)/.
	cp -a images $(TEST_DIR)/.
	cp *.aspx $(TEST_DIR)/.

cleanup-test-dir:
	rm -rf $(TEST_DIR)/bin
