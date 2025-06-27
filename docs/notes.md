game bank layout:

RODATA: generated bits and bobs
BANK1: sound
BANK2: collision/game tile maps


On cycle budget:

tepples on https://forums.nesdev.org/viewtopic.php?p=215656#p215656

On Super NES, a scanline (minus DRAM refresh time) is 1324 master clocks.
A fast cycle is 6 master clocks. There are 220.7 fast cycles per line.
A slow cycle is 8 master clocks. There are 165.5 slow cycles per line.
DMA copies one byte per slow cycle.
On NTSC with overscan mode turned off, there are 262 - 224 = 38 scanlines in vblank. Subtract one scanline for prerender time, and you may end up with 165.5 * 37 = a smidge under 6 KiB per vblank.
This means you can replace one-third of the 16 KiB sprite tile area per vblank.

You can replace more if you force blanking in the top and bottom of the screen by changing the master rendering enable bit in $2100. Let's say you add 24 lines of letterbox on the top and bottom making the middle 176 pixels tall, which covers the entirety of a widescreen TV when zoomed in. In that case, you might be able to push (262-176-1)*165.5 = over 13.5 KiB.


also: Anomie timing document: https://wiki.superfamicom.org/timing


So non-vblank budget: 37,072 slow cycles, 49,437 fast ones
       vblank budget:  6,289 slow          8,387 fast
       ---------------------------------------------- total
                      43,361              57,823




sprite collisions:

- comment at https://www.reddit.com/r/retrogamedev/comments/y3jnw3/collision_detection_in_8_bit_games/

In general, sprite/tile collisions work something like this…

(This is for simple tile collisions where a tile is either solid or it isn’t (like Super Mario Bros style). If you need pixel-to-pixel accuracy then you’ll need to extend this once you have the tiles you want to test identified with some other method to get that kind of accuracy - shifting and ANDing sprite data against tile data or height tables or something)

You need a way to convert your sprite’s game world coordinates into coarse tile map coordinates and a fine pixel coordinate within a tile. That is usually as simple as dividing the sprite’s X & Y coordinates by the size of your tiles to get the coarse position and then remainder is the fine position. So, if you’ve got 8x8px tiles then shift sprite X/Y right 3 times to get the coarse X pos and AND sprite X/Y with 7 to get the fine position.

Before we can start checking tiles though, we’ve got a problem which is that math is telling us what tile is under a point at the sprite’s local origin which is usually at the top left of the sprite and we may not care what is under a point floating a few pixels to the left of its head, we need to know what is under a more useful spot(s). So, we need to add some offsets first and for more accuracy we’re probably going to want to check several spots depending on what action is happening. At the simplest level we could choose the horizontal center of the sprite and the check at the bottom for his feet and the top for his head but you probably really want to check a point in each quadrant for more accuracy and to not have half a sprite overlapping walls when it runs into them.

So, if your sprites are 16x16px and you’re walking right and want to test his right foot then you might add something like 14px to X and 16px to Y and the translate to tile coordinates and start testing tiles and you might want to check something like X+14, Y+1 to make sure there’s not a low hanging wall blocking progress. You can fine tune the offsets to match the shape/size of your sprite and to fine tune the feel (like, does one pixel of tippy toe count for catching a ledge or do you need to get more of the foot on there to count?)

for walking right you want to check the lower and upper right quadrants but you probably don’t care what’s happening on the left
walking left is the opposite, check lower and upper left and ignore the right
jumping straight up you’d want to check the upper left and right
jumping right you’d want to check the upper right and lower right (where you choose to place this offset will affect how close to barriers you can be to jump over them). On the way up you might check the upper left (this check and its offset placement affects how far under obstacles you can be and jump out from under them) but you don’t care about the lower left. On the way down you need to check the lower left though to see up if it lands on anything but now you don’t need to worry about the upper left.
jumping left is the reverse of above
when you’re walking you need to check if there is a tile underneath supporting you, otherwise you should start falling
if you’re falling you need to check if you’re landing on a solid tile
You also need some way of knowing if your sprite is going to cross the boundary into a new tile so you know it’s time to check for collisions. You can store its old tile position and compare it to the new one after you’ve done all your movement updates and if it is different, do your checks and then move it back if the new position is impassible. Or, you can test before updating positions and only allow the movement if it is clear.

For the latter you look at the fine X/Y positions and if adding DX/DY would cause fine X/Y pos to exceed 7 or be less 0 then you need to check those tiles at the new locations. If they are blocking then don’t do the requested movement or bounce off of it or whatever the sprite should do when it hits that type of tile. If all the tiles in that direction are clear then you can go ahead and update positions and move the sprite. If the fine position is still between 0-7 after applying DX/DY then you don’t need to worry about collision detection.

- 