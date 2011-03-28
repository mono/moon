import re

def get_object_type (val):
    return int(val['object_type']);

def get_type_name (val):
    try:
      return str(val['deployment']['types']['types']['array'][get_object_type(val)].cast(gdb.lookup_type("Moonlight::Type").pointer())['name'])
    except:
      return "Unknown"


def moonlight_is (deployment, dest, type):
    def is_subclass_of (types, type, super):
        if int(type) == 0: # INVALID
            return False
        if int(type) == int(super):
            return True

        t = types['types']['array'][int(type)].cast(gdb.lookup_type("Moonlight::Type").pointer())

        while True:
            parent = t['parent']
            if int(parent) == int(super):
                return True

            if int(parent) == 0:
                return False

            t = types['types']['array'][int(parent)].cast(gdb.lookup_type("Moonlight::Type").pointer())

        return False

    def is_assignable_from (types, dest, type):
        if int(dest) == int(type):
            return True

        if is_subclass_of (types, type, dest):
            return True

        # add in interface checks here

        return False


    return is_assignable_from (deployment['types'], dest, type)



def is_eventobject (val):
    def is_eventobject_helper (type):
        if str(type) == "Moonlight::EventObject":
            return True

        while type.code == gdb.TYPE_CODE_TYPEDEF:
            type = type.target()

        if type.code != gdb.TYPE_CODE_STRUCT:
            return False

        type = gdb.lookup_type (str(type))

        fields = type.fields()
        if len (fields) < 1:
            return False

        first_field = fields[0]
        return is_eventobject_helper(first_field.type)

    type = val.type
    if type.code != gdb.TYPE_CODE_PTR:
        return False
    type = type.target()
    type = type.unqualified()
    return is_eventobject_helper (type)

def find_type (name):
    return gdb.parse_and_eval ("(int)Moonlight::Type::" + name)


class WeakRefPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return self.val['field']

class ValuePrinter:
    def __init__(self, val, deployment):
        self.val = val
        self.deployment = deployment

#	bool		AsBool () const	            { checked_get_exact (Type::BOOL, false, (bool)u.i32); }
#	gunichar	AsChar () const             { checked_get_exact (Type::CHAR, 0, u.c); }
#	double 		AsDouble () const           { checked_get_exact (Type::DOUBLE, 0.0, u.d); }
#	gint64		AsEnum (Types *types = NULL) const { checked_get_enum (); }
#	float 		AsFloat () const	    { checked_get_exact (Type::FLOAT, 0.0, u.f); }
#	guint64		AsUInt64 () const	    { checked_get_exact (Type::UINT64, 0, u.ui64); }
#	gint64		AsInt64 () const	    { checked_get_exact (Type::INT64, 0, u.i64); }
#	TimeSpan	AsTimeSpan () const	    { checked_get_exact (Type::TIMESPAN, 0, (TimeSpan)u.i64); }
#	guint32		AsUInt32 () const	    { checked_get_exact (Type::UINT32, 0, u.ui32); }
#	gint32		AsInt32 () const	    { checked_get_exact (Type::INT32, 0, u.i32); }
#	char*		AsString () const	    { checked_get_exact (Type::STRING, NULL, u.s); }

    def formatString (self):
        return "String=%s" % (self.val['u']['s'])
    def formatColor (self):
        color = self.val['u']['color']
        if long(color) == 0:
          return "Color=NULL"
        return "Color (r=%d,g=%d,b=%d,a=%g)" % (int(float(color.val['r']) * 255), int(float(color.val['g']) * 255), int(float(color.val['b']) * 255), float(color.val['a']))
    def formatPoint (self):
        point = self.val['u']['point']
        if long(Point) == 0:
          return "Point=NULL"
        return "Point (x=%g,y=%g)" % (float(point.val['x']), float(point.val['y']))
    def formatUri (self):
        return "Uri" # TODO
    def formatRect (self):
        return "Rect" # TODO
    def formatSize (self):
        return "Size" # TODO
    def formatFontFamily (self):
        return "FontFamily" # TODO
    def formatFontWeight (self):
        return "FontWeight" # TODO
    def formatFontStyle (self):
        return "FontStyle" # TODO
    def formatFontStretch (self):
        return "FontStretch" # TODO
    def formatFontSource (self):
        return "FontSource" # TODO
    def formatFontResouce (self):
        return "FontResource" # TODO
    def formatPropertyPath (self):
        return "FontPropertyPath" # TODO
    def formatXmlLanguage (self):
        return "XmlLanguage" # TODO
    def formatDependencyProperty (self):
        return str (self.val['u']['dp'])
    def formatDateTime (self):
        return "DateTime" # TODO
    def formatGCHandle (self):
        return "GCHandle" # TODO
    def formatNPObject (self):
        return "NPObject" # TODO
    def formatRepeatBehavior (self):
        return "RepeatBehavior" # TODO
    def formatDuration (self):
        duration = self.val['u']['duration']
        if long(duration) == 0:
            return "Duration(NULL)"

        if int(duration['k']) == 0:
            return "Duration(Automatic)"
        elif int(duration['k']) == 2:
            return "Duration(Forever)"
        else:
            return "Duration(Timespan:%d)" % (long(duration['timespan']))
    def formatKeyTime (self):
        return "KeyTime" # TODO
    def formatGridLength (self):
        return "GridLength" # TODO
    def formatThickness (self):
        return "Thickness" # TODO
    def formatCornerRadius (self):
        return "CornerRadius" # TODO
    def formatDefault (self):
        return self.val['k']

    def to_string(self):
        formatters = {
            "Moonlight::Type::DURATION": self.formatDuration,
            "Moonlight::Type::STRING": self.formatString
        }

        if self.val == None or (self.val['padding'] & 1 == 1):
          return "[NULL]"
        else:
          if self.deployment == None:
            self.deployment = self.find_deployment_on_stack ()

          if self.deployment == None:
            return "[Value]"
          elif moonlight_is (self.deployment, find_type ("DEPENDENCY_OBJECT"), self.val['k']):
            return "[" + str(self.val['u']['dependency_object']) + "]"
          else:
