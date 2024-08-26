# Casio CTK-500 MIDI-fy

http://amatriz.net/posts/midi-fy-ing-a-casio-ctk-500-keyboard-with-arduino-leonardo/

# Arduino Leonardo pin-out to CTK-500 Keyboard Flat Connector

First connect the wires according to this diagram. The accompanying code that comes later actually injects voltage over the KO scanning matrix lines (make sure to Google for the CTK-500 Service Manual) - that's why I recommend specifically not connecting the DC voltage to the back of CTK-500. During testing I became aware that the Red LED lights of the CTK-500 were lighting-up just from the Arduino driving of the KO lines, which are indeed connected to the LEDs according to the Service Manual. I strongly recommend against applying DC voltage to the main-board (i.e. connecting the barrel power plug to the back of the Keyboard). I want no complaints of MIDI-frys, hence the fair warning.

It's also worth noting that initially I tried a purely passive approach to coding where you could just probe the state of the KO lines as dictated by the CTK-500 microprocessor (CPU) but that code went nowhere because some KO lines wouldn't have enough representativity in the overall sampling. I'm clueless to why this happens, but while KO00 would have many probes and just worked, other KOs weren't in the end responsive enough.

The code that worked follows later, and it's the kind where we drive the KOs with 5v from Arduino. With this change, there's full responsivity and low-latency. But be smart, and do not energize the main-board with Arduino on - I'll say again just in case :)
![Arduino Leonardo pin-out to CTK-500 Keyboard Flat Connector](esquema_midificacao.svg "First connect the wires according to this diagram")

# First try with Female Jumper Cables and a soldered pin-header

Raphaela Elpidio from Circuit Bending Brazil restored the CTK-500 and soldered a pin header to connect Female Jumper Cables

![First try with Female Jumper Cables and a soldered pin-header](jumper-connectors-and-pin-header.jpg "Raphaela Elpidio from Circuit Bending Brazil restored the CTK-500 and soldered a pin header to connect Female Jumper Cables")

# Final work with bare-wire Jumper Cables soldered to Keyboard Flat Connector

Later on I asked my friend Adriano Amaral from ADR Eletronics to solder the bare-wire jumper cables directly to the Keyboard Flat Connector due to persistent bad contact

![Final work with bare-wire Jumper Cables soldered to Keyboard Flat Connector](probes-soldered.jpg "Later on I asked my friend Adriano Amaral from ADR Eletronics to solder the bare-wire jumper cables directly to the Keyboard Flat Connector due to persistent bad contact")

# Soldered Jumper Cables overview with main PCB showing

Overhead view of the soldered probes and the Connector in relation to the main Keyboard PCB

![Soldered Jumper Cables overview with main PCB showing](probes-soldered-interface.jpg "Overhead view of the soldered probes and the Connector in relation to the main Keyboard PCB")

# Closer look at the Soldered Connector with Jumper Cables exiting drilled hole

Raphaela Elpidio from Circuit Bending Brazil also drilled a hole to the back of the Keyboard Top-Cover allowing free flow of Jumper Cables

![Closer look at the Soldered Connector with Jumper Cables exiting drilled hole](probes-soldered-distance.jpg "Raphaela Elpidio from Circuit Bending Brazil also drilled a hole to the back of the Keyboard Top-Cover allowing free flow of Jumper Cables")

# Covered Keyboard with Jumper Cables entering the Arduino Leonardo

Ready for Playing

![Covered Keyboard with Jumper Cables entering the Arduino Leonardo](covered-and-trimmed.jpg "Ready for Playing")
