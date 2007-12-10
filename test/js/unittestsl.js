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

	assertException: function (error, code) {
		try {
			code ();
			this.fail ("Expected exception of type:" + error.message);
		} catch (ex) {
			this.assert (ex.indexOf (error.message) > -1, "Expected exception of type: " + error.message);
		}
	},
});
