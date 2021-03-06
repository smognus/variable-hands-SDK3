Pebble.addEventListener("ready", function() {
  Pebble.addEventListener("showConfiguration", function() {
    Pebble.openURL('http://smognus.github.io/variable-hands-config/index.html');
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
    'secondEndSetting': config_data.secondEndSetting,
    'digitalSetting': config_data.digitalSetting,
    'windowColorSetting': parseInt(config_data.windowColorSetting, 16),
    'windowBorderColorSetting': parseInt(config_data.windowBorderColorSetting, 16),
    'windowTextColorSetting': parseInt(config_data.windowTextColorSetting, 16),
    'lightThemeSetting': config_data.lightThemeSetting,
    'secondHandColorSetting': parseInt(config_data.secondHandColorSetting, 16),
    'secondOutlineColorSetting': parseInt(config_data.secondOutlineColorSetting, 16)
  };
    console.log('AppMessage contents:', JSON.stringify(dict));
  // Send settings to Pebble watchapp
  Pebble.sendAppMessage(dict, function(){
    console.log('Sent config data to Pebble');  
  }, function() {
    console.log('Failed to send config data!');
  });
  });
});