# python-osc-server

This script is supposed to be run on the Elk Pi board at startup. It waits for specific OSC messages on port 6042 which are sent by the puredata controller patch (see [python-osc-client]() for more information). The messages are then parsed and the corresponding actions are executed.
Replies are sent back to the controller (whose ip address was passed with a handshake (/pyshake) message) on port 7042.

The messages and relative actions are the following:
- ```/pyshake <board-ip> <controller-ip>``` : handshake message, sent by the controller at startup. The board replies with a handshake message to the controller on port 7042. ```board-ip``` currently not used.
- ```/startPlugin <piano/acoustic/electric>```: starts the specified plugin. 
- ```/stopPlugin```: stops the Sushi DAW entirely.
- ```/disconnect```: disconnects the controller from the board.
- ```/rename <new-name>```: renames the unnamed recording to ```new-name```. If there are zero or more than one unnamed recordings, an error message is sent back to the controller (```/renamed s "errorNoFile"``` or ```/renamed s "errorTooManyFiles"```).
If the new name is already taken, renaming is not performed and an error message is sent back to the controller (```/renamed s "errorOverwrite"```).
- ```/deleteUnnamed```: deletes allt the recordings that have not been renamed yet.
- ```/deleteAllButLast```: deletes all the recordings except the most recent one.


<!-- 
dispatcher.map("/deleteUnnamed", deleteUnnamed)
dispatcher.map("/deleteAllButLast", deleteAllButLast)  -->