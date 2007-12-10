var Errors = {};

Errors.AG_E_UNKNOWN_ERROR = {
	message: "AG_E_UNKNOWN_ERROR",
	type: [],
	code: 1001
};

Errors.AG_E_INIT_CALLBACK = {
	message: "AG_E_UNKNOWN_ERROR",
	type: ["initializeError"],
	code: 2100
};

Errors.AG_E_INIT_ROOTVISUAL = {
	message: "AG_E_INIT_ROOTVISUAL",
	type: ["initializeError"],
	code: 2101
};

Errors.AG_E_RUNTIME_INVALID_CALL = {
	message: "AG_E_RUNTIME_INVALID_CALL",
	type: ["runtimeError"],
	code: 2201
};

Errors.AG_E_RUNTIME_FINDNAME = {
	message: "AG_E_RUNTIME_FINDNAME",
	type: ["runtimeError"],
	code: 2202
};

Errors.AG_E_RUNTIME_SETVALUE = {
	message: "AG_E_RUNTIME_SETVALUE",
	type: ["runtimeError"],
	code: 2203
};

Errors.AG_E_RUNTIME_GETVALUE = {
	message: "AG_E_RUNTIME_GETVALUE",
	type: ["runtimeError"],
	code: 2204
};

Errors.AG_E_RUNTIME_ADDEVENT = {
	message: "AG_E_RUNTIME_ADDEVENT",
	type: ["runtimeError"],
	code: 2205
};

Errors.AG_E_RUNTIME_DELEVENT = {
	message: "AG_E_RUNTIME_DELEVENT",
	type: ["runtimeError"],
	code: 2206
};

Errors.AG_E_RUNTIME_METHOD = {
	message: "AG_E_RUNTIME_METHOD",
	type: ["runtimeError"],
	code: 2207
};

Errors.AG_E_RUNTIME_GETHOST = {
	message: "AG_E_RUNTIME_GETHOST",
	type: ["runtimeError"],
	code: 2208
};

Errors.AG_E_RUNTIME_GETPARENT = {
	message: "AG_E_RUNTIME_GETPARENT",
	type: ["runtimeError"],
	code: 2209
};

Errors.AG_E_INVALID_ARGUMENT = {
	message: "AG_E_INVALID_ARGUMENT",
	type: ["runtimeError"],
	code: 2210
};

Errors.AG_E_UNNAMED_RESOURCE = {
	message: "AG_E_UNNAMED_RESOURCE",
	type: ["runtimeError"],
	code: 2211
};

Errors.AG_E_UNABLE_TO_PLAY = {
	message: "AG_E_UNABLE_TO_PLAY",
	type: ["mediaError"],
	code: 3000
};

Errors.AG_E_INVALID_FILE_FORMAT = {
	message: "AG_E_INVALID_FILE_FORMAT",
	type: ["mediaError", "imageError", "downloadError"],
	code: 3001
};

Errors.AG_E_NOT_FOUND = {
	message: "AG_E_INVALID_FILE_FORMAT",
	type: ["mediaError", "imageError"],
	code: 3002
};

Errors.AG_E_ABORT_FAILED = {
	message: "AG_E_ABORT_FAILED",
	type: ["downloadError"],
	code: 4000
};

Errors.AG_E_CONNECTION_ERROR = {
	message: "AG_E_CONNECTION_ERROR",
	type: ["downloadError"],
	code: 4001
};

Errors.AG_E_NETWORK_ERROR = {
	message: "AG_E_NETWORK_ERROR",
	type: ["downloadError"],
	code: 4002
};
