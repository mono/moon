Object.extend (Test.Unit.Testcase.prototype, {
	_createObject: function (model) {
		return model.create (document.getElementById ("MoonlightControl"));
	},

	_slotsMatch: function (model, array, obj) { with (this) {
		if (!array)
			return;

		for (var i = 0; i < array.length; i++) {
			var slot = array [i];
			this.assert (obj.hasOwnProperty (slot), model.name + " does not support " + slot);
		}
	}},

	_classMatch: function (model, obj) {
		this._slotsMatch (model, model.properties, obj);
		this._slotsMatch (model, model.methods, obj);
		this._slotsMatch (model, model.events, obj);

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
				var name = model.properties [length];
				var fullname = model.name + "." + name;
				var prop = Properties [fullname];
				if (!prop)
					continue;

				var val = obj [name];
				this.assert (typeof (val) == prop.type, fullname + ": expected type: " + prop.type + ", found: " + typeof (val));
				this.assert (val == this._getDefaultValue (prop), fullname + ": expected default value: " + prop.valdef + ", found: " + val);
			}
		}

		return model.parent ? this._checkModelValues (model.parent, obj) : true;
	},

	assertDefaultValues: function (model) {
		try {
			var obj = this._createObject (model);

			this._checkModelValues (model, obj);
		} catch (ex) {
			this.fail ("Cannot create object of type " + model.name + ", " + ex);
		}
	},

	assertException: function (error, code) {
		try {
			code ();
			this.fail ("Expected exception of type:" + error.message);
		} catch (ex) {
			this.assert (ex.indexOf (error.message) > -1, "Expected exception of type: " + error.message);
		}
	},
});
