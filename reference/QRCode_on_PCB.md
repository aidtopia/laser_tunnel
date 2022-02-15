# QR Code KiCad PCB

Adrian McCarthy 2022-02-15

Notes from attempting to put a QR code on the PCB for the Github repo for the Laser Tunnel project.

It'll have to be pretty big for most fabricators.  And you'll have to find clear space on your board to place it.  Pads, vias, and even traces under the soldermask can distort or confuse a code scanner.  If the back of your board is mostly the ground plane, you have a chance.

KiCad 6 has a QR code plugin for the New Footprint Wizard in the Footprint Editor.

* Create a project-local footprint library.
* In the Footprint Editor, select the new library.
* Start the New Footprint Wizard (DIP icon with red star).
* The wizard lists footprint generator plugins.  Select "2D Barcode QRCode".
* The generator has two pages of parameters:  Barcode and Caption
* Barcode pixel width should be kept as high as possible.  Online I found recommendations for at least 0.5 mm.  I dropped mine to 0.44 mm to squeeze it into the available space.
* I chose silkscreen.  I chose Negative because my silkscreen is white and my soldermask is dark.  (QR codes usually have dark ink on a bright background.)  I set the border margin to 1 pixel because I think the scanners need a frame of the "background" color to find the code.
* I disabled the caption since I didn't have room for it.  I could add the same text to the PCB elsewhere.
* Export the footprint to the editor.

If your content is a URL, you will likely see an "Invalid argument" warning, then the footprint will appear in the editor, but it won't be in any library.  I'm pretty sure the problem is that the wizard named the footprint with the URL, and `https://` in the middle of a path name isn't valid (at least, not on Windows).  Fear not.

In the File menu, open the Footprint Properties.  On the "General" tab, below the table is a field labeled "Footprint name".  Change that to something descriptive, like "MyProjectQRCode_Silkscreen".

While the footprint properties are open:

* Uncheck Show for the Reference designator.
* Change the Layer for the Value to F.Fab.
* Check the "Not in schematic" box.

Click OK to dismiss the properties dialog box and save your changes.

If you had the URL, you will see another warning showing a truncated file name.  That's the system trying to delete the file with the footprint's original name (the bogus file name with the URL in it).  Once you dismiss the warning, the corrected footprint name should appear in your library.

In the PCB editor, select Add Footprint, choose your QR code from your library, and place it on the board.  Remember that you need a pretty big clear space.

While it's still selected, open the Footprint Properties (type 'e').  If you want the code on the back of the board, there's a drop-down labeled Side in the lower part of the dialog box.  You can manually assign a reference designator if you like.

Now bring up the 3D viewer with Alt+3, and look at the rendering to verify your code is legible, unobstructed, and looks like the code would look on a sticker (dark ink on a bright background).  Turn on the raytracing option, size the window until the board is actual size, then try scanning the code from the rendering to see if it works.

Note that the actual silkscreen may not be as crisp as the final boards.  Only a test run will tell you that.
