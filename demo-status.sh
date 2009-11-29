#!/bin/bash -e

# input file
DEMOSTATUS=demo-status.txt

# variables we use
SITE=
URL=
RATING=
COMMENTS=
ROW=
ROWS=
# the final result: pass (0) if all test sites have rating >= 2, otherwise fail (RESULT=1)
RESULT=0

# icons
IMG_OK=http://www.mono-project.com/files/2/22/Accept.png
IMG_STAR=http://www.mono-project.com/files/2/2e/Star.png
IMG_BAD=http://www.mono-project.com/files/8/8c/Delete.png
IMG_BROKEN=http://www.mono-project.com/files/a/aa/Help.png

STAR_N1="<img src='$IMG_BROKEN' alt='Site url is broken or the application is broken on Silverlight' />"
STAR_0="<img src='$IMG_BAD' alt='Site has no functionality or crashes Firefox' />"
STAR_1="<img src='$IMG_STAR' alt='Site has minimal functionality and/or major cosmetic issues.' />"
STAR_2="<img src='$IMG_STAR' alt='Site has some functionality and/or cosmetic issues' /><img src='$IMG_STAR' alt='Site has some functionality and/or cosmetic issues' />"
STAR_3="<img src='$IMG_STAR' alt='Site has most features working and/or has minor cosmetic issues' /><img src='$IMG_STAR' alt='Site has most features working and/or has minor cosmetic issues' /><img src='$IMG_STAR' alt='Site has most features working and/or has minor cosmetic issues' />"
STAR_4="<img src='$IMG_OK' alt='Site has some functionality and/or cosmetic issues'/>"

# can't seem to get a -1 index to work in bash, move to 10 and add a special case below
ICONS[10]=$STAR_N1
ICONS[0]=$STAR_0
ICONS[1]=$STAR_1
ICONS[2]=$STAR_2
ICONS[3]=$STAR_3
ICONS[4]=$STAR_4

# parse the input file
while read i; do
	if [[ "x${i:0:1}" == "x#" ]]; then
		continue
	fi

	if [[ "x$i" == "x" ]]; then
		if [[ "x$RATING" == "x-1" ]]; then
			IDX=10
		else
			IDX=$RATING
		fi

		ROW="<tr><td>$RATING ${ICONS[$IDX]}</td><td><a href='$URL'>$SITE</a></td><td><ul>$COMMENTS</ul></td></tr>"
		ROWS="$ROWS
$ROW"
		COMMENTS=
		SITE=
		URL=
		RATING=
		continue
	fi

	case "$i" in
		site:*)
			SITE=${i:5}
			#echo "Site is: $SITE"
			;;
		url:*)
			URL=${i:4}
			#echo "Url is: $URL"
			;;
		rating:*)
			RATING=${i:7}
			if [[ $RATING -le 2 ]]; then
				RESULT=1
				echo Site \"$SITE\" has a rating below 3 \(rating: $RATING\): we fail.
			fi
			#echo "Rating is: $RATING"
			;;
		"*"*)
			COMMENT=${i:2}

			if echo $COMMENT | grep "Bug [[:digit:]]\{5\}" > /dev/null; then
				COMMENT=`echo $COMMENT | sed 's|Bug \([0-9]*\)|<a href="http://bugzilla.novell.com/show_bug.cgi?id=\1">&</a>|'`
			fi

			COMMENTS="$COMMENTS<li>$COMMENT</li>"
			#echo "Comment is: ${i:2}"
			;;
		*)
			#echo "Else: $i"
			;;
	esac
done < $DEMOSTATUS


cat > demo-status.html << EOF
<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Transitional//EN' 'http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd'>
<html xmlns='http://www.w3.org/1999/xhtml'>
<head>
<meta http-equiv='Content-Type' content='text/html;charset=utf-8' />
<title>Moonlight Test Sites</title>

