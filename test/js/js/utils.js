function ErrorEventArgsToString (errorArgs)
{
    var errorMsg = "";
    
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
