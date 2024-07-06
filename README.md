# AA2MID
## AudioArts (GBC) to MIDI converter

This tool converts music from Game Boy Color games using the AudioArts sound engine to MIDI format.

It works with ROM images. To use it, you must specify the name of the ROM followed by the number of the bank containing the sound data (in hex).
For games that contain 2 banks of music, you must run the program multiple times specifying where each different bank is located. However, in order to prevent files from being overwritten, the MIDI files from the first bank must either be moved to a separate folder or renamed.

Note that for some games, the first track is "empty". This is normal.

This tool was based on my own reverse-engineering of the sound engine. Several different variations of the driver were accounted for to ensure that every game using the sound engine is supported.

Also included is another program, AA2TXT, which prints out information about the song data from each game. However, it is "behind" compared to the MIDI converter (not every game's sequence is properly supported), and the code is quite messy.

Supported games:
  * 3-D Ultra Pinball: Thrillride
  * 102 Dalmatians: Puppies to the Rescue
  * Astérix & Obélix vs. Caesar
  * Benjamin Blümchen: Ein Verrückter Tag im Zoo
  * Bibi Blocksberg: Im Bann der Hexenkugel
  * Bibi und Tina: Fohlen Felix in Gefahr
  * Carmageddon
  * Chicken Run
  * Colin McRae Rally
  * Cool Bricks
  * Cubix: Robots for Everyone: Race 'n Robots
  * The Dukes of Hazzard: Racing for Home
  * Extreme Ghostbusters
  * The Flintstones: Burgertime in Bedrock
  * Ghosts 'n Goblins
  * Magical Drop
  * Die Maus: Verrückte Olympiade
  * Mission: Impossible
  * The Nations: Land of Legends
  * No Fear: Downhill Mountain Biking
  * NSYNC: Get to the Show
  * O'Leary Manager 2000
  * Painter
  * Rip-Tide Racer
  * Road Rash (GBC)
  * Santa Claus Junior
  * Superman: Battle for Metropolis (prototype)
  * SWIV
  * Thunderbirds
  * Tom and Jerry in Mouse Attacks!
  * Tony Hawk's Pro Skater 3
  * Toobin'
  * Tootuff
  * Total Soccer 2000/Barca Total 2000
  * Wendy: Der Traum von Arizona
  * Worms Armageddon

## To do:
  * Better support for "held" notes (currently acts the same as rest)
  * Support for the GBA version of the sound engine
  * GBS file support
