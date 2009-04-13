function ErrorEventArgsToString (errorArgs)
{
    var errorMsg = "";
    
	if (errorArgs == null)
		return "null";

    // Error information common to all errors.
    errorMsg += "Error Type:    " + errorArgs.errorType + "\n";
    errorMsg += "Error Message: " + errorArgs.errorMessage + "\n";
    errorMsg += "Error Code:    " + errorArgs.errorCode + "\n";
    
    // Determine the type of error and add specific error information.
    switch (errorArgs.errorType)  {
    case "RuntimeError":
        // Display properties specific to RuntimeErrorEventArgs.
        if (errorArgs.lineNumber != 0)
        {
            errorMsg += "Line: " + errorArgs.lineNumber + "\n";
            errorMsg += "Position: " +  errorArgs.charPosition + "\n";
        }
        errorMsg += "MethodName: " + errorArgs.methodName + "\n";
        break;
    case "ParserError":
        // Display properties specific to ParserErrorEventArgs.
        errorMsg += "Xaml File:      " + errorArgs.xamlFile      + "\n";
        errorMsg += "Xml Element:    " + errorArgs.xmlElement    + "\n";
        errorMsg += "Xml Attribute:  " + errorArgs.xmlAttribute  + "\n";
        errorMsg += "Line:           " + errorArgs.lineNumber    + "\n";
        errorMsg += "Position:       " + errorArgs.charPosition  + "\n";
        break;
    default:
        break;
    }
	return errorMsg;	
}

function ErrorEventArgsToOneLineString (errorArgs)
{
    var errorMsg = "";
    
	if (errorArgs == null)
		return "null";

    // Error information common to all errors.
    errorMsg += "Error Type:    " + errorArgs.errorType;
    errorMsg += ", " + "Error Message: " + errorArgs.errorMessage;
    errorMsg += ", " + "Error Code:    " + errorArgs.errorCode;
    
    // Determine the type of error and add specific error information.
    switch (errorArgs.errorType)  {
    case "RuntimeError":
        // Display properties specific to RuntimeErrorEventArgs.
        if (errorArgs.lineNumber != 0)
        {
            errorMsg += ", " + "Line: " + errorArgs.lineNumber;
            errorMsg += ", " + "Position: " +  errorArgs.charPosition;
        }
        errorMsg += ", " + "MethodName: " + errorArgs.methodName;
        break;
    case "ParserError":
        // Display properties specific to ParserErrorEventArgs.
        errorMsg += ", " + "Xaml File:      " + errorArgs.xamlFile;
        errorMsg += ", " + "Xml Element:    " + errorArgs.xmlElement;
        errorMsg += ", " + "Xml Attribute:  " + errorArgs.xmlAttribute;
        errorMsg += ", " + "Line:           " + errorArgs.lineNumber;
        errorMsg += ", " + "Position:       " + errorArgs.charPosition;
        break;
    default:
        break;
    }
	return errorMsg;	
}

function ExceptionToString (ex)
{
	try {
		// In IE ex is an object, and the text is in ex.description
		// In FF ex is the string message itself
		if (!ex)
			return null;
		if (typeof(ex) == "string")
			return ex;
		if (typeof(ex) == "object" && ex.description)
			return ex.description;
		return ex;
	} catch (ex) {
		return ErrorEventArgsToOneLineString (ex);
	}
}

var Host = {
	Windows: navigator.userAgent.indexOf ('Windows') > -1,
	//Linux: navigator.userAgent.indexOf ('Linux') > -1,
	X11: navigator.userAgent.indexOf ('X11') > -1,
	Mac: navigator.userAgent.indexOf ('Macintosh') > -1,
	Firefox: navigator.userAgent.indexOf ('Firefox') > -1,
	IE: navigator.userAgent.indexOf ('MSIE') > -1
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

