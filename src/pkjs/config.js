module.exports = [
    {
      "type": "heading",
      "defaultValue": "Pebball! Configuration"
    },
    {
      "type": "text",
      "defaultValue": "Advanced options for self-hosting"
    },
    {
      "type": "section",
      "items": [
        {
          "type": "heading",
          "defaultValue": "Pebball! Web Server Configuration"
        },
        {
            "type": "input",
            "messageKey": "WEBSERVER_URL",
            "defaultValue": "",
            "label": "Web Server URL (without http or https header!)",
            "attributes": {
              "placeholder": "eg: 192.168.1.1:3000",
            //   "limit": 10,
            //   "type": "email"
            }
        }
      ]
    },
    {
      "type": "submit",
      "defaultValue": "Save Settings"
    }
  ];