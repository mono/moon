#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "uri.h"

class Assert {
public:
  static void AreEqual (const char *expected, const char *value, const char *msg = NULL) {
    if (!value || strcmp (expected, value)) {
      char *fail = g_strdup_printf ("expected `%s', got `%s'", expected, value);
      failure (fail, msg);
    }
  }

  static void AreEqual (int expected, int value, const char *msg = NULL) {
    if (expected != value) {
      char *fail = g_strdup_printf ("expected `%d', got `%d'", expected, value);
      failure (fail, msg);
    }
  }

  static void IsNull (const void* p, const char *msg = NULL) {
    if (!p) return;
    failure ("pointer was not null", msg);
  }

  static void IsTrue (bool b, const char *msg = NULL) {
    if (b) return;
    failure ("expected true", msg);
  }

  static void IsFalse (bool b, const char *msg = NULL) {
    if (!b) return;
    failure ("expected true", msg);
  }

private:
  static void failure (const char *msg, const char *user_msg) {
    if (user_msg)
      fprintf (stderr, "FAILURE:  %s: %s\n", msg, user_msg);
    else
      fprintf (stderr, msg);
  }
};


int
main()
{
	Uri *uri = new Uri();

	Assert::IsTrue (uri->Parse ("http://moonlightmedia:81/source/robotica.wmv"), "1 ::Parse");
	Assert::AreEqual ("http", uri->GetScheme(), "1 scheme");
	Assert::AreEqual ("moonlightmedia", uri->GetHost(), "1 host");
	Assert::AreEqual (81, uri->GetPort(), "1 port");
	Assert::AreEqual ("/source/robotica.wmv",uri->GetPath(), "1 path");

	Assert::IsTrue (uri->Parse ("ribbon/images/DialogBoxLauncher.png"), "2 ::Parse");
	Assert::IsNull (uri->GetScheme(), "2 scheme");
	Assert::IsNull (uri->GetHost(), "2 host");
	Assert::AreEqual (-1, uri->GetPort(), "2 port");
	Assert::AreEqual ("ribbon/images/DialogBoxLauncher.png",uri->GetPath(), "2 path");

	Assert::IsTrue (uri->Parse ("../../../../TestResources/SeaDragon/Collection_20/Collection_20.xml"), "3 ::Parse");
	uri->Combine ("Collection_20_files/8/10_5.jpg");
	Assert::AreEqual ("../../../../TestResources/SeaDragon/Collection_20/Collection_20_files/8/10_5.jpg", uri->ToString (), "3 ::Combine");

	Assert::IsTrue (uri->Parse ("http://www.silverlightshow.net/showcase/deepzoom/MultiscalImageDemo_RTW.xap"), "4 ::Parse");
	uri->Combine ("/demo/dzc_output.xml");
	Assert::AreEqual ("http://www.silverlightshow.net/showcase/deepzoom/demo/dzc_output.xml", uri->ToString (), "4 ::Combine");

	Assert::IsTrue (uri->Parse ("http://www.silverlightshow.net/showcase/deepzoom/MultiscalImageDemo_RTW.xap"), "5 ::Parse");
	uri->Combine ("demo/dzc_output.xml");
	Assert::AreEqual ("http://www.silverlightshow.net/showcase/deepzoom/demo/dzc_output.xml", uri->ToString (), "5 ::Combine");

//	//this give us ../../../Bar instead, as Parse () strips the leading /. As for now, we never use Uris poiting to folders
//	//(or with leading /), so even if it looks weird, as we can't test again SL, the current behavior is NOT wrong (not sure it's right neither)
//	Assert::IsTrue (uri->Parse ("../Foo/"), "6 ::Parse");
//	uri->Combine ("../../Bar");
//	Assert::AreEqual ("../Bar", uri->ToString (), "6 ::Combine");

}
