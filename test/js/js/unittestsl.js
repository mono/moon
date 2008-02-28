Object.extend (Test.Unit.Testcase.prototype, {

	ignore: function () {
		var message = arguments [0] || "ignored";
		this.info (message);
		this.pass ();
	},

	_createObject: function (model) {
		return model.create (document.getElementById ("MoonlightControl"));
	},

	_respondTo: function (obj, slot) {
		try {
			if (typeof (obj) == "object" && obj ["hasOwnProperty"])
				return obj.hasOwnProperty (slot);
		} catch (ex) {}

		try {
			var s = obj [slot];
			return true;
		} catch (ex) {
			return false;
		}
	},

	_slotsMatch: function (model, array, obj) { with (this) {
		if (!array)
			return;

		for (var i = 0; i < array.length; i++) {
			var slot = array [i];
			//this.assert (obj.hasOwnProperty (slot), model.name + " does not support " + slot);
			this.assert (this._respondTo (obj, slot), model.name + " does not support " + slot);
		}
	}},

	_classMatch: function (model, obj) {
		this._slotsMatch (model, model.properties, obj);

		if (!Prototype.Browser.IE) {
			this._slotsMatch (model, model.methods, obj);
			this._slotsMatch (model, model.events, obj);
		}

		return model.parent ? this._classMatch (model.parent, obj) : true;
	},

	assertCodeModel: function (model) {
		var obj = null;
		if (arguments [1])
			obj = arguments [1];
		else {
			try {
				obj = this._createObject (model);
			} catch (ex) {
				this.fail ("Cannot create object of type " + model.name + ", " + ex);
			}
		}

		if (typeof (obj) == "object")
			this.assertEqual (model.name, obj.toString ());

		return this._classMatch (model, obj);
	},

	_getDefaultValue: function (prop) {
		var val = prop.valdef;
		if (typeof (val) == "object" && val != null)
			return val.toString ();

		return val;
	},

	_checkModelValues: function (model, obj) {
		if (model.properties) {
			for (var i = 0; i < model.properties.length; i++) {
				var name = model.properties [i];
				var fullname = model.name + "." + name;
				var prop = Properties [fullname];
				if (!prop)
					continue;

				if (prop.type == "exception") {
					try {
						var val = obj [name];
						this.fail ("expected exception when accessing: " + fullname);
					} catch (ex) {
						this.pass ();
					}
				} else {
					var val = obj [name];
					this.assert (typeof (val) == prop.type, fullname + ": expected type: " + prop.type + ", found: " + typeof (val) + ", expected value: " + prop.valdef + ", found: " + val);
					this.assert (val == this._getDefaultValue (prop), fullname + ": expected default value: " + prop.valdef + ", found: " + val);
				}
			}
		}

		return model.parent ? this._checkModelValues (model.parent, obj) : true;
	},

	assertDefaultValues: function (model) {
		var obj = null;
		if (arguments [1])
			obj = arguments [1];
		else {
			try {
				var obj = this._createObject (model);
			} catch (ex) {
				this.fail ("Cannot create object of type " + model.name + ", " + ex);
			}
		}
		this._checkModelValues (model, obj);
	},

	assertError: function (error, code) {
		try {
			code ();
			this.fail ("Expected exception of type:" + error.message);
		} catch (ex) {
			var message = typeof (ex) == "object" ? ex.message : ex;
			this.assert (message.indexOf (error.message) > -1, "Expected exception of type: " + error.message);
		}
	}
});

var Browser = Prototype.Browser;

var Host = {
	Windows: navigator.userAgent.indexOf ('Windows') > -1,
	//Linux: navigator.userAgent.indexOf ('Linux') > -1,
	X11: navigator.userAgent.indexOf ('X11') > -1,
	Mac: navigator.userAgent.indexOf ('Macintosh') > -1
};

var Plugin = {
	Silverlight: Host.Windows || Host.Mac,
	Moonlight: Host.X11
};

function is1_0 (plugin)
{
	return plugin.IsVersionSupported ("1.0") && !plugin.IsVersionSupported ("1.1");
}

function is1_1 (plugin)
{
	return plugin.IsVersionSupported ("1.1") && !plugin.IsVersionSupported ("1.2");
}

function is2_0 (plugin)
{
	return plugin.IsVersionSupported ("2.0") && !plugin.IsVersionSupported ("2.1");
}

