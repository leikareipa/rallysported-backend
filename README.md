# RallySportED
The cult DOS game Rally-Sport (1996) was largely without modding tools&mdash;until RallySportED. Released by a third party on the 20th anniversary of the game, RallySportED allows the user to edit many of Rally-Sport's internal assets, like tracks, textures, and hard-coded functionality.

To-date, two versions of the game are known to exist. The initial release, in late-1996, was labeled internally as a beta but externally a free demo. The second release, in mid-1997, required payment but added few new features. Since the demo version is not far behind in features and is more accessible legally, RallySportED targets this version only.

This repo contains RallySportED's full source code, with the exception of one of its DOS-based modules (see below). The repo tracks my local one with low granularity only.

For pre-compiled binaries, including a copy of the Rally-Sport demo, you can navigate to [Tarpeeksi Hyvae Soft's pretty website](http://tarpeeksihyvaesoft.com/soft).

### Modules
RallySportED consists of four modules: ```RGEO```, ```RLOAD```, ```RTEX```, and ```RAI```. For a slightly more utilitarian description of these modules, and to avoid duplicating that description here, download the binary distribution of RallySportED (link above), and see the included readme and faq.

In short, ```RGEO```, a 3d track editor, is the main module. It is written in C++, runs in DOS, Linux and Windows, and should perform adequately even on a 66-MHz i486. For graphics, RGEO supports a pseudo-3d view for slow CPUs (the DOS version), hardware acceleration through Glide or OpenGL for Pentium-class or better CPUs, and full software 3d rendering for newer CPUs. Note that the DOS version of RGEO is a separate codebase that is not included in this source release of RallySportED.

```RLOAD``` is a purely DOS-based module (like Rally-Sport itself), and is required to load custom tracks into the game. It, like the next two modules, is written in learner-level 386 assembly.

```RTEX``` is a DOS-based texture editor, meant to augment the lack in the DOS version of RGEO of a texture editor. If you want a full retro experience, the combination of RTEX and the DOS version of RGEO will be the ticket.

Finally, ```RAI```, another DOS tool, allows easy editing of the game's AI opponent.

You'll note that many of the modules are DOS-based. The reason is that Rally-Sport itself is a DOS game, rendering DOS a natural platform for extending it in certain cases. For instance, RAI requires the game to be running to operate, necessitating a DOS-compatible environment, to begin with. (In modern usage, that environment will invariably be the convenient DOSBox.)

### Building
Each module comes with its own readme file that has instructions on building.

Generally  speaking, you may need the fasm assembler for RLOAD, RTEX, and RAI; and a GCC-compatible toolset for RGEO.

### A note on RallySportED's retro roots
RallySportED generally prefers to revel in the retro nature of DOS gaming; and the code will often express a desire to keep the needs of older architectures in mind, in addition to more modern ones.

![A screenshot of RallySportED](http://tarpeeksihyvaesoft.com/soft/img/rsed2_b.png)
