noinst_SCRIPTS = generator.exe

generator_SRC = 		\
	Annotation.cs		\
	FieldInfo.cs		\
	Generator.cs		\
	GlobalInfo.cs		\
	HeaderCollection.cs	\
	Helper.cs		\
	Log.cs			\
	MemberInfo.cs		\
	MethodInfo.cs		\
	ParameterInfo.cs	\
	SignatureType.cs	\
	Tokenizer.cs		\
	TypeInfo.cs		\
	TypeReference.cs

FILES = $(wildcard ../src/*.cpp) $(wildcard ../src/*.h) $(wildcard ../src/*.h.in) $(wildcard ../plugin/*.h) $(wildcard ../plugin/*.h.in)

stamp: generator.exe $(FILES)
	$(SYSTEM_MONO) --debug generator.exe
	$(SYSTEM_MONO) --debug generator.exe
	touch stamp

install all: stamp

generator.exe: $(generator_SRC)
	gmcs $(generator_SRC) -r:System.dll,System.Xml.dll -debug+ -out:generator.exe

CLEANFILES= generator.exe generator.exe.mdb

EXTRA_DIST = ChangeLog HOWTO README TODO $(generator_SRC)
