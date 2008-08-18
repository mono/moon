function OnMoreInfoClick (event)
{
  var target_id = String (event.currentTarget.id);
  var details_id = target_id.replace("more-info", "#details");
  $(details_id).show('fast');
  return false;
}

function OnReady ()
{
  $('.click').click(OnMoreInfoClick); 
}

$(document).ready(OnReady);
