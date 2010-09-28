new function () {

    var _serverTypeFieldName = '__type';
    var _stringRegEx = new RegExp('[\"\\\\b\\\\f\\\\n\\\\r\\\\t\\\\\\\\\\\\x00-\\\\x1F]', 'i');
    var _dateRegEx = new RegExp('(^|[^\\\\\\\\])\\\\\"\\\\\\\\/Date\\\\((-?[0-9]+)(?:[a-zA-Z]|(?:\\\\+|-)[0-9]{4})?\\\\)\\\\\\\\/\\\\\"', 'g');
    var _jsonRegEx = new RegExp('[^,:{}\\\\[\\\\]0-9.\\\\-+Eaeflnr-u \\\\n\\\\r\\\\t]', 'g');
    var _jsonStringRegEx = new RegExp('\"(\\\\\\\\.|[^\"\\\\\\\\])*\"', 'g');

    this.serialize = function (object) {
        var stringBuilder = [];
        this._serialize (object, stringBuilder);
        return stringBuilder.join ('');
    }

    this.deserialize = function (data) {
        if (data.length === 0) throw 'cannotDeserializeEmptyString';
        var exp = data.replace (_dateRegEx, "$1new Date($2)");

        return eval ('(' + exp + ')');
    }

    this._serializeBoolean = function (object, stringBuilder) {
        stringBuilder.push (object.toString ());
    }

    this._serializeNumber = function (object, stringBuilder) {
        if (isFinite (object))
            stringBuilder.push (String (object));
        else
            throw 'cannotSerializeNonFiniteNumbers';
    }

    this._serializeString = function (object, stringBuilder) {
        stringBuilder.push ('\"');
        if (_stringRegEx.test (object)) {
            var length = object.length;
            for (i = 0; i < length; i++) {
                var curChar = object.charAt (i);
                if (curChar >= ' ') {
                    if (curChar === '\\\\' || curChar === '\"')
                        stringBuilder.push ('\\\\');
                    stringBuilder.push (curChar);
                } else {
                    switch (curChar) {
                        case '\\b':
                            stringBuilder.push ('\\\\b');
                            break;
                        case '\\f':
                            stringBuilder.push ('\\\\f');
                            break;
                        case '\\n':
                            stringBuilder.push ('\\\\n');
                            break;
                        case '\\r':
                            stringBuilder.push ('\\\\r');
                            break;
                        case '\\t':
                            stringBuilder.push ('\\\\t');
                            break;
                        default:
                            stringBuilder.push ('\\\\u00');
                            if (curChar.charCodeAt () < 16)
								stringBuilder.push ('0');
                            stringBuilder.push (curChar.charCodeAt ().toString (16));
                    }
                }
            }
        } else {
            stringBuilder.push (object);
        }
        stringBuilder.push ('\"');
    }

    this._serialize = function (object, stringBuilder) {
        var i;
        switch (typeof object) {
        case 'object':
            if (object) {
                if (object instanceof Number)
                    this._serializeNumber (object, stringBuilder);
                else if (object instanceof Boolean)
                    this._serializeBoolean (object, stringBuilder);
                else if (object instanceof String)
                    this._serializeString (object, stringBuilder);
                else if (object instanceof Array) {
                    stringBuilder.push ('[');
                    for (i = 0; i < object.length; i++) {
                        if (i > 0)
                            stringBuilder.push (',');
                        this._serialize (object[i], stringBuilder);
                    }
                    stringBuilder.push (']');
                } else if (object instanceof Date) {
                    stringBuilder.push ('\"\\/Date(');
                    stringBuilder.push (object.getTime ());
                    stringBuilder.push (')\\/\"');
                } else if (this._managedServices.requiresManagedSerializer (object))
                    stringBuilder.push (this._managedServices.jsonSerialize (object));
                else {
                    var properties = [];
                    var propertyCount = 0;

                    for (var name in object) {
                        if (name.length > 0 && name[0] == '$')
                            continue;

                        if (name === _serverTypeFieldName && propertyCount !== 0) {
                            properties[propertyCount++] = properties[0];
                            properties[0] = name;
                        } else
                            properties[propertyCount++] = name;
                    }

                    stringBuilder.push ('{');
                    var needComma = false;

                    for (i = 0; i < propertyCount; i++) {
                        var value = object[properties[i]];
                        if (typeof value !== 'undefined' && typeof value !== 'function') {
                            if (needComma)
                                stringBuilder.push (',');
                            else
                                needComma = true;

                            this._serialize (properties[i], stringBuilder);
                            stringBuilder.push (':');
                            this._serialize (value, stringBuilder);

                        }
                    }
                    stringBuilder.push ('}');
                }
            } else
                stringBuilder.push('null');
            break;
        case 'number':
            this._serializeNumber (object, stringBuilder);
            break;
        case 'string':
            this._serializeString (object, stringBuilder);
            break;
        case 'boolean':
            this._serializeBoolean (object, stringBuilder);
            break;
        case 'function':
            if (this._managedServices.requiresManagedSerializer (object))
                stringBuilder.push (this._managedServices.jsonSerialize (object));
            else
                stringBuilder.push ('null');
            break;
        default:
            stringBuilder.push ('null');
            break;
        }
    }
}
