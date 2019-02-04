# RallySportED-DOS
Versions of [RallySportED's](../../../rallysported) track, texture, and AI editors for DOS. These will run on 386/486-class hardware in DOS, and/or on modern systems under DOSBox.

You can find pre-compiled binaries of the DOS toolset on [Tarpeeksi Hyvae Soft's website](http://tarpeeksihyvaesoft.com/soft).

### Modules
The DOS toolset consists of four modules: ```RGEO```, ```RLOAD```, ```RTEX```, and ```RAI```. To learn more about them, you can leaf through the documentation that comes with the tools' binary distribution (see link, above). Put briefly, ```RGEO``` is a 3d track editor, written in C++ and is not included in this repo. ```RLOAD``` is a module for loading RallySportED-made tracks into the game. ```RTEX``` is a texture editor with which you can edit tracks' surface textures. ```RAI``` lets you edit the tracks' AI.

### Building
You'll find a readme file in each module's folder with instructions on building that module. Generally speaking, you'll need the free fasm assembler, and not much else.

![A screenshot of RallySportED](http://tarpeeksihyvaesoft.com/soft/img/rsed2_b.png)
