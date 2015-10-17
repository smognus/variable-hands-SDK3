Pebble.addEventListener("ready", function() {
  Pebble.addEventListener("showConfiguration", function() {
    Pebble.openURL('https://smognus.github.io/variable-hands-config/index.html');
  });
  
  Pebble.addEventListener('webviewclosed', function(e) {
  // Decode and parse config data as JSON
  var config_data = JSON.parse(decodeURIComponent(e.response));
  console.log('Config window returned: ', JSON.stringify(config_data));
  // Prepare AppMessage payload
  var dict = {
    'tickSetting': config_data.tickSetting,
    'daySetting': config_data.daySetting,
    'batterySetting': config_data.batterySetting,
    'secondStartSetting': config_data.secondStartSetting,
    'secondEndSetting': config_data.secondEndSetting
  };

  // Send settings to Pebble watchapp
  Pebble.sendAppMessage(dict, function(){
    console.log('Sent config data to Pebble');  
  }, function() {
    console.log('Failed to send config data!');
  });
  });
});