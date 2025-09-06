## What?

This started as a demo project to lean how to write SNES games, and ended up as
a submission for the SNESDEV-2025 game jam

## No I mean what kind of game is this? Should I play it?

Yes for sure! You are an alien slave on an alien world, and you will fight to
the death against up to 3 friends/enemies to win your freedom and some arms!
Pretty exciting, right?

### Arms?

Yes arms! You see, you don't have any arms. Were you born this way? We don't
really know, but you will need to use your head to survive. There's a cannon on
it (mechanical? biological?) so you can blow each other to bits.

But don't just wacht out for your foes. The fighting arena is full of scary
things!! Some walls bounce your bullets, some explode, and some walls even kill
you. 10 different arenas of goodness. And if you don't like any of them, just
use the editor and make your own.

So yea, you're not in a cosy happy place. But the rewards for winning are
big. You'd love to be able to scratch your back even once in your life. That is
worth a lot.


## Manual

### How to play

#### Multiplayer SNES setup

For 2 players, a SNES or emulated SNES will work without any additional
setup. However a SNES only has two controller ports, so for 3 and 4 players, you
will need some way to plug in additional controllers. For this you should use a
so-called SNES [multitap](https://en.wikipedia.org/wiki/Multitap).

For a physical SNES you can still find plenty second-hand. I bought a
Bomberman-branded one. Works like a charm.

Most emulators or FPGA devices will have an option to use a simulated
multitap. How to do this will vary from device to device, but the ones I tested
(Snes9x, Mesen 2 and MiSTer) all worked without issue.

If you are going to play a 3 or 4 player game an you forget to turn on multitap,
you're going to have a bad time.


#### Controls

- left, right - move

- `X` - shoot up
- `Y` - shoot left
- `A` - shoot right
- `B` - shoot down

Press two adjacent shoot buttons to shoot diagonally.

- `L` - parry bullets
- `R` - jump

- `start` - pause/unpause


#### Game behaviour

##### Shooting

Hold a shoot direction to shoot continuously. Head cannons need to recharge
between shots, so it takes a bit of time before you can shoot the next one.

Also, the bullets it shoots are a type of energy bolt that will wear out after a
few seconds and they will disappear.


##### Parrying

You can reflect bullets with your energy shield if you time it right. The energy
shield is up for about a quarter of a second, and when a bullet hits the shield,
it will bounce off.

Normally bullets can't hit their owner, but when they're reflected by a parry
shield, they can.


##### Out of bounds and spawn portals

When the game starts, each gladiator emerges from their designated spawn
portal. However when a gladiator falls from the bottom of the screen, or when
they go off the sides or the top, they didn't just escape. We can't let that
happen! They're slaves after all.

Instead they will spawn randomly from one of the four spawn portals, to keep the
game interesting.


#### Matches

Matches are made up of a number of games. The goal of a game is to kill
each other with your head-mounted cannon (not to be confused with
headcanon). Whoever is left standing last wins the game. If the last gladiators
left alive are all killed at exactly the same time, no-one wins, and we try
again.

Whoever first wins the amount of games set by the 'games > win' config option,
wins the match and wins themselves a sturdy pair of arms for life (conditions
apply).

##### HUD

When in a game, at the top of your screen you see a bunch of gladiator heads.

On the left, next to the heart the numbers next to the heads indicate how many
lives they have left.

On the right, next to the trophy the numbers indicate how many games you have won.


#### Configure screen

In between matches and during matches when pressing `start` you will have access
to a config screen. During a match, press `start` again to return to the match.

In between matches player 1 is always the one in control of the menu.

When someone presses `start` during a game to access the menu, it will be that
person that is in control.

As for meny navigation, move your cursor to the desired option and press
`A`. When you're in a sub-screen, you can press `B` to go to the previous
screen.

During the match some of these options won't go into effect until you start a next match.


##### Main menu

- At the top, next to the gladiator heads, as you can read from the description,
  the numbers indicate how many matches each of you have won.
- `new match` - Start a new match. During a game, this will immediately start a
  new match.
- `edit` - Will allow you to edit the game map, also during a game. See the
  `edit` section for more info.
- `players` - The amount of players in the game. Will go into effect when next
  match starts. For 3 or 4 players, you will need a multitap setup. See
  the `Multitap SNES setup` section at the top.
- `hitpoints` - Clear enough I think. Will go into effect the next match.
- `games > win` - Whoever gets to this amount of games first will win the match.
- `level` - Choose the level you want to play. If you're in a game, the level
  layout you were playing will change immediately, but the current player
  hitpoints and the position of the players and bullets will stay the same.

  If you have edited a level, your changes will be overwritten by the new
  level. Right now there's no way to bring them back, even if you go back to the
  level that the changes were based on.


#### Editing a level

If you choose the `edit` config option, you will be dropped in the tile select
screen. This will give you access to all the tile types that are used in the
levels.

Every row is made up of different tile, which are of the same type. When you
move around with the direction buttons, the text below will tell you the type of
the highlighted tile.

Button layout:

- direction buttons - Move cursor across the tiles.
- `A` - To select the tile and move to the draw screen.
- `B` - Go back to the main screen
- `start` - When in a game, exit the config screen.


##### Tile types

- `passthrough` - These tiles are the empty space tiles. You can't interact
  with them. All of them act like empty space, and are decorative, even the
  cute dog-like mini deamon.
- `wall` - No frills solid wall tiles. Players can't pass, and bullets explode
  when they hit them.
- `bounce wall` - These are a little bit more interesting. Players can't pass,
  but bullets bounce off them.
- `bullet pass` - Players can't pass, but bullets will go through them like
  empty space.
- `player pass` - Reversing the previous tile type, players can go through them,
  but bullets bounce off them.
- `kill void` - Movement-wise these function like empty space, but they do
  damage to players.
- `kill block` - These act like walls, but they do damage to players when
  touched.
- `destruct wall` - Players can't pass. Whe touched by a bullet, both wall and
  bullet explode.
- `destruct bounce` - Players can't pass. Whe touched by a bullet, the wall
  explodes and the bullet bounces off. Like Arkanoid.


##### Draw mode

When you select a tile from the select screen, you will see the level screen in
draw mode where you can paint the level with the tile you selected.

Button layout for draw mode:

- direction buttons - Move the cursor/tile around the screen.
- `A` - Place tile. Hold tile and move cursor to draw the tile as you move
  around.
- `B` - Go back to the tile select screen.
- `X` - Flip your chosen tile horizontally.
- `Y` - Flip your chosen tile vertically.
- `start` - When in a game, exit the config screen.

You move the caret around the screen with the direction buttons.


## Build

Disclaimer: This setup has only been tested in a unix-like environment, on MacOS
to be specific.

### Prerequisites

- [CC65](https://cc65.github.io) (really just the assembler (`ca65`) and the
  linker (ld65))
- [Terrific Audio
  Driver](https://github.com/undisbeliever/terrific-audio-driver), aka TAD
  See TAD for its dependencies.
- [SuperFamiconv](https://github.com/Optiroc/SuperFamiconv)
- [SuperFamicheck](https://github.com/Optiroc/SuperFamicheck)
- Ninja (or use your own preferred CMake generator)
- CMake
- Python 3
- Bash

### Building

- Clone this repo
- Clone Terrific Audio Driver in the root of your local copy of this repo.
- Make sure the dependencies above are built (including TAD), and they're
  available in your binary search path.
- `tad-compiler` is expected as a relative path:
  `<project-root>/terrific-audio-driver/target/release/tad-compiler`
- execute:
```
    $ mkdir build
    $ cd build
    $ cmake .. -GNinja
    $ ninja
```

This should create an `unarmed.sfc` SNES binary, and an `unarmed.dbg` file,
handy for debugging in Mesen 2.


## Acknowledgements

All coding, graphics and music by me, except for the following, for which I'm
very grateful:

### software

- Terrific Audio Driver by Marcus Rowe. Mentioned elsewhere in this
  document. Great piece of software. MML turns out to be quite fun. Will
  hopefully explore it and the SNES sound system when I have a bit more time on
  my hands.
- RAM blanking code and CMake project code (and maybe some more snippets) by
  [georgjz](https://georgjz.github.io) at
  [Github](https://github.com/georgjz/snes-assembly-adventure-code)
- some misc library routines (in modified form) by [nesdoug](https://nesdoug.com)


### Graphics

- Both intro screen graphics made by Paco specifically for this game. Thanks so
  much!
- Stencilled part of the background layer of
  [City](https://stonegamesnh.itch.io/city) by [Stone
  Games](https://itch.io/profile/stonegamesnh)
- Boxy Bold font by [Clint
  Bellanger](http://opengameart.org/users/clint-bellanger), but modified by
  [cemkalyoncu](http://opengameart.org/users/cemkalyoncu), and
  [William.Thompsonj](https://opengameart.org/users/williamthompsonj) and also
  [devurandom](http://opengameart.org/users/usrshare) who added lowercase. At
  [Open Game Art](https://opengameart.org/content/boxy-bold-truetype-font)


### sound

- explosion by [BitingChaos](https://opengameart.org/users/bitingchaos) at [Open
  Game Art](https://opengameart.org/content/16x16-explosion)
- music samples supplied by Loafspell and nesdoug
- music samples by nesdoug (Doug Fraker) at his [own
  website](https://nesdoug.com/2022/01/27/why-b21-cents/#free-samples)
- hurt sound by [qubodup](https://opengameart.org/users/qubodup) at [Open Game
  Art](https://opengameart.org/content/15-vocal-male-strainhurtpainjump-sounds)
- shield deflect by [qubodup](https://opengameart.org/users/qubodup) at [Open
  Game Art](https://opengameart.org/content/impact)
- gun shot by Michel Baradari at [Open Game
  Art](https://opengameart.org/content/4-projectile-launches)
- explosion by Michel Baradari at [Open Game
  Art](https://opengameart.org/content/2-high-quality-explosions)
- jumping, landing sound by [leohpaz](https://opengameart.org/users/leohpaz) at
  [Open Game Art](https://opengameart.org/content/12-player-movement-sfx)
- dying sound by [HaelDB](https://opengameart.org/users/haeldb) at [Open Game
  Art](https://opengameart.org/content/male-gruntyelling-sounds)


(AI has not been used in any part of the development process)


### tutorials

In the beginning of the project I followed and got mileage out of the following
tutorials:

- The excellent [in-depth Youtube
  playlist](https://www.youtube.com/watch?v=57ibhDU2SAI&list=PLHQ0utQyFw5KCcj1ljIhExH_lvGwfn6GV)
  by Retro Game Mechanics Explained. Just a really good video manual-like
  series. A lot of the written content refers to this Youtube series, as it
  doesn't make sense to rehash this content badly.
  
- The excellent [SNES Assembly Adventure
  series](https://georgjz.github.io/snesaa01/) by jeorgjz. Pretty
  lenghy. Provides you with quite a decent setup.

- Nesdoug's [SNES Programming Guide](https://nesdoug.com/2020/03/19/snes-projects/)

- Wesley Aptekar-Cassels [SNES
  Development](https://blog.wesleyac.com/posts/snes-dev-1-getting-started)
  tutorial.

I've used code from all of these for SNES initialization routines, some library
functions.




## tips

- The project has a sensible CMake setup. Georgjz has a great
  [setup](https://georgjz.github.io/snesaa10/) that was almost exactly what I
  wanted/needed.

- For VSCode plugin, I switched mid-development to [CA65 Assembly Language
  Server](https://github.com/techwritescode/ca65-lsp), which is in active
  development, and is great. Especially the integrated assembly instruction help
  is great, but it also has sophisticated jump to definition and other goodies.

- For VSCode I was also using a locally modified version of the [ASM Code
  Lens](https://github.com/maziac/asm-code-lens) plugin, which I modified
  locally to add syntax highlighting for 65816 and 6502. ASM Code Lens was nice
  because it would give in-line definitions of IO registers. I copied the whole
  of the register info pages from the [SNESdev
  Wiki](https://snes.nesdev.org/wiki/SNESdev_Wiki), and made them into assembly
  files with comments, so ASM Code Lens can pop up the definitions
  in-line. Works great!

- The SNES Development Server Discord channel is a great place to hang out and
  ask questions.  A number of the people that make often used libraries and
  tools I use hang out there as well, and you can get a lot of feedback there.

- [Mesen2](https://github.com/SourMesen/Mesen2) is by far the best dev SNES
  emulator at the moment for debugging. Takes debug files from the assembler,
  which means it will display your source files with comments and all and you
  can put breakpoints straight in those. Plus the maker, Sour, helped me out
  with an issue on Discord. You can't beat that.

- For audio, [Terrific Audio
  Driver](https://github.com/undisbeliever/terrific-audio-driver) is probably
  the best option.  It's pretty featureful, is very actively worked on ATM,
  integrates easy with a bunch of assemblers, and again the developer hangs out
  on Discord, and is helpful. Music notation format is MML, but someone also
  made an ad-hoc quick stab Furnace front-end for it.

- Assembler: [cc65](https://cc65.github.io), or rather the assembler,
  ca65. Seems to be what most devs outside of romhacking use. Integrates
  especially well with Mesen and also Terrific Audio Driver.

- For graphic file conversions I'm currently using
  [SuperFamiconv](https://github.com/Optiroc/SuperFamiconv).  Commandline tool
  that I integrated in CMake.

- For level maps, I use [Tiled](https://www.mapeditor.org). I take the JSON
  output, and massage it with a script into SNES binary maps.

### Useful documentation:
  - [SNESdev Wiki](https://snes.nesdev.org/wiki/SNESdev_Wiki)
  - [SFC Development Wiki](https://wiki.superfamicom.org)
  - [nocash SNES specs](https://problemkaputt.de/fullsnes.htm)
  - [opcode summaries](https://undisbeliever.net/snesdev/65816-opcodes.html)
  - [Programming the 65816](https://www.amazon.com/Programming-65816-Including-65C02-65802/dp/0893037893)
  - [NesDev SNES forum](https://forums.nesdev.org/viewforum.php?f=12)
  - [stack article](http://6502org.wikidot.com/software-65816-parameters-on-stack) on 6502.org wiki
  - [65816S programmer manual](https://www.westerndesigncenter.com/wdc/documentation/w65c816s.pdf)

## License

See the [LICENSE file](LICENSE.md)
