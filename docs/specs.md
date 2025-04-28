# map and tile encoding

We want users to be able to make maps themselves, so we're in need of some kind of API. And it's also useful for my own sanity. I guess most if this is for my own sanity (albeit it's a little to late for that).

Currently we've got a 256x256 pixel (so 32x32 tile (perhaps should be 32x28)) png with all tiles that are potentially used for maps. Every tile in a particular row has the same properties. This way you can easily map visual variations of the same tile type.

And by properties we mean "how does the tile behave in the gameworld". Things like 'can we pass through this tile' or 'do bullets bounce of it'.

With these tiles we construct maps. Our maps are the size of an NTSC screen, so 30x28 tiles. And the way we encode all the data, is by constructing two maps for our background. One map is the map that will go in to video ram. The entries in this map by their nature already encode which tile to use and x/y flip, so why reinvent the wheel.

The sibling map is the map that contains a bitmap of the before mentioned properties.

These properties are encoded in for now a byte. Every property occupies a bit in this byte, so for now a tiles can have 8 different properties. I think this is nice because you don't really want to deal with every tile type having an id, for which you need to check a lookup table. You can just check if a bit is  on for the purpose that you need it for.

We create a file with Tiled, so a `.tmj` map file is used as the map input, that together with the JSON file is used to produce the property map.

We use Superfamiconv to convert the png with tiles to a crunched set of tiles, and we know that Superfamiconv will deduplicate the tiles and that logically it will go from left to right and from up to down to put every new tile it finds behind the other, again from left to right and from up till down. We use this knowledge to index into this unique set when we process tiled as per below.

 
## tile schema JSON format

We use a JSON file to map rows in the png to properties, and we denote how many tile entries each row has, so we can do some validation of a Tiled tmj file.


### list layout

- every list below corresponds to a line in the tile png
- first list is row 0, second is row 1, etc..
- row items from left to right:
    - tile name
    - row entry count
    - A string chars that denote the property that the tile  
      has. So no properties means the tile does nothing. Just decorative.
    - Scripts will transform these denotations into actual binary data where
      each bit has a specific order in the property-map. Let's call it a p-map.


### bit legend

- w = wall
- b = bounce
- c = climb


### Property binary layout per entry:

7 6 5 4 3 2 1 0
          c b w
