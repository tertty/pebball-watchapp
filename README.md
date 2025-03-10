# Pebball! Watchapp
Pebble Watchapp element of the Pebball! tech demo.

Pebball! is a baseball inspired interactive, motion controlled game concept for the Pebble Smartwatch. When paired with the Pebball! Web Server, this Watchapp acts as a controller for an endless batting cage reaction mini-game.

## Building Watchapp
To build the Watchapp, you need to have the Pebble SDK installed. 

While thoroughly archived and active, the official Pebble/Rebble website may be difficult due to the age of the SDK. If you'd like to try anyways, you can find instructions [here](https://developer.rebble.io/developer.pebble.com/sdk/install/index.html).

For a quicker option, the folks at Rebble have created a Ubuntu based VM with the SDK pre-installed. A link to download is [here](https://rebble-binaries.s3.us-west-2.amazonaws.com/vms/Rebble-Hackathon-Vm.zip), instructions [here](https://rebble.io/hackathon-002/vm/).

### Build commands
To build:
`pebble build`

To install on device:
`pebble install --phone <phone ip>`

To install on emulator:
`pebble install --emulator <emulator name>`