#            print self.val['k']
            return "[%s]" % (formatters.get(str(self.val['k']), self.formatDefault) ())

    def find_deployment_on_stack (self):
        frame = gdb.selected_frame()
        # look back along the stack for DO*'s used as "this" in a c++ method
        # FIXME we should also grovel along the parameter list in methods

        while frame != None and frame.is_valid():
            try:
                this_var = frame.read_var ("this")
                if this_var != None and is_eventobject (this_var):
                    deployment = this_var.cast (gdb.lookup_type ("Moonlight::EventObject").pointer())['deployment']
                    return deployment
                frame = frame.older()
            except:
                frame = frame.older()
        return None

class DependencyPropertyPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return  "[name=%s, owner=%s]" % (self.val['name'], str(self.val['owner_type']))

class EventObjectPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return  "[%s]" % (get_type_name (self.val))

class CollectionPrinter(EventObjectPrinter):
    class _iterator:
        def __init__(self, val):
            self.val = val
            self.count = 0

        def __iter__(self):
            return self

        def next(self):
            if self.count == self.val['array']['len']:
                raise StopIteration
            data = self.val['array']['pdata'][self.count].cast(gdb.lookup_type("Moonlight::Value").pointer());
            count = self.count
            self.count = self.count + 1
            value_printer = check_printer (ValuePrinter (data, self.val['deployment']));
            if value_printer == None:
                val_thing = data
            else:
                val_thing = value_printer.to_string()
            return ('[%d]' % count, val_thing)

    def __init__ (self, val):
        # not sure why this isn't working...
        # super(CollectionPrinter, self).__init__(val)
        eo_init = EventObjectPrinter.__init__;
        self.eo_init (self, val)

    def children(self):
        return self._iterator(self.val)

    def display_hint (self):
        return "array"

def check_printer (printer):
    try:
       s = printer.to_string()
       return printer
    except:
       return None

def lookup_printer (val):

    if val == None:
       return None

    type = val.type
    if type.code == gdb.TYPE_CODE_PTR:
      type = type.target()

    type = type.unqualified()


    try:
        if is_eventobject (val):
            val_type = val['object_type']
            if moonlight_is (val['deployment'], find_type ("COLLECTION"), val_type):
                return CollectionPrinter (val.cast(gdb.lookup_type("Moonlight::Collection").pointer()))

            str_type = str(val_type)
            # try the printer's to_string method, and if it throws, return None
            return check_printer (EventObjectPrinter (val));
    except:
        # we hit this if val is null
        return None


    str_type = str(type)
    if str_type == "Moonlight::Value":
        # try the printer's to_string method, and if it throws, return None
        return check_printer (ValuePrinter (val, None))

    if str_type == "Moonlight::DependencyProperty":
        # try the printer's to_string method, and if it throws, return None
        return check_printer (DependencyPropertyPrinter (val))

    lookup_tag = val.type.tag

    if lookup_tag == None:
        return None

    regex = re.compile ('^Moonlight::WeakRef<.*>$')
    if regex.match (lookup_tag):
        return check_printer (WeakRefPrinter (val))

    return None

gdb.pretty_printers.append (lookup_printer)

class ForeachCommand (gdb.Command):
    """Foreach on Moonlight::List"""

    def __init__ (self):
        super (ForeachCommand, self).__init__ ("mforeach",
                                               gdb.COMMAND_DATA,
                                               gdb.COMPLETE_SYMBOL)


    def valid_name (self, name):
        if not name[0].isalpha():
            return False
        return True

    def parse_args (self, arg):
        i = arg.find(" ")
        if i <= 0:
            raise Exception ("No var specified")
        var = arg[:i]
        if not self.valid_name(var):
            raise Exception ("Invalid variable name")

        while i < len (arg) and arg[i].isspace():
            i = i + 1

        if arg[i:i+2] != "in":
            raise Exception ("Invalid syntax, missing in")

        i = i + 2

        while i < len (arg) and arg[i].isspace():
            i = i + 1

        colon = arg.find (":", i)
        if colon == -1:
            raise Exception ("Invalid syntax, missing colon")

        val = arg[i:colon]

        colon = colon + 1
        while colon < len (arg) and arg[colon].isspace():
            colon = colon + 1

        command = arg[colon:]

        return (var, val, command)

    def do_iter(self, arg, item, command):
        item = item.cast (gdb.lookup_type("void").pointer())
        item = long(item)
        to_eval = "set $%s = (void *)0x%x\n"%(arg, item)
        gdb.execute(to_eval)
        gdb.execute(command)

    def list_iterator (self, arg, container, command):
        l = container.cast (gdb.lookup_type("Moonlight::List").pointer())['head']
        while long(l) != 0:
            self.do_iter (arg, l, command)
            l = l["next"].cast (gdb.lookup_type("Moonlight::List::Node").pointer())

    def invoke (self, arg, from_tty):
        (var, container, command) = self.parse_args(arg)
        container = gdb.parse_and_eval (container) # container is now the list
        self.list_iterator(var, container, command)

ForeachCommand()
