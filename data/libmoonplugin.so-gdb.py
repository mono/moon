import re

def get_object_type (val):
    return int(val['object_type']);

def get_type_name (val):
    return str(val['deployment']['types']['types']['array'][get_object_type(val)].cast(gdb.lookup_type("Moonlight::Type").pointer())['name'])


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

    def to_string(self):
        if self.val == None or (self.val['padding'] & 1 == 1):
          return "Value (NULL)"
        else:
          if moonlight_is (self.deployment, find_type ("DEPENDENCY_OBJECT"), self.val['k']):
            return "Value (" + str(self.val['u']['dependency_object']) + ")"
          else:
            return "value"

class CollectionPrinter:
    class _iterator:
        def __init__(self, val):
	    self.val = val;
            self.count = 0

        def __iter__(self):
            return self

        def next(self):
            if self.count == self.val['array']['len']:
                raise StopIteration
            data = self.val['array']['pdata'][self.count].cast(gdb.lookup_type("Moonlight::Value").pointer());
            count = self.count
            self.count = self.count + 1
            return ('[%d]' % count, ValuePrinter (data, self.val['deployment']).to_string())

    def __init__ (self, val):
        self.val = val

    def children(self):
        return self._iterator(self.val)

    def to_string (self):
        return  "0x%x" % (long(self.val))

    def display_hint (self):
        return "array"


class EventObjectPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return  "0x%x [%s]" % (long(self.val), get_type_name (self.val))

def lookup_printer (val):

    if is_eventobject (val):
        try:
            val_type = val['object_type']
            if moonlight_is (val['deployment'], find_type ("COLLECTION"), val_type):
	        print "collection!"
	        return CollectionPrinter (val.cast(gdb.lookup_type("Moonlight::Collection").pointer()))
        except:
            # we hit this if val is null
            return None
        str_type = str(val_type)
	return EventObjectPrinter (val)
      
    str_type = str(val.type)
    if str_type == "Moonlight::Value *":
        return ValuePrinter (val)

    lookup_tag = val.type.tag

    if lookup_tag == None:
        return None

    regex = re.compile ('^Moonlight::WeakRef<.*>$')
    if regex.match (lookup_tag):
        return WeakRefPrinter (val)

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
