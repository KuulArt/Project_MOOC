# Project_MOCS (Modular OSC Control Surface)
This was my dissertation project. 

The current state of this project:
Done: 
1) Written a simple two way communication protocol between computer and arduino based controller.
2) Pure Data patch that translates incoming data into REAPER DAW OSC messages.
3) Added iPad integration through OSC with PureData patch.

Future plans:
1) Refactor arduino code. Make it modular.
2) Rewrite PC side of code in a Object Oriented language.
3) Add support for different radio modules.

The abstract of my project was:

"Music recording studios of today are using more software based editing and processing tools than ever before. The processing power of such systems is increasing in a very fast pace but the actual means of controlling the software is still dominated by a traditional keyboard and mouse combination.

The Modular OSC Control Surface (MOCS) offers a new approach on controlling the software by combining technologies such as wireless networks, open-source hardware and physical interaction sensors in a one complete system. The modularity of this device gives the engineer the ability to combine different sensor modules in a control surface that meets his preferred workflow.

The MOCS system uses Arduino microcontrollers to read values from the connected sensors. The read data is then transmitted through a radio frequency network to the data interface module where it initiates a specific OSC command to a digital audio workstation.

To test the system a prototype module which uses buttons and potentiometers was built. The tests concluded that the MOCS can be used efficiently to simplify tasks such as writing automation and navigating around the session. Additionally the built-in infrastructure allows inclusion of new features that would make MOCS system more than a music editing controller. The use of various kinds of physical interaction sensors would enable the project to be used as a basis for a new breed of digital music instruments that interact with the surrounding environment."



