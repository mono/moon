function OnMoreInfoClick (event)
{
  var target_id = String (event.currentTarget.id);
  var details_id = target_id.replace("more-info", "#details");
  $(details_id).show('fast');
  return false;
}

function TooltipHandler (sender)
{
  var target_id = String (sender.id);
  var changelog_id = target_id.replace("detail", "#changelog");
  return $(changelog_id).html ();
}

function OnReady ()
{
  $('.click').click(OnMoreInfoClick);
  $('.detail').tooltip( {
    bodyHandler: function () { return TooltipHandler (this); },
    showURL: false});
}

$(document).ready(OnReady);
