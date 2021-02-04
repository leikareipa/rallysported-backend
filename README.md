# RallySportED's backend

RallySportED is a cool asset editor for the classic MS-DOS-era racing game Rally-Sport. It lets you make new tracks etc.

The backend of RallySportED consists of DOS-based tools to inject RallySportED-made content - e.g. tracks - into the game, as the game itself wasn't made to allow modding. So these tools are absolutely required to play someone's new Rally-Sport creations.

You can find pre-compiled binaries of the RallySportED backend on [Tarpeeksi Hyvae Soft's website](https://www.tarpeeksihyvaesoft.com/). If you're not interested in the code, that's where you should be heading right now; otherwise, read on.

## The tools

Two tools comprise the RallySportED backend:
- RLOAD
- RAI

The RLOAD tool, which you can find under the [rload/](./rload/) folder, is responsible for loading modded content into the game. It basically makes a copy of the Rally-Sport executable, modifies the copy to inject the new content, then runs Rally-Sport with the new content via the modified executable.

The RAI tool, found under the [rai/](./rai/) folder, is for editing RallySportED-made tracks' AI opponents. With this tool, you race a lap around the given track and the tool saves your lap as the track AI's new driving line (AI in Rally-Sport is just a pre-determined racing line that the CPU driver sheepishly follows).

The backend's pre-compiled binary distribution, found on [Tarpeeksi Hyvae Soft's website](https://www.tarpeeksihyvaesoft.com/), comes with further instructions for using the RLOAD and RAI tools.
