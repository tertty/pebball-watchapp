var socket;

function connect_gameserver_websocket(gameserver_id) {
    var url = "wss://echo.websocket.org";
    
    socket = new WebSocket(url);
    
    socket.onopen = function(e) {
        console.log("[open] Connection established");
        // socket.send("My name is John");
        bubble_up_app_message({ 'WEBSOCKET_ACK_EVENT': 17 });
    };
    
    socket.onmessage = function(event) {
        var converted_payload = JSON.parse(event.data);

        console.log(`[message] Data received from server: ${JSON.stringify(converted_payload)}`);

        if (converted_payload['ackd_event']) {
            bubble_up_app_message({ 'WEBSOCKET_ACK_EVENT': converted_payload['ackd_event'] });
        } else if (converted_payload['event'] == 20) {
            bubble_up_app_message({ 'WEBSOCKET_ACK_EVENT': converted_payload['event'], 'PITCH_REACH_TIME': converted_payload['pitch_reach_time'] });
        } else {
            console.log("Received unsupported message...");
        }
    };
    
    socket.onclose = function(event) {
        if (event.wasClean) {
            console.log(`[close] Connection closed cleanly, code=${event.code} reason=${event.reason}`);
        } else {
            // e.g. server process killed or network down
            // event.code is usually 1006 in this case
            console.log('[close] Connection died');
        }
    };
    
    socket.onerror = function(error) {
        console.log(`[error]`);
    };
}

function debug_ack_event(event_id) {
    if (event_id == 19) {
        socket.send(JSON.stringify({    
            'event': 20,
            'pitch_reach_time': 9
        }));

        return;
    }

    socket.send(JSON.stringify({    
        'ackd_event': event_id
    }));
}

function send_start_game_event(wrist_position) {
    socket.send(JSON.stringify({    
        'event': 18,
        'wrist_position': wrist_position
    }));

    debug_ack_event(18);
}

function send_batter_position_ready_event() {
    socket.send(JSON.stringify({    
        'event': 19
    }));

    debug_ack_event(19);
}

function batter_swing_event(swing_epoch) {
    socket.send(JSON.stringify({    
        'event': 21,
        'swing_epoch': swing_epoch
    }));

    debug_ack_event(18);
}

function bubble_up_app_message(dictionary_value) {
    Pebble.sendAppMessage(dictionary_value,
        function(e) {
            console.log('Info has bubbled up to Pebble successfully!');
        },
        function(e) {
            console.log('Error bubbling up info to Pebble!');
        }
    )
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
    function(e) {
        console.log('PebbleKit JS ready!');
    }
);
  
  // Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
    function(e) {
        console.log(`AppMessage received: ${JSON.stringify(e.payload)}!`);

        if (e.payload['17']) {
            connect_gameserver_websocket();
        }

        if (e.payload['18']) {
            send_start_game_event();
        }

        if (e.payload['19']) {
            send_batter_position_ready_event();
        }

        if (e.payload['20']) {
            console.log("Unsupported message!");
        }

        if (e.payload['21']) {
            batter_swing_event();
        }
    }                     
);