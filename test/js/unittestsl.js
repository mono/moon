Object.extend (Test.Unit.Testcase.prototype, {
	_createObject: function (model) {
		return model.create (document.getElementById ("MoonlightControl"));
	},

	_slotsMatch: function (model, array, obj) { with (this) {
		for (var i = 0; i < array.length; i++) {
			var slot = array [i];
			assert (obj.hasOwnProperty (slot), model.name + " does not support " + slot);
		}
	}},

	_classMatch: function (model, obj) { with (this) {
		_slotsMatch (model, model.properties, obj);
		_slotsMatch (model, model.methods, obj);
		_slotsMatch (model, model.events, obj);

		return model.parent ? _classMatch (model.parent, obj) : true;
	}},

	assertCodeModel: function (model) { with (this) {
		var obj = null;
		if (arguments [1])
			obj = arguments [1];
		else {
			try {
				obj = _createObject (model);
			} catch (ex) {
				fail ("Cannot create object of type " + model.name + ", " + ex.toString ());
			}
		}

		return _classMatch (model, obj);
	}},
});
