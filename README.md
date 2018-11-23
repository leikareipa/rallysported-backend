# RallySportED
The cult DOS game Rally-Sport (1996) was largely without modding tools&mdash;until RallySportED came along. Released by a third party, i.e. me, on the 20th anniversary of the game, RallySportED allows one to edit many of Rally-Sport's internal assets, like tracks, textures, and certain hard-coded functionality.

To-date, two versions of the game are known to exist. The initial release, in late-1996, is considered a free demo. The second release, in mid-1997, is the full, commercial release, but adds few new features. Since the demo version is not far behind in features and is accessible to a larger number of people, RallySportED targets this version only.

This repo contains RallySportED's full source code, with the exception of the RGEO module (see below). The repo tracks my local one with low granularity only.

For pre-compiled binaries, including a copy of the Rally-Sport demo, you can navigate to [Tarpeeksi Hyvae Soft's website](http://tarpeeksihyvaesoft.com/soft).

### Modules
RallySportED consists of four modules: ```RGEO```, ```RLOAD```, ```RTEX```, and ```RAI```. For a slightly more utilitarian description of these modules, and to avoid having to duplicate that description here, download the binary distribution of RallySportED from the link above, and see the included readme and faq.

In short, ```RGEO```, a 3d track editor, is the main module. It's written in C++, runs in DOS, and should perform adequately even on a 66-MHz i486. Note that the RGEO codebase is not included in this source release of RallySportED.

```RLOAD``` is a DOS-based module for loading custom tracks into the game.

```RTEX``` is a DOS-based texture editor, meant to augment the lack in the DOS version of RGEO of a texture editor.

Finally, ```RAI```, another DOS tool, allows the user to edit a custom track's AI opponent.

You can see that, much like Rally-Sport itself, the RallySportED modules are all DOS-based. In case that worries you, let it not, since you can simply run them in DOSBox on your modern desktop. That's what I do.

### Building
Each module comes with its own readme file that has instructions on building; but generally speaking, you'll need the (free) fasm assembler, and not much else.

![A screenshot of RallySportED](http://tarpeeksihyvaesoft.com/soft/img/rsed2_b.png)