<script type='text/javascript'>
<!--
	/* this script was found here: http://www.kryogenix.org/code/browser/sorttable/sorttable.js */
		/*
		SortTable
		version 2
		7th April 2007
		Stuart Langridge, http://www.kryogenix.org/code/browser/sorttable/

		Instructions:
		Download this file
		Add <script src="sorttable.js"><\/script> to your HTML
		Add class="sortable" to any table you'd like to make sortable
		Click on the headers to sort

		Thanks to many, many people for contributions and suggestions.
		Licenced as X11: http://www.kryogenix.org/code/browser/licence.html
		This basically means: do what you want with it.
		*/


		var stIsIE = /*@cc_on!@*/false;

		sorttable = {
		init: function() {
			// quit if this function has already been called
			if (arguments.callee.done) return;
			// flag this function so we don't do the same thing twice
			arguments.callee.done = true;
			// kill the timer
			if (_timer) clearInterval(_timer);

			if (!document.createElement || !document.getElementsByTagName) return;

			sorttable.DATE_RE = /^(\d\d?)[\/\.-](\d\d?)[\/\.-]((\d\d)?\d\d)$/;

			forEach(document.getElementsByTagName('table'), function(table) {
			if (table.className.search(/\bsortable\b/) != -1) {
				sorttable.makeSortable(table);
			}
			});

		},

		makeSortable: function(table) {
			if (table.getElementsByTagName('thead').length == 0) {
			// table doesn't have a tHead. Since it should have, create one and
			// put the first table row in it.
			the = document.createElement('thead');
			the.appendChild(table.rows[0]);
			table.insertBefore(the,table.firstChild);
			}
			// Safari doesn't support table.tHead, sigh
			if (table.tHead == null) table.tHead = table.getElementsByTagName('thead')[0];

			if (table.tHead.rows.length != 1) return; // can't cope with two header rows

			// Sorttable v1 put rows with a class of "sortbottom" at the bottom (as
			// "total" rows, for example). This is B&amp;R, since what you're supposed
			// to do is put them in a tfoot. So, if there are sortbottom rows,
			// for backwards compatibility, move them to tfoot (creating it if needed).
			sortbottomrows = [];
			for (var i=0; i<table.rows.length; i++) {
			if (table.rows[i].className.search(/\bsortbottom\b/) != -1) {
				sortbottomrows[sortbottomrows.length] = table.rows[i];
			}
			}
			if (sortbottomrows) {
			if (table.tFoot == null) {
				// table doesn't have a tfoot. Create one.
				tfo = document.createElement('tfoot');
				table.appendChild(tfo);
			}
			for (var i=0; i<sortbottomrows.length; i++) {
				tfo.appendChild(sortbottomrows[i]);
			}
			delete sortbottomrows;
			}

			// work through each column and calculate its type
			headrow = table.tHead.rows[0].cells;
			for (var i=0; i<headrow.length; i++) {
			// manually override the type with a sorttable_type attribute
			if (!headrow[i].className.match(/\bsorttable_nosort\b/)) { // skip this col
				mtch = headrow[i].className.match(/\bsorttable_([a-z0-9]+)\b/);
				if (mtch) { override = mtch[1]; }
				if (mtch && typeof sorttable["sort_"+override] == 'function') {
					headrow[i].sorttable_sortfunction = sorttable["sort_"+override];
				} else {
					headrow[i].sorttable_sortfunction = sorttable.guessType(table,i);
				}
				// make it clickable to sort
				headrow[i].sorttable_columnindex = i;
				headrow[i].sorttable_tbody = table.tBodies[0];
				dean_addEvent(headrow[i],"click", function(e) {

				if (this.className.search(/\bsorttable_sorted\b/) != -1) {
					// if we're already sorted by this column, just
					// reverse the table, which is quicker
					sorttable.reverse(this.sorttable_tbody);
					this.className = this.className.replace('sorttable_sorted',
															'sorttable_sorted_reverse');
					this.removeChild(document.getElementById('sorttable_sortfwdind'));
					sortrevind = document.createElement('span');
					sortrevind.id = "sorttable_sortrevind";
					sortrevind.innerHTML = stIsIE ? '&nbsp<font face="webdings">5</font>' : '&nbsp;&#x25B4;';
					this.appendChild(sortrevind);
					return;
				}
				if (this.className.search(/\bsorttable_sorted_reverse\b/) != -1) {
					// if we're already sorted by this column in reverse, just
					// re-reverse the table, which is quicker
					sorttable.reverse(this.sorttable_tbody);
					this.className = this.className.replace('sorttable_sorted_reverse',
															'sorttable_sorted');
					this.removeChild(document.getElementById('sorttable_sortrevind'));
					sortfwdind = document.createElement('span');
					sortfwdind.id = "sorttable_sortfwdind";
					sortfwdind.innerHTML = stIsIE ? '&nbsp<font face="webdings">6</font>' : '&nbsp;&#x25BE;';
					this.appendChild(sortfwdind);
					return;
				}

				// remove sorttable_sorted classes
				theadrow = this.parentNode;
				forEach(theadrow.childNodes, function(cell) {
					if (cell.nodeType == 1) { // an element
					cell.className = cell.className.replace('sorttable_sorted_reverse','');
					cell.className = cell.className.replace('sorttable_sorted','');
					}
				});
				sortfwdind = document.getElementById('sorttable_sortfwdind');
				if (sortfwdind) { sortfwdind.parentNode.removeChild(sortfwdind); }
				sortrevind = document.getElementById('sorttable_sortrevind');
				if (sortrevind) { sortrevind.parentNode.removeChild(sortrevind); }

				this.className += ' sorttable_sorted';
				sortfwdind = document.createElement('span');
				sortfwdind.id = "sorttable_sortfwdind";
				sortfwdind.innerHTML = stIsIE ? '&nbsp<font face="webdings">6</font>' : '&nbsp;&#x25BE;';
				this.appendChild(sortfwdind);

					// build an array to sort. This is a Schwartzian transform thing,
					// i.e., we "decorate" each row with the actual sort key,
					// sort based on the sort keys, and then put the rows back in order
					// which is a lot faster because you only do getInnerText once per row
					row_array = [];
					col = this.sorttable_columnindex;
					rows = this.sorttable_tbody.rows;
					for (var j=0; j<rows.length; j++) {
					row_array[row_array.length] = [sorttable.getInnerText(rows[j].cells[col]), rows[j]];
					}
					/* If you want a stable sort, uncomment the following line */
					//sorttable.shaker_sort(row_array, this.sorttable_sortfunction);
					/* and comment out this one */
					row_array.sort(this.sorttable_sortfunction);

					tb = this.sorttable_tbody;
					for (var j=0; j<row_array.length; j++) {
					tb.appendChild(row_array[j][1]);
					}

					delete row_array;
				});
				}
			}
		},

		guessType: function(table, column) {
			// guess the type of a column based on its first non-blank row
			sortfn = sorttable.sort_alpha;
			for (var i=0; i<table.tBodies[0].rows.length; i++) {
			text = sorttable.getInnerText(table.tBodies[0].rows[i].cells[column]);
			if (text != '') {
				if (text.match(/^-?[£$¤]?[\d,.]+%?$/)) {
				return sorttable.sort_numeric;
				}
				// check for a date: dd/mm/yyyy or dd/mm/yy
				// can have / or . or - as separator
				// can be mm/dd as well
				possdate = text.match(sorttable.DATE_RE)
				if (possdate) {
				// looks like a date
				first = parseInt(possdate[1]);
				second = parseInt(possdate[2]);
				if (first > 12) {
					// definitely dd/mm
					return sorttable.sort_ddmm;
				} else if (second > 12) {
					return sorttable.sort_mmdd;
				} else {
					// looks like a date, but we can't tell which, so assume
					// that it's dd/mm (English imperialism!) and keep looking
					sortfn = sorttable.sort_ddmm;
				}
				}
			}
			}
			return sortfn;
		},

		getInnerText: function(node) {
			// gets the text we want to use for sorting for a cell.
			// strips leading and trailing whitespace.
			// this is *not* a generic getInnerText function; it's special to sorttable.
			// for example, you can override the cell text with a customkey attribute.
			// it also gets .value for <input> fields.

			hasInputs = (typeof node.getElementsByTagName == 'function') &&
						node.getElementsByTagName('input').length;

			if (node.getAttribute("sorttable_customkey") != null) {
			return node.getAttribute("sorttable_customkey");
			}
			else if (typeof node.textContent != 'undefined' && !hasInputs) {
			return node.textContent.replace(/^\s+|\s+$/g, '');
			}
			else if (typeof node.innerText != 'undefined' && !hasInputs) {
			return node.innerText.replace(/^\s+|\s+$/g, '');
			}
			else if (typeof node.text != 'undefined' && !hasInputs) {
			return node.text.replace(/^\s+|\s+$/g, '');
			}
			else {
			switch (node.nodeType) {
				case 3:
				if (node.nodeName.toLowerCase() == 'input') {
					return node.value.replace(/^\s+|\s+$/g, '');
				}
				case 4:
				return node.nodeValue.replace(/^\s+|\s+$/g, '');
				break;
				case 1:
				case 11:
				var innerText = '';
				for (var i = 0; i < node.childNodes.length; i++) {
					innerText += sorttable.getInnerText(node.childNodes[i]);
				}
				return innerText.replace(/^\s+|\s+$/g, '');
				break;
				default:
				return '';
			}
			}
		},

		reverse: function(tbody) {
			// reverse the rows in a tbody
			newrows = [];
			for (var i=0; i<tbody.rows.length; i++) {
			newrows[newrows.length] = tbody.rows[i];
			}
			for (var i=newrows.length-1; i>=0; i-=1) {
			tbody.appendChild(newrows[i]);
			}
			delete newrows;
		},

		/* sort functions
			each sort function takes two parameters, a and b
			you are comparing a[0] and b[0] */
		sort_numeric: function(a,b) {
			aa = parseFloat(a[0].replace(/[^0-9.-]/g,''));
			if (isNaN(aa)) aa = 0;
			bb = parseFloat(b[0].replace(/[^0-9.-]/g,''));
			if (isNaN(bb)) bb = 0;
			return aa-bb;
		},
		sort_alpha: function(a,b) {
			if (a[0]==b[0]) return 0;
			if (a[0]<b[0]) return -1;
			return 1;
		},
		sort_ddmm: function(a,b) {
			mtch = a[0].match(sorttable.DATE_RE);
			y = mtch[3]; m = mtch[2]; d = mtch[1];
			if (m.length == 1) m = '0'+m;
			if (d.length == 1) d = '0'+d;
			dt1 = y+m+d;
			mtch = b[0].match(sorttable.DATE_RE);
			y = mtch[3]; m = mtch[2]; d = mtch[1];
			if (m.length == 1) m = '0'+m;
			if (d.length == 1) d = '0'+d;
			dt2 = y+m+d;
			if (dt1==dt2) return 0;
			if (dt1<dt2) return -1;
			return 1;
		},
		sort_mmdd: function(a,b) {
			mtch = a[0].match(sorttable.DATE_RE);
			y = mtch[3]; d = mtch[2]; m = mtch[1];
			if (m.length == 1) m = '0'+m;
			if (d.length == 1) d = '0'+d;
			dt1 = y+m+d;
			mtch = b[0].match(sorttable.DATE_RE);
			y = mtch[3]; d = mtch[2]; m = mtch[1];
			if (m.length == 1) m = '0'+m;
			if (d.length == 1) d = '0'+d;
			dt2 = y+m+d;
			if (dt1==dt2) return 0;
			if (dt1<dt2) return -1;
			return 1;
		},

		shaker_sort: function(list, comp_func) {
			// A stable sort function to allow multi-level sorting of data
			// see: http://en.wikipedia.org/wiki/Cocktail_sort
			// thanks to Joseph Nahmias
			var b = 0;
			var t = list.length - 1;
			var swap = true;

			while(swap) {
				swap = false;
				for(var i = b; i < t; ++i) {
					if ( comp_func(list[i], list[i+1]) > 0 ) {
						var q = list[i]; list[i] = list[i+1]; list[i+1] = q;
						swap = true;
					}
				} // for
				t-=1;

				if (!swap) break;

				for(var i = t; i > b; i-=1) {
					if ( comp_func(list[i], list[i-1]) < 0 ) {
						var q = list[i]; list[i] = list[i-1]; list[i-1] = q;
						swap = true;
					}
				} // for
				b++;

			} // while(swap)
		}
		}

		/* ******************************************************************
		Supporting functions: bundled here to avoid depending on a library
		****************************************************************** */

		// Dean Edwards/Matthias Miller/John Resig

		/* for Mozilla/Opera9 */
		if (document.addEventListener) {
			document.addEventListener("DOMContentLoaded", sorttable.init, false);
		}

		/* for Internet Explorer */
		/*@cc_on @*/
		/*@if (@_win32)
			document.write("<script id=__ie_onload defer src=javascript:void(0)><\/script>");
			var script = document.getElementById("__ie_onload");
			script.onreadystatechange = function() {
				if (this.readyState == "complete") {
					sorttable.init(); // call the onload handler
				}
			};
		/*@end @*/

		/* for Safari */
		if (/WebKit/i.test(navigator.userAgent)) { // sniff
			var _timer = setInterval(function() {
				if (/loaded|complete/.test(document.readyState)) {
					sorttable.init(); // call the onload handler
				}
			}, 10);
		}

		/* for other browsers */
		window.onload = sorttable.init;

		// written by Dean Edwards, 2005
		// with input from Tino Zijdel, Matthias Miller, Diego Perini

		// http://dean.edwards.name/weblog/2005/10/add-event/

		function dean_addEvent(element, type, handler) {
			if (element.addEventListener) {
				element.addEventListener(type, handler, false);
			} else {
				// assign each event handler a unique ID
				if (!handler.\$\$guid) handler.\$\$guid = dean_addEvent.guid++;
				// create a hash table of event types for the element
				if (!element.events) element.events = {};
				// create a hash table of event handlers for each element/event pair
				var handlers = element.events[type];
				if (!handlers) {
					handlers = element.events[type] = {};
					// store the existing event handler (if there is one)
					if (element["on" + type]) {
						handlers[0] = element["on" + type];
					}
				}
				// store the event handler in the hash table
				handlers[handler.\$\$guid] = handler;
				// assign a global event handler to do all the work
				element["on" + type] = handleEvent;
			}
		};
		// a counter used to create unique IDs
		dean_addEvent.guid = 1;

		function removeEvent(element, type, handler) {
			if (element.removeEventListener) {
				element.removeEventListener(type, handler, false);
			} else {
				// delete the event handler from the hash table
				if (element.events && element.events[type]) {
					delete element.events[type][handler.\$\$guid];
				}
			}
		};

		function handleEvent(event) {
			var returnValue = true;
			// grab the event object (IE uses a global event object)
			event = event || fixEvent(((this.ownerDocument || this.document || this).parentWindow || window).event);
			// get a reference to the hash table of event handlers
			var handlers = this.events[event.type];
			// execute each event handler
			for (var i in handlers) {
				this.\$\$handleEvent = handlers[i];
				if (this.\$\$handleEvent(event) === false) {
					returnValue = false;
				}
			}
			return returnValue;
		};

		function fixEvent(event) {
			// add W3C standard event methods
			event.preventDefault = fixEvent.preventDefault;
			event.stopPropagation = fixEvent.stopPropagation;
			return event;
		};
		fixEvent.preventDefault = function() {
			this.returnValue = false;
		};
		fixEvent.stopPropagation = function() {
		this.cancelBubble = true;
		}

		// Dean's forEach: http://dean.edwards.name/base/forEach.js
		/*
			forEach, version 1.0
			Copyright 2006, Dean Edwards
			License: http://www.opensource.org/licenses/mit-license.php
		*/

		// array-like enumeration
		if (!Array.forEach) { // mozilla already supports this
			Array.forEach = function(array, block, context) {
				for (var i = 0; i < array.length; i++) {
					block.call(context, array[i], i, array);
				}
			};
		}

		// generic enumeration
		Function.prototype.forEach = function(object, block, context) {
			for (var key in object) {
				if (typeof this.prototype[key] == "undefined") {
					block.call(context, object[key], key, object);
				}
			}
		};

		// character enumeration
		String.forEach = function(string, block, context) {
			Array.forEach(string.split(""), function(chr, index) {
				block.call(context, chr, index, string);
			});
		};

		// globally resolve forEach enumeration
		var forEach = function(object, block, context) {
			if (object) {
				var resolve = Object; // default
				if (object instanceof Function) {
					// functions have a "length" property
					resolve = Function;
				} else if (object.forEach instanceof Function) {
					// the object implements a custom forEach method so use that
					object.forEach(block, context);
					return;
				} else if (typeof object == "string") {
					// the object is a string
					resolve = String;
				} else if (typeof object.length == "number") {
					// the object is array-like
					resolve = Array;
				}
				resolve.forEach(object, block, context);
			}
		};
