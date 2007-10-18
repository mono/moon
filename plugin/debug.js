
var printf_proxy;

function printf (obj, message)
{

	//alert ("host: " + host + " host.tostring: " + host.toString () + " host.content: " + host.content + " host.content.tostring: " + host.content.toString ());
	//return;
	if (printf_proxy == null) {
		var host;
		
		try {
			if (obj == null)
				return;

			if (obj.toString () === "Canvas") {
				printf_proxy = obj;
			} else {
				host = obj.get_element ();
				printf_proxy = host.content.root;
			}
		} catch (err) {
			alert ("err: " + err + " obj: " + obj + " tostring: " + obj.toString ());
			return;
		}
	}

	printf_proxy.printf (message);
}
