## what

A demo project to lean how to write SNES games.

In it I'm slowly building the means to be able to build an actual project.

Tips so far:
- The project has a sensible CMake setup. Georgjz has a great [setup](https://georgjz.github.io/snesaa10/)
  that was almost exactly what I wanted/needed.
- For VSCode I'm using a locally modified version of the
  [ASM Code Lens](https://github.com/maziac/asm-code-lens) plugin, which I modified locally to add
  syntax highlighting for 65816 and 6502. ASM Code Lens is nice because it adds jump-to-definition
  and documentation bubbles for labels and symbols, which greatly increases my coding enjoyment.
- Auto-completion of IO registers. I copied the whole of the register info pages from the
  [SNESdev Wiki](https://snes.nesdev.org/wiki/SNESdev_Wiki), and made them into assembly
  files with comments, so ASM Code Lens can pop up the definitions in-line. Works great!
- The SNES Development Server Discord channel is a great place to hang out and ask questions.
  Seems like the people that make some of the libraries and tools I use hang out there as well.
- [Mesen2](https://github.com/SourMesen/Mesen2) seems to be by far the best dev SNES emulator at
  the moment. Takes debug files from the assembler, which means it will display your source files
  with comments and all and you can put breakpoints straight in those. Plus the maker, Sour, helped
  me out with an issue on Discord. You can't beat that.
- For audio, looks like [Terrific Audio Driver](https://github.com/undisbeliever/terrific-audio-driver)
  It's pretty featureful, is very actively worked on ATM and maker hangs out on Discord.
- Assembler: [cc65](https://cc65.github.io), or rather the assembler, ca65. Seems to be what all the
  devs outside of romhacking use. Also integrates well with Mesen and Terrific Audio Driver.
- For graphic file conversions I'm currently using [SuperFamiconv](https://github.com/Optiroc/SuperFamiconv).
  Commandline tool that integrates nicely in CMake.
- Documentation:
  - [SNESdev Wiki](https://snes.nesdev.org/wiki/SNESdev_Wiki)
  - [SFC Development Wiki](https://wiki.superfamicom.org)
  - [nocash SNES specs](https://problemkaputt.de/fullsnes.htm)
  - [opcode summaries](https://undisbeliever.net/snesdev/65816-opcodes.html)
  - [Programming the 65816](https://www.amazon.com/Programming-65816-Including-65C02-65802/dp/0893037893)
  - [stack article](http://6502org.wikidot.com/software-65816-parameters-on-stack) on 6502.org wiki
  - [65816S programmer manual](https://www.westerndesigncenter.com/wdc/documentation/w65c816s.pdf)


## acknowledgements (besides above)

- town graphic from [Ansimuz Legacy Collection](https://ansimuz.itch.io/gothicvania-patreon-collection)
- I've been following the following tutorials:
  - as already mentioned [jeorgjz](https://georgjz.github.io/snesaa01/). Pretty lenghy. In the end
    provides you with quite a decent setup.
  - Nesdoug's [SNES Programming Guide](https://nesdoug.com/2020/03/19/snes-projects/)
  - Wesley Aptekar-Cassels [SNES Development](https://blog.wesleyac.com/posts/snes-dev-1-getting-started) tutorial.