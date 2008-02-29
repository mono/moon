function media_stop(sender, args) 
{
    sender.findName("MediaPlayer").stop();
}

function media_pause(sender, args) 
{
    sender.findName("MediaPlayer").pause();
}

function media_begin(sender, args) 
{
    player = sender.findName("MediaPlayer");
 //   player.Source = playlist_source;
    player.play();
	alert (player.Source);
}

function onCurrentStateChanged(sender, args)
{
        player = sender.findName("MediaPlayer");
        alert(player.currentState);
}

function media_forward(sender, args)
{
    player = sender.findName("MediaPlayer");
    player.source = "test.asx";
}

function plog (html)
{
	var d = document.getElementById ("plog");
	d.innerHTML = html + d.innerHTML;
}

function media_state_changed(sender, args)
{
    // Get the TextBlock object to display the current status.
    var mediaStateTextBlock = sender.findName("mediaStateTextBlock");
    // Get the MediaElement object
    var media = sender.findName("MediaPlayer");
   
    mediaStateTextBlock.Text = media.CurrentState;

	plog ("<hr />");
	plog ("attributes: " + media.Attributes + "<br />");
	plog ("count: " + media.Attributes.count + "<br />");

	for (var i = 0; i < media.Attributes.count; i++) {
		var attribute = media.Attributes.getItem (i);

		plog ("name: " + attribute.Name + ", value:" + attribute.Value + "<br />");
	}
}








