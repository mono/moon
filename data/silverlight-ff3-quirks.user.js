// silverlight-ff3-quirks
//
// FireFox3 changed the way that <object /> tags are handled breaking
// Silverlight / Moonlight.  This script fixes these bugs by patching the 
// offending method.
//
// 2008-07-18
//
// (c) 2008 Geoff Norton
// Released under the WTFPL
// http://sam.zoy.org/wtfpl/COPYING
//
// ==UserScript==
// @name    FireFox3 Silverlight Quirks
// @namespace    http://blog.sublimeintervention.com/userscripts/
// @description    Fixes Silverlight javascript on FireFox 3
// @include    *
// ==/UserScript==
//

var ftn = '' + unsafeWindow.Silverlight.isInstalled;

if (ftn.indexOf ("data=") > 0 || ftn.indexOf ("<embed") > 0 ) {
    unsafeWindow.Silverlight.isInstalled = function(e) {
		var a = false, j = null;
		try {
			var i = null;
			try {
				i = new ActiveXObject("AgControl.AgControl");
				if (e == null) {
					a = true;
				} else if (i.IsVersionSupported(e)) {
					a = true;
				}
				i = null;
			} catch (l) {
				var k = unsafeWindow.navigator.plugins['Silverlight Plug-In'];
				if (k) {
					if (e === null) {
						a = true;
					} else {
						var h = k.description;
						if (h === "1.0.30226.2") {
							h = "2.0.30226.2";
						}
						var b = h.split(".");
						while (b.length > 3) {
							b.pop();
						}
						while (b.length < 4) {
							b.push(0);
						}
						var d = e.split(".");
						while (d.length > 4) {
							d.pop();
						}
						var c, g, f = 0;
						do {
							c = parseInt(d[f], 10);
							g = parseInt(b[f], 10);
							f++;
						} while (f < d.length && c === g);
						if (c <= g && !isNaN(c)) {
							a = true;
						}
					}
				}
			}
		} catch (l) {
			a = false;
		}
		if (j) {
			unsafeWindow.document.body.removeChild(j);
		}
		return a;
	};

    if (typeof unsafeWindow.createSilverlight != 'undefined') {
        var imgs = document.getElementsByTagName ('img');
        var removed = false;
    
        for (var i = 0; i < imgs.length; i++) {
            if (imgs[i].src = 'http://go.microsoft.com/fwlink/?LinkID=92801&clcid=0x409') {
                imgs[i].style.display = 'none';
                removed = true;
            }
        }
    
        if (removed) {
            unsafeWindow.setTimeout (unsafeWindow.createSilverlight, 1000);
        }
    }
}
