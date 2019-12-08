### ESP32Boy Case

This case was designed around reusing a PSX controller to lower the overall part count.  Also, using a PSX controller means only 6 wires to connect.

Please note, this is still being designed and your results may vary.  To date I have confirmed a good fit with the PSX controller board and buttons.  I also have test-fitted the screen and moved the SD card slot accordingly.  That being said, right now I still have yet to assemble the whole thing and put internal mounting locations, so in the spirit of a true prototype anything else is going to be done with duct tape and bubble gum until I can finalize things a bit better.


### Parts used

1) ESP32 WRover, I use "Geekworm ESP32 WROVER PCB Development Board with 8MB PSRAM"
2) ili9341-based screen.  I'm using "HiLetgo ILI9341 2.8 SPI TFT LCD Display Touch Panel 240X320 with PCB 5V/3.3V STM32" -- If you get a different size screen then you'll have to adjust the model in OpenSCAD (and have fun with that, it's organized but still could use a lot more organization still) -- If you get another 2.8" display make sure that the SD card is in the same relative location otherwise you'll have to adjust the SD card offsets in the model, which is not as hard as changing the size of the whole display.
3) Class D Amp, I use "Mini PAM8403 Digital Power Amplifier Board 23W Class D 2.5-5V USB Power"
4) TP4056 battery charger board, I use "Icstation TP4056 Micro USB 5V 1A 18650 Lithium Battery Charger Board"
5) 3.7v Lithium Ion battery, I use "MakerFocus 4pcs 3.7V Lithium Rechargable Battery, 802540 3.7V 1000mAh Battery "
6) 8ohm 0.5watt speaker.  I use "uxcell 4 Pcs 0.5W 8 Ohm 26mm Internal Magnet Electronic Toy Speaker Amplifier"

### Printing

I recommend printing with a layer height of 0.15 or smaller to ensure a better surface finish and also to make sure there are many layers supporting the screw holes.  If you are using a material that supports post-print annealing (e.g. bake it in the oven) that would improve rigidity but is hopefully not necessary.  I recommend printing with a 0.4 nozzle or smaller because those screw holes and gaps for the controller buttons are really small, otherwise expect that you will need to do post-print adjustments.  Final note is that if you're using Cura pay attention to the advanced option "Slicing tolerance" which can make a big difference in how well things will fit.  I've been trying both "includsive" and "middle" but your mileage may vary.

### Assembling

This really depends on if you want to experiment or make everything permanent.  For experimentation use female/female prototype wiring and solder male headers to everything.  This makes it much easier to confirm everything works.

Remove all the screws from the PSX controller and carefully dismantle it without breaking any wires going to the shoulder buttons boards.  I haven't decided where to put them in the final design but hopefully it will involve using the original wiring for simplicity.  Save all the screws and innards as they will be reused.

You will most likely need to desolder the PSX controller wiring plug from the board and solder new wires, but that's your preference.  Note the wiring order on the board since it's all color-coded.  More information about the PSX pinout is in the main readme in the top directory of this project.

Once you're fairly sure that everything is working, mount the LCD screen with very small and very short screws.

Carefully place the controller buttons into the case, noting that there are notches that prevent you from putting buttons in the wrong places.  When assembling the D-Pad area make sure to place the X cross correctly.

Mount the ESP32 to the back of the LCD screen using heavy-duty 2-sided tape, with the main part of the ESP32 module (the silver part) facing down.  This makes it easier to solder to the board.

Connect the battery to the charger board and then connect the ground of the charger board to the ESP32 ground.  However note that the charger board outputs too much voltage for the ESP32, which you can drop using a simple zener diode going from the VCC of the charger board's output to the 3.3 VCC pin on the ESP32.

Connect the speaker output, VCC, and ground pins to the Class D amp board and then connect the speaker.  Mount the speaker using hot glue to the case between the controller supports.  Be careful not to damage the membrane of the speaker!

The LCD, PSX, and SD connections are provded in the main readme.  See the excel wiring pinout excel file for helpful diagrams of all these locations. 