-->
</script>
<style type='text/css'>
		table
		{
			font-family: Verdana, sans-serif; font-size: 12px;
			color: black;
			border: solid 1px black;
		}
		table tbody td
		{
			border: solid 1px black;
			padding: 2px;
		}
		table.center
		{
			margin-left:auto;
			margin-right:auto;
		}
		h1, h2
		{
			text-align: center;
		}
    </style>
</head>
<body>
<h1>Moonlight Test Sites</h1>
<center>
	<p>This is a list of various Silverlight websites that we are using to test Moonlight, and the current status of each test.
				To edit this page, modify trunk/moon/demo-status.txt.</p>
	<p>The rating of each site is a somewhat arbitrary rank of 0-4 based on the functionality and appearance of the site.
	</p>
</center>
	<br/>
	<table class='sortable center'>
	<tr>
			<th width='50'>Rating</th><th width='100'>Icons</th><th>Description</th></tr>
		<tr><td>-1</td><td>$STAR_N1</td>
		<td>Site url is broken or the application is broken on Silverlight</td></tr>

		<tr><td>0</td><td>$STAR_0</td>
		<td>Site has no functionality or crashes Firefox</td></tr>

		<tr><td>1</td><td>$STAR_1</td>
		<td>Site has minimal functionality and/or major cosmetic issues.</td></tr>

		<tr><td>2</td><td>$STAR_2</td>
		<td>Site has some functionality and/or cosmetic issues</td></tr>

		<tr><td>3</td><td>$STAR_3</td>
		<td>Site has most features working and/or has minor cosmetic issues</td></tr>

		<tr><td>4</td><td>$STAR_4</td>
		<td>All feature of the site work reliably and has proper appearance</td></tr>
	</table>
	<br/>

<table class='sortable center'>
	<tr><th width='100'>Rating</th><th width='200'>Site</th><th width='600'>Comments</th></tr>
	$ROWS
</table>

</body>
</html>
EOF

#exit $RESULT

