
MCS = gmcs
RUNTIME = mono
MCSFLAGS = -debug+ -t:library -out:moonlight.dll -r:System.Web.dll -r:System.Xml.dll -r:System.Data.dll -r:Mono.Data.SqliteClient.dll

CLEANFILES = test-runner.exe test-runner.exe.mdb logging-server.exe ndesk-options.flags

results_asp_sources = 		\
	Global.asax.cs
	ShowTest.aspx.cs
	ShowTest.aspx.designer.cs
	TestHarness.aspx.cs
	TestHarness.aspx.designer.cs

all: moonlight.dll

moonlight.dll: $(results_asp_sources)
	$(MCS) $(MCSFLAGS) $(results_asp_sources) /out:$@ $^ 

setup-test-dir: all
	mkdir -p $(TEST_DIR)
	cp moonlight.dll $(TEST_DIR)/.
	cp -a images $(TEST_DIR)/.
	cp *.aspx $(TEST_DIR)/.

cleanup-test-dir:
	rm -rf $(TEST_DIR)
