.ifndef IO_INC
IO_INC = 1


; Register types:
; R - Readable
; W - Writeable
; 8 - 8-bit access only
; 16 - 8-bit access to either address, or 16-bit access to the lower address.
; 24 - 8-bit or 16-bit access to 3 registers.
; 8x2 - An internal 2-byte state accessed by two 8-bit read or writes (LSB first).


; PPU registers
; -------------

; Display configuration
; ---------------------

; INIDISP - Screen display ($2100 write)
;
; 7  bit  0
; ---- ----
; F... BBBB
; |    ||||
; |    ++++- Screen brightness (linear steps from 0 = none to $F = full)
; +--------- Force blanking
INIDISP = $2100

; BGMODE - BG mode and Character size ($2105 write)
;
; 7  bit  0
; ---- ----
; 4321 PMMM
; |||| ||||
; |||| |+++- BG mode (see below)
; |||| +---- Mode 1 BG3 priority (0 = normal, 1 = high)
; |||+------ BG1 character size (0 = 8x8¹, 1 = 16x16)
; ||+------- BG2 character size (0 = 8x8¹, 1 = 16x16)
; |+-------- BG3 character size (0 = 8x8¹, 1 = 16x16)
; +--------- BG4 character size (0 = 8x8, 1 = 16x16)
;                                                       BG Modes
; Mode| BG bit depth  |Offsets |     Priorities (front -> back)       |                     Notes
;     |BG1 BG2 BG3 BG4|per tile|                                      |
;  0  | 2   2   2   2 |   No   |   S3 1H 2H S2 1L 2L S1 3H 4H S0 3L 4L|
;  1  | 4   4   2     |   No   |   S3 1H 2H S2 1L 2L S1 3H    S0 3L   |BG3 priority = 0
;     |               |        |3H S3 1H 2H S2 1L 2L S1       S0 3L   |BG3 priority = 1
;  2  | 4   4         |  Yes   |   S3 1H    S2 2H    S1 1L    S0 2L   |
;  3  | 8   4         |   No   |   S3 1H    S2 2H    S1 1L    S0 2L   |
;  4  | 8   2         |  Yes   |   S3 1H    S2 2H    S1 1L    S0 2L   |
;  5  | 4   2         |   No   |   S3 1H    S2 2H    S1 1L    S0 2L   |Fixed 16 pixel char width. Forced high-res mode.
;  6  | 4             |  Yes   |   S3 1H    S2       S1 1L    S0      |Fixed 16 pixel char width. Forced high-res mode.
;  7  | 8             |   No   |   S3       S2       S1 1L    S0      |Fixed 8x8 char size.
; 7EXT| 8   7         |   No   |   S3       S2 2H    S1 1L    S0 2L   |Fixed 8x8 char size. BG2 bit 7 acts as priority.
; ¹: In modes 5 and 6, characters and OPT entries are always 16 pixels wide.
BGMODE = $2105

; MOSAIC - Screen pixelation ($2106 write)
;
; 7  bit  0
; ---- ----
; SSSS 4321
; |||| ||||
; |||| |||+- Enable BG1 mosaic
; |||| ||+-- Enable BG2 mosaic
; |||| |+--- Enable BG3 mosaic
; |||| +---- Enable BG4 mosaic
; ++++------ Mosaic size in pixels (0 = 1x1, ..., 15 = 16x16)
MOSAIC = $2106

; BGnSC - BG1-4 tilemap address and size ($2107-$210A write)
;
; 7  bit  0
; ---- ----
; AAAA AAYX
; |||| ||||
; |||| |||+- Horizontal tilemap count (0 = 1 tilemap, 1 = 2 tilemaps)
; |||| ||+-- Vertical tilemap count (0 = 1 tilemap, 1 = 2 tilemaps)
; ++++-++--- Tilemap VRAM address (word address = AAAAAA << 10)
;
; Tilemaps may be placed at any 2 KiB (1 KiW) page.
BG1SC = $2107
BG2SC = $2108
BG3SC = $2109
BG4SC = $210A

; CHR word base address
; ---------------------
;
; The tile base address for background CHR can start at any 8 KiB (4 KiW) page.
;
; Tilemap offsets that go past the end of VRAM are allowed to wrap around to the
; beginning.


; BG12NBA - BG1 and BG2 CHR word base address ($210B write)
;
; 7  bit  0
; ---- ----
; BBBB AAAA
; |||| ||||
; |||| ++++- BG1 CHR word base address (word address = AAAA << 12)
; ++++------ BG2 CHR word base address (word address = BBBB << 12)
BG12NBA = $210B

; BG34NBA - BG3 and BG4 CHR word base address ($210C write)
;
; 7  bit  0
; ---- ----
; DDDD CCCC
; |||| ||||
; |||| ++++- BG3 CHR word base address (word address = CCCC << 12)
; ++++------ BG4 CHR word base address (word address = DDDD << 12)
BG34NBA = $210C

; Scroll
; ------
;
; Each of these scroll registers is normally updated by two single-byte writes to
; the same address. After two consecutive writes the scroll value is fully
; updated.  The two-write mechanism internally keeps shared latch values, so these
; registers should not normally be written in mixed order. Complete both writes to
; one register before moving on to the next.
;
; The scroll offset is always relative to the top-left of the screen, even when
; updating mid-frame with HDMA.
;
; Because the first line of rendering is always a blank line, with vertical scroll
; of 0 the top line of the BG will be hidden. In the default 224-lines mode an
; extra (224th) line of BG is also visible at the bottom to compensate.


; BGnHOFS - BG1-4 horizontal scroll offset ($210D, $210F, $2111, $2113 write twice)
;
; 15  bit  8   7  bit  0
;  ---- ----   ---- ----
;  .... ..XX   XXXX XXXX
;         ||   |||| ||||
;         ++---++++-++++- Bgn horizontal scroll
;
; On write: BGnHOFS = (value << 8) | (bgofs_latch & ~7) | (bghofs_latch & 7)
;           bgofs_latch = value
;           bghofs_latch = value
;
; Note: BG1HOFS uses the same address as M7HOFS
BG1HOFS = $210D
BG2HOFS = $210F
BG3HOFS = $2111
BG4HOFS = $2113

; BGnVOFS - BG1-4 vertical scroll offset ($210E, $2110, $2112, $2114 write twice)
;
; 15  bit  8   7  bit  0
;  ---- ----   ---- ----
;  .... ..YY   YYYY YYYY
;         ||   |||| ||||
;         ++---++++-++++- BGn vertical scroll
;
; On write: BGnVOFS = (value << 8) | bgofs_latch
;           bgofs_latch = value
;
; Note: BG1VOFS uses the same address as M7VOFS
BG1VOFS = $210E
BG2VOFS = $2110
BG3VOFS = $2112
BG4VOFS = $2114

; Layer enable
; ------------

; TM - Main screen layer enable ($212C write)
;
; 7  bit  0
; ---- ----
; ...O 4321
;    | ||||
;    | |||+- Enable BG1 on main screen
;    | ||+-- Enable BG2 on main screen
;    | |+--- Enable BG3 on main screen
;    | +---- Enable BG4 on main screen
;    +------ Enable OBJ on main screen
TM = $212C

; TS - Subscreen layer enable ($212D write)
;
; 7  bit  0
; ---- ----
; ...O 4321
;    | ||||
;    | |||+- Enable BG1 on subscreen
;    | ||+-- Enable BG2 on subscreen
;    | |+--- Enable BG3 on subscreen
;    | +---- Enable BG4 on subscreen
;    +------ Enable OBJ on subscreen
TS = $212D

; SETINI - Screen Mode/Video Select ($2133 write)
;
; 7  bit  0
; ---- ----
; EX.. HOiI
; ||   ||||
; ||   |||+- Screen interlacing
; ||   ||+-- OBJ interlacing
; ||   |+--- Overscan mode
; ||   +---- High-res mode
; |+-------- EXTBG mode
; +--------- External sync
;
; - Screen interlacing causes every odd frame to lower its picture scanlines half
;   a line between the even frames. When enabled, this produces a 480i picture
;   composed of 2 frames (fields), instead of the default 240p progressive picture
;   where each frame appears at the same vertical level.
;
;   - STAT78 ($213F) can be used to check whether the current frame is an even or
;     odd field.
;
;   - When interlacing is enabled for BG mode 5 or 6, the BG layers are
;     automatically interlaced to give a view of the background that has double
;     the vertical resolution in 480i, effectively making every BG pixel half as
;     tall.
;
;     - The BGMODE character size bits still choose between 16x8 and 16x16px tiles
;       even when interlacing is true.
;
; - OBJ interlacing interlaces the sprites to double their vertical resolution in
;   480i. Sprite pixels will appear half as tall.
;
; - Overscan mode enables the full 239 line picture when set, instead of only
;   224. On NTSC televisions this extra area is not normally visible, but on PAL
;   it is very visible. Setting this causes NMI/vblank to begin 8 lines later, and
;   end 8 lines earlier, dramatically reducing the vblank length in NTSC. Sprite
;   and scroll positions are relative to the end of the blanking period, so
;   enabling this automatically shifts everything up 8 lines. Using this feature
;   makes the SNES drawing positions similar to the NES.
;
; - High-res mode doubles the horizontal output resolution from 256 to 512 pixels.
;
;   - In most BG modes this causes the sub screen to render pixels on even columns
;     (assuming zero-based column indices), and the main screen to render on odd
;     columns. This is sometimes called "pseudo-hires". Some games use this for a
;     transparency effect (Kirby's Dreamland 3, Jurassic Park), relying on
;     blurring from the composite video signal to blend the columns.
;
;   -In BG modes 5 and 6, this high-res is forced, but the BG layers are
;    automatically interleaved to double their horizontal resolution, making every
;    BG pixel half as wide.
;
; - EXTBG controls a second-layer effect in BG mode 7 only. In other modes,
;   enabling EXTBG will display garbage.
;
; - External sync is used for super-imposing images from an external
;   device. Normally 0.
SETINI = $2133

; VRAM
; ----

; VMAIN - Video Port Control ($2115 write)
;
; 7  bit  0
; ---- ----
; M... RRII
; |    ||||
; |    ||++- Address increment amount:
; |    ||     0: Increment by 1 word
; |    ||     1: Increment by 32 words
; |    ||     2: Increment by 128 words
; |    ||     3: Increment by 128 words
; |    ++--- Address remapping: (VMADD -> Internal)
; |           0: None
; |           1: Remap rrrrrrrr YYYccccc -> rrrrrrrr cccccYYY (2bpp)
; |           2: Remap rrrrrrrY YYcccccP -> rrrrrrrc ccccPYYY (4bpp)
; |           3: Remap rrrrrrYY YcccccPP -> rrrrrrcc cccPPYYY (8bpp)
; +--------- Address increment mode:
;             0: Increment after writing $2118 or reading $2139
;             1: Increment after writing $2119 or reading $213A
;
; - Address remapping allows redirection of the write address to update 32-tile
;   rows horizontally when using II = 0. Within a 32-tile group, sequential access
;   iterates through the same 8-pixel row of each tile horizontally. After 32
;   spans, it will reach the second row of the first tile. Finally after a group
;   of 32 tiles has been updated, it advances to the next group of 32 tiles..
;
;   - This is suitable for a 32x32 tilemap in 8x8 tile mode. By filling each row
;     of the tilemap with sequential values, each group of 32 tiles now
;     corresponds to a contiguous horizontal span of pixels.
;
;   - P = tile bitplane-word, c = group column, Y = tile pixel row, r = group row.
;
;   - When setting the starting address, the starting tile of a 32-tile group will
;     always be the at the same position as its remapped address.
;
;   - With 4bpp or 8bpp modes, each increment advances through the 2 or 4
;     plane-words of a single tile before advancing to the next tile.
;
;   - Simplified explanation:
;
;     1. Write all planes for an 8 pixel span before
;        proceeding horizontally to the next.
;
;     2. After completing a row of 256 pixels (32 spans), proceed vertically to
;        the next.
VMAIN = $2115

; VRAM address
; ------------

; VMADDL, VMADDH - VRAM word address ($2116, $2117 write)
;
;   VMADDH      VMADDL
;   $2117       $2116
; 7  bit  0   7  bit  0
; ---- ----   ---- ----
; hHHH HHHH   LLLL LLLL
; |||| ||||   |||| ||||
; ++++-++++---++++-++++- VRAM word address
;
; On write: Update VMADD
;           vram_latch = [VMADD]
;
; Because the SNES only has 64 KiB of VRAM, VRAM address bit 15 has no effect.
;
; The VRAM can only be read during vertical-blank or force-blank. If the PPU is in
; horizontal-blank or active-display then the VRAM will not be read and vram_latch
; will contain invalid data.
VMADDL = $2116
VMADDH = $2117

; VRAM data
; ---------

; VMDATAL, VMDATAH - VRAM data write ($2118, $2119 write)
;
;   VMDATAH     VMDATAL
;   $2119       $2118
; 7  bit  0   7  bit  0
; ---- ----   ---- ----
; HHHH HHHH   LLLL LLLL
; |||| ||||   |||| ||||
; ++++-++++---++++-++++- VRAM data word
;
; On $2118 write: If address increment mode == 0: increment VMADD
; On $2119 write: If address increment mode == 1: increment VMADD
;
; The VRAM can only be written to in vertical-blank or force-blank. Any VRAM
; writes during horizontal-blank or active-display will be ignored.  VMADD will
; always increment, depending on the state of VMAIN, even if the VRAM write is
; ignored.
VMDATAL = $2118
VMDATAH = $2119

; VMDATALREAD, VMDATAHREAD - VRAM data read ($2139, $213A read)
;
;    VMDATAHREAD VMDATALREAD
;    $213A       $2139
;  7  bit  0   7  bit  0
;  ---- ----   ---- ----
;  HHHH HHHH   LLLL LLLL
;  |||| ||||   |||| ||||
;  ++++-++++---++++-++++- VRAM data word from vram_latch
;
; On $2139 read: value = vram_latch.low
;                If address increment mode == 0:
;                  vram_latch = [VMADD]
;                  Increment VMADD
;
; On $213A read: value = vram_latch.high
;                If address increment mode == 1:
;                  vram_latch = [VMADD]
;                  Increment VMADD
;
; When reading multiple bytes/words with increment, we normally have to do 1 extra
; read at the start to account for the vram_latch behaviour.
;
; The vram_latch is loaded immediately after you set an address with VMADD, and
; the word value at that address will be available for the next reads from
; VMDATAxREAD.
;
; When incrementing due to VMDATAxREAD, the next word value is loaded into
; vram_latch before the increment. This means that the first 2 reads after setting
; VMADD will both return the same word stored at that address, before the
; increment takes effect and allows you to read the subsequent bytes/words.
;
; So:
;
;   - When reading a single byte/word of data: simply set the address with VMADD,
;     and then read the data via VMDATAxREAD.
;
;   - When reading a block of contiguous data: after writing VMADD do one dummy
;     read to VMDATAxREAD to pre-load the vram_latch. After this you can simply
;     reach each byte/word sequentially with auto-increment.
;
; The VRAM can only be read during vertical-blank or force-blank. If the PPU is in
; horizontal-blank or active-display then the VRAM will not be read and vram_latch
; will contain invalid data.
;
; VMADD will always increment, depending on the state of VMAIN, even if the VRAM
; is not read.
VMDATALREAD = $2139
VMDATAHREAD = $213A

; CGRAM
; -----

; CGADD - CGRAM word address ($2121 write)
;
; 7  bit  0
; ---- ----
; AAAA AAAA
; |||| ||||
; ++++-++++- CGRAM word address
;
; On write: cgram_byte = 0
CGADD = $2121

; CGRAM data
; ----------

; CGDATA - CGRAM data write ($2122 write twice)
;
; 15  bit  8   7  bit  0
;  ---- ----   ---- ----
;  .BBB BBGG   GGGR RRRR
;   ||| ||||   |||| ||||
;   ||| ||||   |||+-++++- Red component
;   ||| ||++---+++------- Green component
;   +++-++--------------- Blue component
;
; On write: If cgram_byte == 0: cgram_latch = value
;           If cgram_byte == 1: CGDATA = (value << 8) | cgram_latch
;           cgram_byte = ~cgram_byte
;
; Two single-byte writes to this register will update a single CGRAM word. The
; effect is applied only once the second byte is written.  Each write will
; increment the internal byte address. After two writes it will automatically have
; incremented to the next word.
CGDATA = $2122

; CGDATAREAD - CGRAM data read ($213B read twice)
;
; 15  bit  8   7  bit  0
;  ---- ----   ---- ----
;  xBBB BBGG   GGGR RRRR
;  |||| ||||   |||| ||||
;  |||| ||||   |||+-++++- Red component
;  |||| ||++---+++------- Green component
;  |+++-++--------------- Blue component
;  +--------------------- PPU2 open bus
;
; On read: If cgram_byte == 0: value = CGDATA.low
;          If cgram_byte == 1: value = CGDATA.high
;          cgram_byte = ~cgram_byte
CGDATAREAD = $213B

; OAM
; ---

; OBJSEL - Object size and Character address ($2101 write)
;
; 7  bit  0
; ---- ----
; SSSN NbBB
; |||| ||||
; |||| |+++- Name base address (word address = bBB << 13)
; |||+-+---- Name select (word offset = (NN+1) << 12)
; +++------- Object size:
;             0:  8x8  and 16x16
;             1:  8x8  and 32x32
;             2:  8x8  and 64x64
;             3: 16x16 and 32x32
;             4: 16x16 and 64x64
;             5: 32x32 and 64x64
;             6: 16x32 and 32x64
;             7: 16x32 and 32x32
;
; - Name base address selects a 16 KiB-aligned quarter of VRAM for the first 8 KiB
;   of available sprite tiles. Bit 2 was reserved for a planned but never
;   implemented expansion to 128 KiB VRAM, so is normally 0.
;
; - Name select controls a relative offset from the name base address in NN+1 8
;   KiB increments, selecting a second 8 KiB of available sprite tiles. With name
;   select of 0, the second half follows the base 8 KiB contiguously.
;
; - Object size controls the sizes available for sprites. The two modes featuring
;   rectangular sizes (6, 7) were not documented by the SNES development manual.
;
; Fullsnes refers to this register as OBSEL.
OBJSEL = $2101

; OAM address
; -----------

; OAMADDL, OAMADDH - OAM word address ($2102, $2103 write)
;
;   OAMADDH     OAMADDL
;   $2103       $2102
; 7  bit  0   7  bit  0
; ---- ----   ---- ----
; P... ...B   AAAA AAAA
; |       |   |||| ||||
; |       |   ++++-++++- OAM word address
; |       |   ++++-+++0- OAM priority rotation index
; |       +------------- OAM table select (0 = 256 word table, 1 = 16 word table)
; +--------------------- OAM priority rotation (1 = enable)
;
; On write: Update OAMADD
;           internal_oamadd = (OAMADD & $1FF) << 1
;
; Priority rotation causes the highest priority sprite to be at the last OAMADD
; set before the visible picture (bits 1-7 only). Otherwise OAM 0 is the highest
; priority sprite. This can be used for a simple sprite priority rotation.
OAMADDL = $2102
OAMADDH = $2103

; OAM data
; --------

; OAMDATA - OAM data write ($2104 write)
;
; 7  bit  0
; ---- ----
; DDDD DDDD
; |||| ||||
; ++++-++++- OAM data
;
; On write: If (internal_oamadd & 1) == 0: oam_latch = value
;           If internal_oamadd < $200 and (internal_oamadd & 1) == 1:
;             [internal_oamadd-1] = oam_latch
;             [internal_oamadd] = value
;           If internal_oamadd >= $200: [internal_oamadd] = value
;           internal_oamadd = internal_oamadd + 1
;
; When the OAM byte address is less than 512:
;   Two single-byte writes to this register will update a single OAM word. The
;   effect is applied only once the second byte is written.
;
; When the OAM byte address is 512 or above:
;   Each write immediately applies to the current byte.
;
; Each write will increment the internal byte address.
OAMDATA = $2104

; OAMDATAREAD - OAM data read ($2138 read)
;
; 7  bit  0
; ---- ----
; DDDD DDDD
; |||| ||||
; ++++-++++- OAM data
;
; On read: value = [internal_oamadd]
;          internal_oamadd = internal_oamadd + 1
OAMDATAREAD = $2138

; Mode 7
; ------

; M7SEL - Mode 7 settings ($211A write)
;
; 7  bit  0
; ---- ----
; RF.. ..YX
; ||     ||
; ||     |+- Flip screen horizontally (backgrounds only)
; ||     +-- Flip screen vertically (backgrounds only)
; |+-------- Non-tilemap fill (0 = transparent, 1 = character 0)
; +--------- Tilemap repeat (0 = tilemap repeats, 1 = Non-tilemap fill beyond tilemap boundaries)
M7SEL = $211A

; Scroll
; ------

; M7HOFS - Mode 7 horizontal scroll offset ($210D write twice)
;
; 15  bit  8   7  bit  0
;  ---- ----   ---- ----
;  ...X XXXX   XXXX XXXX
;     | ||||   |||| ||||
;     +-++++---++++-++++- Mode 7 horizontal scroll (signed)
;
; On write: M7HOFS = (value << 8) | mode7_latch
;           mode7_latch = value
;
; Note: This register uses the same address as BG1HOFS
M7HOFS = $210D

; M7VOFS - Mode 7 vertical scroll offset ($210E write twice)
;
; 15  bit  8   7  bit  0
;  ---- ----   ---- ----
;  ...Y YYYY   YYYY YYYY
;     | ||||   |||| ||||
;     +-++++---++++-++++- Mode 7 vertical scroll (signed)
;
; On write: M7VOFS = (value << 8) | mode7_latch
;           mode7_latch = value
;
; Note: This register uses the same address as BG1VOFS
M7VOFS = $210E

; Matrices
; --------

; M7A - Mode 7 matrix A and Multiplication factor 1 ($211B write twice)
;
; 15  bit  8   7  bit  0
;  ---- ----   ---- ----
;  DDDD DDDD   dddd dddd
;  |||| ||||   |||| ||||
;  ++++-++++---++++-++++- Mode 7 matrix A (8.8 fixed point)
;  ++++-++++---++++-++++- 16-bit multiplication factor (signed)
;
; On write: M7A = (value << 8) | mode7_latch
;           mode7_latch = value
;
; The last 16-bit value (signed) written here is also used to provide a 24-bit
; multiplication result at MPY.
M7A = $211B

; M7B - Mode 7 matrix B and Multiplication factor 2 ($211C write twice)
;
; 15  bit  8   7  bit  0
;  ---- ----   ---- ----
;  DDDD DDDD   dddd dddd
;  |||| ||||   |||| ||||
;  ++++-++++---++++-++++- Mode 7 matrix B (8.8 fixed point)
;              ++++-++++- 8-bit multiplication factor (signed)
;
; On write: M7B = (value << 8) | mode7_latch
;           mode7_latch = value
;
; The last 8-bit value (signed) written here is also used to provide a 24-bit
; multiplication result at MPY.
M7B = $211C

; M7n - Mode 7 matrix C-D ($211D-211E write twice)
;
; 15  bit  8   7  bit  0
;  ---- ----   ---- ----
;  DDDD DDDD   dddd dddd
;  |||| ||||   |||| ||||
;  ++++-++++---++++-++++- Mode 7 matrix n (8.8 fixed point)
;
; On write: M7n = (value << 8) | mode7_latch
;           mode7_latch = value
M7C = $211D
M7D = $211E

; Center
; ------
;
; M7X - Mode 7 center X ($211F write twice)
;
; 15  bit  8   7  bit  0
;  ---- ----   ---- ----
;  ...X XXXX   XXXX XXXX
;     | ||||   |||| ||||
;     +-++++---++++-++++- Mode 7 center X (signed)
;
; On write: M7X = (value << 8) | mode7_latch
;           mode7_latch = value
M7X = $211F

; M7Y - Mode 7 center Y ($2120 write twice)
;
; 15  bit  8   7  bit  0
;  ---- ----   ---- ----
;  ...Y YYYY   YYYY YYYY
;     | ||||   |||| ||||
;     +-++++---++++-++++- Mode 7 center Y (signed)
;
; On write: M7Y = (value << 8) | mode7_latch
;           mode7_latch = value
M7Y = $2120

; Windows
; -------

; Window mask settings
; --------------------

; W12SEL - Window Mask Settings for BG1 and BG2 ($2123 write)
;
; 7  bit  0
; ---- ----
; DdCc BbAa
; |||| ||||
; |||| |||+- Invert window 1 for BG1
; |||| ||+-- Enable window 1 for BG1
; |||| |+--- Invert window 2 for BG1
; |||| +---- Enable window 2 for BG1
; |||+------ Invert window 1 for BG2
; ||+------- Enable window 1 for BG2
; |+-------- Invert window 2 for BG2
; +--------- Enable window 2 for BG2
W12SEL = $2123

; W34SEL - Window Mask Settings for BG3 and BG4 ($2124 write)
;
; 7  bit  0
; ---- ----
; HhGg FfEe
; |||| ||||
; |||| |||+- Invert window 1 for BG3
; |||| ||+-- Enable window 1 for BG3
; |||| |+--- Invert window 2 for BG3
; |||| +---- Enable window 2 for BG3
; |||+------ Invert window 1 for BG4
; ||+------- Enable window 1 for BG4
; |+-------- Invert window 2 for BG4
; +--------- Enable window 2 for BG4
W34SEL = $2124

; WOBJSEL - Window Mask Settings for OBJ and Color Window ($2125 write)
;
; 7  bit  0
; ---- ----
; LlKk JjIi
; |||| ||||
; |||| |||+- Invert window 1 for OBJ
; |||| ||+-- Enable window 1 for OBJ
; |||| |+--- Invert window 2 for OBJ
; |||| +---- Enable window 2 for OBJ
; |||+------ Invert window 1 for color
; ||+------- Enable window 1 for color
; |+-------- Invert window 2 for color
; +--------- Enable window 2 for color
;
; The color window is used to black areas of the main or sub screen, see: CGWSEL.
; Window positions
WOBJSEL = $2125

; WH0 - Window 1 left position ($2126 write)
;
; 7  bit  0
; ---- ----
; LLLL LLLL
; |||| ||||
; ++++-++++- Window 1 left edge position
WH0 = $2126

; WH1 - Window 1 right position ($2127 write)
;
; 7  bit  0
; ---- ----
; RRRR RRRR
; |||| ||||
; ++++-++++- Window 1 right edge position
WH1 = $2127

; WH2 - Window 2 left position ($2128 write)
;
; 7  bit  0
; ---- ----
; LLLL LLLL
; |||| ||||
; ++++-++++- Window 2 left edge position
WH2 = $2128

; WH3 - Window 2 right position ($2129 write)
;
; 7  bit  0
; ---- ----
; RRRR RRRR
; |||| ||||
; ++++-++++- Window 2 left edge position
WH3 = $2129

; Window mask logic
; -----------------

; WBGLOG - Window BG mask logic ($212A write)
;
; 7  bit  0
; ---- ----
; 4433 2211
; |||| ||||
; |||| ||++- BG1 window mask logic
; |||| ++--- BG2 window mask logic
; ||++------ BG3 window mask logic
; ++-------- BG4 window mask logic
WBGLOG = $212A

; WOBJLOG - Window OBJ and color math mask logic ($212B write)
;
; 7  bit  0
; ---- ----
; .... CCOO
;      ||||
;      ||++- OBJ window mask logic
;      ++--- Color window mask logic
;
; Mask logic types
; Value|Logic
;    0 | OR
;    1 | AND
;    2 | XOR
;    3 | XNOR
;
; The color window is used to mask regions of the main and sub-screens, see:
; CGWSEL.
WOBJLOG = $212B

; Window enable
; -------------

; TMW - Main screen layer window enable ($212E write)
;
; 7  bit  0
; ---- ----
; ...O 4321
;    | ||||
;    | |||+- Apply enabled windows to main screen BG1
;    | ||+-- Apply enabled windows to main screen BG2
;    | |+--- Apply enabled windows to main screen BG3
;    | +---- Apply enabled windows to main screen BG4
;    +------ Apply enabled windows to main screen OBJ
TMW = $212E

; TSW - Subscreen layer window enable ($212F write)
;
; 7  bit  0
; ---- ----
; ...O 4321
;    | ||||
;    | |||+- Apply enabled windows to subscreen BG1
;    | ||+-- Apply enabled windows to subscreen BG2
;    | |+--- Apply enabled windows to subscreen BG3
;    | +---- Apply enabled windows to subscreen BG4
;    +------ Apply enabled windows to subscreen OBJ
TSW = $212F

; Color math
; ----------

; CGWSEL - Color addition select ($2130 write)
;
; 7  bit  0
; ---- ----
; MMSS ..AD
; ||||   ||
; ||||   |+- Direct color mode
; ||||   +-- Addend (0 = fixed color, 1 = subscreen)
; ||++------ Sub screen color window transparent region
; ++-------- Main screen color window black region
;
; Region types
; Value|Region
;    0 |Nowhere
;    1 |Outside color window
;    2 |Inside color window
;    3 |Everywhere
;
; - The window region settings will replace the main-screen color with black, or
;   sub-screen with transparent, on pixels according to the color windows (WOBJSEL
;   high nibble). If the color windows are not enabled by WOBJSEL, everything is
;   "outside" them. The main-screen setting is used to force a region of the main
;   screen to black. The sub-screen setting is for masking color math.
;
; - Addend selects either the fixed color (COLDATA) or sub-screen for color
;   math. Both can be masked by the window region.
;
; - Direct color mode is not directly related to color math, but for 8-bpp
;   background modes it selects between palettes and direct color.
;
; - Some older emulators have known inaccurate implementations of the MM bits:
;
;   - Snes9x 1.43 ignores color math for the entire line if either bit is 1.
;
;   - ZSNES ignores color math for any pixels where the main screen was replaced
;     with black. This means that the final result for those pixels is always
;     black.
CGWSEL = $2130

; CGADSUB - Color math designation ($2131 write)
;
; 7  bit  0
; ---- ----
; MHBO 4321
; |||| ||||
; |||| |||+- BG1 color math enable
; |||| ||+-- BG2 color math enable
; |||| |+--- BG3 color math enable
; |||| +---- BG4 color math enable
; |||+------ OBJ color math enable (palettes 4-7 only)
; ||+------- Backdrop color math enable
; |+-------- Half color math
; +--------- Operator type (0 = add, 1 = subtract)
;
; This designates which elements of the main screen will have color math applied
; to them. After layering, if the visible pixel belongs to a color-math enabled
; layer, the chosen operation will be applied with the subscreen (or fixed color).
CGADSUB = $2131

; COLDATA - Fixed color data ($2132 write)
;
; 7  bit  0
; ---- ----
; BGRC CCCC
; |||| ||||
; |||+-++++- Color value
; ||+------- Write color value to blue channel
; |+-------- Write color value to green channel
; +--------- Write color value to red channel
;
; COLDATA requires one, two or three writes to set the fixed color to a target
; color value. For example:
; - Black - 1 write: %111_00000 (bgr=0)
; - White - 2 write: %111_11111 (bgr=31)
; - Dark Blue - 2 writes: %100_10010 (b=18), %011_00000 (gr=0)
; - Light Green - 2 writes: %101_10010 (br=20), %010_11111 (g=31)
; - Light Blue - 3 writes: %100_11110 (b=30), %010_11011 (g=27), %001_10110 (r=22)
; - Gold - 3 writes: %100_00000 (b=0), %010_11011 (g=27), %001_11111 (r=31)
COLDATA = $2132

; Multiplication result
; ---------------------

; MPYL, MPYM, MPYH - Multiplication result ($2134, $2135, $2136 read)
;
;   MPYH        MPYM        MPYL
;   $2136       $2135       $2134
; 7  bit  0   7  bit  0   7  bit  0
; ---- ----   ---- ----   ---- ----
; HHHH HHHH   MMMM MMMM   LLLL LLLL
; |||| ||||   |||| ||||   |||| ||||
; ++++-++++---++++-++++---++++-++++- 24-bit multiplication result (signed)
;
; This result may be read back after writing M7A with a signed 16-bit value (write
; twice), and M7B with a signed 8-bit value (write once). These two values will be
; multiplied to produce the signed 24-bit value read here.
MPYL = $2134
MPYM = $2135
MPYH = $2136

; H/V counters
; ------------

; SLHV - Software latch for H/V counters ($2137 read)
;
; 7  bit  0
; ---- ----
; xxxx xxxx
; |||| ||||
; ++++-++++- CPU Open bus
;
; On read: counter_latch = 1
SLHV = $2137

; Counters
; --------

; OPHCT - Output horizontal counter ($213C read twice)
;
; 15  bit  8   7  bit  0
;  ---- ----   ---- ----
;  xxxx xxxH   HHHH HHHH
;  |||| ||||   |||| ||||
;  |||| |||+---++++-++++- Horizontal counter value
;  ++++-+++-------------- PPU2 open bus
;
; On read: If ophct_byte == 0: value = OPHCT.low
;          If ophct_byte == 1: value = OPHCT.high
;          ophct_byte = ~ophct_byte
OPHCT = $213C

; OPVCT - Output vertical counter ($213D read twice)
;
; 15  bit  8   7  bit  0
;  ---- ----   ---- ----
;  xxxx xxxV   VVVV VVVV
;  |||| ||||   |||| ||||
;  |||| |||+---++++-++++- Vertical counter value
;  ++++-+++-------------- PPU2 open bus
;
; On read: If opvct_byte == 0: value = OPVCT.low
;          If opvct_byte == 1: value = OPVCT.high
;          opvct_byte = ~opvct_byte
;
; When counter_latch transitions from 0 to 1, these registers are latched with the
; current counter values. counter_latch is set when SLHV is read or /EXTLATCH
; (PPU2 pin 29) is asserted, and is cleared when STAT78 is read. /EXTLATCH is
; connected to joypad IO D7 and can be controlled by the CPU via WRIO or by a
; joypad.
;
; counter_latch behavior has not been fully confirmed.
OPVCT = $213D

; Status
; ------

; STAT77 - PPU1 status flags and version ($213E read)
;
; 7  bit  0
; ---- ----
; TRMx VVVV
; |||| ||||
; |||| ++++- PPU1 version
; |||+------ PPU1 open bus
; ||+------- Master/slave mode (PPU1 pin 25)
; |+-------- Range over flag (sprite tile overflow)
; +--------- Time over flag (sprite overflow)
STAT77 = $213E

; STAT78 - PPU2 status flags and version ($213F read)
;
; 7  bit  0
; ---- ----
; FLxM VVVV
; |||| ||||
; |||| ++++- PPU2 version
; |||+------ 0: 262 or 525i lines = 60Hz, 1: 312 or 625i lines = 50Hz (PPU2 pin 30)
; ||+------- PPU2 open bus
; |+-------- Counter latch value
; +--------- Interlace field
;
; On read: counter_latch = 0
;          ophct_byte = 0
;          opvct_byte = 0
;
; If a condition that sets counter_latch is active when STAT78 is read, it is not
; known if counter_latch is cleared. Existing documentation suggests it is not
; cleared and the counters are not relatched.
STAT78 = $213F

; MMIO registers
; --------------

; Interrupts
; ----------

; NMITIMEN - Interrupts and Joypad reading ($4200 write)
;
; 7  bit  0
; ---- ----
; N.VH ...J
; | ||    |
; | ||    +- Joypad auto-read enable
; | ++------ H/V timer IRQ:
; |           00 = Disable timer
; |           01 = IRQ when H counter == HTIME
; |           10 = IRQ when V counter == VTIME and H counter == 0
; |           11 = IRQ when V counter == VTIME and H counter == HTIME
; +--------- Vblank NMI enable
;
; On power-on: NMITIMEN = $00
; On reset:    NMITIMEN = $00
;
; - If the vblank NMI is enabled, then an NMI will occur at the start of
;   vblank. More precisely, an NMI occurs whenever the bitwise AND of vblank NMI
;   enable and RDNMI's vblank flag becomes 1. As a result, it is possible to get
;   multiple NMIs for a single vblank if vblank NMI is disabled and enabled again
;   while the vblank flag is still 1. This can be prevented by reading RDNMI,
;   which clears the vblank flag.
;
; - TODO: Details on the exact conditions for an IRQ.
;
; - Auto-read runs shortly after the beginning of vblank, reading the current
;   state of the two controller ports into the JOY1-4 registers without halting
;   the CPU. It is equivalent to writing 1 and then 0 to JOYOUT (used for latching
;   standard controllers) and then reading JOYSER0 and JOYSER1 16 times each. The
;   controllers are then left in this state so that additional reads can be done
;   manually from JOYSER0 and JOYSER1 to get any additional data, though this is
;   unnecessary for standard controllers. Auto-read takes approximately 3
;   scanlines, during which the JOYOUT, JOYSER0, and JOYSER1 registers should not
;   be manually accessed.
NMITIMEN = $4200

; Screen timer values
; -------------------

; HTIMEL, HTIMEH - H timer target ($4207, $4208 write)
;
;   HTIMEH      HTIMEL
;   $4208       $4207
; 7  bit  0   7  bit  0
; ---- ----   ---- ----
; .... ...H   LLLL LLLL
;         |   |||| ||||
;         +---++++-++++- H counter target for timer IRQ
;
; On power-on: HTIME = $1FF
;
; Note that setting a value larger than the maximum H counter value of 339 will
; prevent the timer's H condition from being met.
HTIMEL = $4207
HTIMEH = $4208

; VTIMEL, VTIMEH - V timer target ($4209, $420A write)
;
;   VTIMEH      VTIMEL
;   $420A       $4209
; 7  bit  0   7  bit  0
; ---- ----   ---- ----
; .... ...H   LLLL LLLL
;         |   |||| ||||
;         +---++++-++++- V counter target for timer IRQ
;
; On power-on: VTIME = $1FF
;
; Note that setting a value larger than the maximum V counter value will prevent
; the timer's V condition from being met. The maximum depends on the region (261
; for NTSC, 311 for PAL) and interlacing (1 additional scanline every other
; frame).
VTIMEL = $4209
VTIMEH = $420A

; Status
; ------

; RDNMI - Vblank flag and CPU version ($4210 read)
;
; 7  bit  0
; ---- ----
; Nxxx VVVV
; |||| ||||
; |||| ++++- CPU version
; |+++------ (Open bus)
; +--------- Vblank flag
;
; On power-on: RDNMI = RDNMI & $7F
; On reset:    RDNMI = RDNMI & $7F
; On read:     RDNMI = RDNMI & $7F
;
; The vblank flag is set at the start of vblank and cleared at the end of vblank
; or on read.
RDNMI = $4210

; TIMEUP - Timer flag ($4211 read)
;
; 7  bit  0
; ---- ----
; Txxx xxxx
; |||| ||||
; |+++-++++- (Open bus)
; +--------- Timer flag
;
; On power-on: TIMEUP = TIMEUP & $7F
; On reset:    TIMEUP = TIMEUP & $7F
; On read:     TIMEUP = TIMEUP & $7F
;
; The timer flag is set when the timer condition specified in NMITIMEN becomes
; true and is cleared on read.
TIMEUP = $4211

; HVBJOY - Screen and Joypad status ($4212 read)
;
; 7  bit  0
; ---- ----
; VHxx xxxJ
; |||| ||||
; |||| |||+- Joypad auto-read in-progress flag
; ||++-+++-- (Open bus)
; |+-------- Hblank flag
; +--------- Vblank flag
; J - Set during joypad auto-read.
; H - Set during horizontal blank period.
; V - Set during vertical blank period.
;
; When enabled via NMITIMEN, auto-read begins between H=32.5 and H=95.5 of the
; first vblank scanline, and ends 4224 master cycles later.[1]
HVBJOY = $4212

; APU
; ---

; APUIOn - Data-to-APU register n ($214n write) (n = 0..3)
;
; 7  bit  0
; ---- ----
; DDDD DDDD
; |||| ||||
; ++++-++++- Data to APU
;
; APUIOn - Data-from-APU register n ($214n read) (n = 0..3)
;
; 7  bit  0
; ---- ----
; DDDD DDDD
; |||| ||||
; ++++-++++- Data from APU
;
; When the SPC700 reads from $F4+n in its address space, it receives the last
; value written to APUIOn. When APUIOn is read, the value received is the last one
; written by the SPC700 to $F4+n. If APUIOn is read while its corresponding $F4+n
; register is being written, the value read will be the bitwise OR of the old and
; new values.
;
; These registers are mirrored across $2140-217F.
APUIO0 = $2140
APUIO1 = $2141
APUIO2 = $2142
APUIO3 = $2143

; WRAM
; ----

; WMDATA - S-WRAM data access ($2180 read/write)
;
; 7  bit  0
; ---- ----
; DDDD DDDD
; |||| ||||
; ++++-++++- S-WRAM data
;
; On write: [WMADD] = value
;           WMADD = WMADD + 1
;
; This register's presence on the peripheral bus allows DMA between S-WRAM and
; another, different source.
;
; Because DMA simultaneously accesses the source and destination, S-WRAM cannot
; succesfully be both because it cannot simultaneously read from and write to
; itself. DMA from S-WRAM to this register has no effect, and DMA from this
; register to S-WRAM writes open bus. In both cases, the address is not
; incremented.
WMDATA = $2180

; WMADDL, WMADDM, WMADDH - S-WRAM address ($2181, $2182, $2183 write)
;
;   WMADDH      WMADDM      WMADDL
;   $2183       $2182       $2181
; 7  bit  0   7  bit  0   7  bit  0
; ---- ----   ---- ----   ---- ----
; .... ...H   MMMM MMMM   LLLL LLLL
;         |   |||| ||||   |||| ||||
;         +---++++-++++---++++-++++- S-WRAM address for WMDATA
; DMA from S-WRAM to these registers has no effect.
WMADDL = $2181
WMADDM = $2182
WMADDH = $2183

; ROM
; ---

; MEMSEL - ROM access speed ($420D write)
;
;  7  bit  0
;  ---- ----
;  .... ...F
;          |
;          +- FastROM enable
;
; On power-on: MEMSEL = $00
;
; If enabled, ROM access to banks $80-FF takes only 6 system clock cycles instead
; of 8.
MEMSEL = $420D

; Joypads
; -------

; Joypad NES-style interface
; --------------------------

; JOYOUT - Joypad output ($4016 write)
;
; 7  bit  0
; ---- ----
; .... .210
;       |||
;       ||+- OUT0
;       ++-- OUT2-1 (not connected)
;
; OUT0 is used by standard controllers to latch the current button state. OUT2-1
; are not connected in standard consoles, but may be used in the Super Famicom Box
; hotel system.
JOYOUT = $4016

; JOYSER0 - Joypad serial data port 1 ($4016 read)
;
; 7  bit  0
; ---- ----
; xxxx xxDD
; |||| ||||
; |||| ||++- Joypad port 1 data 2-1
; ++++-++--- (Open bus)
;
; On read: Joypad port 1 is clocked (via joypad 1 /OE)
JOYSER0 = $4016

; JOYSER1 - Joypad serial data port 2 ($4017 read)
;
; 7  bit  0
; ---- ----
; xxx1 11DD
; |||| ||||
; |||| ||++- Joypad port 2 data 2-1
; |||+-++--- Joypad 2 D4-2 (always 1)
; +++------- (Open bus)
;
; On read: Joypad port 2 is clocked (via joypad 2 /OE)
;
; The CPU has 5 joypad 2 inputs. Joypad port 2's data 2 and 1 pins connect to
; D1-0, while D4-2 are tied to ground (and thus read as 1).
JOYSER1 = $4017

; Joypad I/O interface
; --------------------

; WRIO - Write I/O ($4201 write)
;
; 7  bit  0
; ---- ----
; 21DD DDDD
; |||| ||||
; ||++-++++- CPU I/O D5-0 (not connected)
; |+-------- Joypad port 1 I/O
; +--------- Joypad port 2 I/O, and
;            PPU /EXTLATCH light pen input
;
; On power-on: WRIO = $FF
;
; Used by the multitap to select reading of controllers 2/3 vs. 4/5.
WRIO = $4201

; RDIO - Read I/O ($4213 read)
;
; 7  bit  0
; ---- ----
; 21DD DDDD
; |||| ||||
; ||++-++++- CPU I/O D5-0 (not connected)
; |+-------- Joypad port 1 I/O
; +--------- Joypad port 2 I/O
;
; The I/O pins allow bidirectional communication between the CPU and joypads on a
; single wire per bit. Either side is able to set the bits to 0, so to read the
; value being sent by the other side, the reader must set its own corresponding
; bits to 1 before reading.
;
; The not-connected D5-0 bits can be used as general-purpose storage on standard
; consoles, but are used in the Super Famicom Box hotel system to communicate with
; its HD64180 CPU.  Joypad port 2's I/O bit is also connected to the PPU's
; /EXTLATCH input, allowing the PPU's H and V counters to be latched when this bit
; is set to 0 by the CPU or joypad. This is intended to support a light pen or gun
; device. This should normally be set to 1 by the CPU to allow the counters to be
; latched. (See PPU registers)
RDIO = $4213

; Auto-read results
; -----------------

; JOY1L, JOY1H - Joypad port 1 data 1 ($4218, $4219 read)
;
;   JOY1H       JOY1L
;   $4219       $4218
; 7  bit  0   7  bit  0
; ---- ----   ---- ----
; HHHH HHHH   LLLL LLLL
; |||| ||||   |||| ||||
; ++++-++++---++++-++++- Joypad port 1 data 1 (first read on left to last read on right)
JOY1L = $4218
JOY1H = $4219

; JOY2L, JOY2H - Joypad port 2 data 1 ($421A, $421B read)
;
;   JOY2H       JOY2L
;   $421B       $421A
; 7  bit  0   7  bit  0
; ---- ----   ---- ----
; HHHH HHHH   LLLL LLLL
; |||| ||||   |||| ||||
; ++++-++++---++++-++++- Joypad port 2 data 1 (first read on left to last read on right)
JOY2L = $421A
JOY2H = $421B

; JOY3L, JOY3H - Joypad port 1 data 2 ($421C, $421D read)
;
;   JOY3H       JOY3L
;   $421D       $421C
; 7  bit  0   7  bit  0
; ---- ----   ---- ----
; HHHH HHHH   LLLL LLLL
; |||| ||||   |||| ||||
; ++++-++++---++++-++++- Joypad port 1 data 2 (first read on left to last read on right)
JOY3L = $421C
JOY3H = $421D

; JOY4L, JOY4H - Joypad port 2 data 2 ($421E, $421F read)
;
;   JOY4H       JOY4L
;   $421F       $421E
; 7  bit  0   7  bit  0
; ---- ----   ---- ----
; HHHH HHHH   LLLL LLLL
; |||| ||||   |||| ||||
; ++++-++++---++++-++++- Joypad port 2 data 2 (first read on left to last read on right)
JOY4L = $421E
JOY4H = $421F

JOY_UP    = $0800
JOY_DOWN  = $0400
JOY_LEFT  = $0200
JOY_RIGHT = $0100

; Math
; ----

; Multiplication
; --------------

; WRMPYA - Multiplication factor A ($4202 write)
;
; 7  bit  0
; ---- ----
; DDDD DDDD
; |||| ||||
; ++++-++++- Multiplication factor (8-bit unsigned)
;
; On power-on: WRMPYA = $FF
WRMPYA = $4202

; WRMPYB - Multiplication factor B ($4203 write)
;
; 7  bit  0
; ---- ----
; DDDD DDDD
; |||| ||||
; ++++-++++- Multiplication factor (8-bit unsigned)
;
; On write: Begins multiplication process
;
; The multiplication process takes up to 8 cycles and the result is written to
; RDMPY as it goes. See 5A22 Multiplication for more information.
WRMPYB = $4203

; Division
; --------

; WRDIVL, WRDIVH - Dividend ($4204, $4205 write)
;
;   WRDIVH      WRDIVL
;   $4205       $4204
; 7  bit  0   7  bit  0
; ---- ----   ---- ----
; HHHH HHHH   LLLL LLLL
; |||| ||||   |||| ||||
; ++++-++++---++++-++++- Dividend (16-bit unsigned)
;
; On power-on: WRDIV = $FFFF
WRDIVL = $4204
WRDIVH = $4205

; WRDIVB - Divisor ($4206 write)
;
; 7  bit  0
; ---- ----
; DDDD DDDD
; |||| ||||
; ++++-++++- Divisor (8-bit unsigned)
;
; On write: Begins division process
;
; The division process takes up to 16 CPU cycles and the result is written to
; RDDIV (quotient) and RDMPY (remainder) as it goes. Dividing by 0 results in a
; quotient of $FFFF and a remainder equal to the dividend (WRDIV). See 5A22
; Division for more information.
WRDIVB = $4206

; Result
; ------

; RDDIVL, RDDIVH - Quotient ($4214, $4215 read)
;
;   RDDIVH      RDDIVL
;   $4215       $4214
; 7  bit  0   7  bit  0
; ---- ----   ---- ----
; HHHH HHHH   LLLL LLLL
; |||| ||||   |||| ||||
; ++++-++++---++++-++++- Quotient (16-bit unsigned)
RDDIVL = $4214
RDDIVH = $4215

; RDMPYL, RDMPYH - Product or Remainder ($4216, $4217 read)
;
;   RDMPYH      RDMPYL
;   $4217       $4216
; 7  bit  0   7  bit  0
; ---- ----   ---- ----
; HHHH HHHH   LLLL LLLL
; |||| ||||   |||| ||||
; ++++-++++---++++-++++- Product (16-bit unsigned), or
;                        Remainder (16-bit unsigned)
RDMPYL = $4216
RDMPYH = $4217

; DMA registers
; -------------
;
; The SNES's DMA (Direct Memory Access) unit allows a game to copy graphics,
; palettes, OAM and more at a much higher speed than the CPU can accomplish
; alone. This allows a game to make better use of the limited amount of time it
; has in vblank to change graphical memory.
;
; The SNES has two address buses: the CPU bus (also known as the A bus, which
; contains cartridge ROM, cartridge RAM, and the SNES's RAM) and the peripheral
; bus (also known as the B bus, which contains anything in the $2100-$21FF range,
; including PPU registers and APU registers). These buses use the same data bus,
; and DMA works by having a read on one address bus act as a write on the other,
; so copies are always from one bus to the other.
;
; Although it can be specified as both the source and destination, DMA cannot copy
; from one area of the console's 128 KiB S-WRAM to another. For that, the MVN and
; MVP instructions are probably the best available choice. However, DMA can copy
; between S-WRAM and some other kind of RAM, such as cartridge RAM.
;
; The SNES also features HDMA (Horizontal-blank DMA) which runs in the background
; and can be set up to automatically write values to hardware registers at
; specific scanlines, allowing for effects.
;
; Because some revisions of the SNES crash when a DMA completes just as an HDMA
; begins, DMA should only be used either within vertical blank, with rendering
; disabled, or with HDMA disabled.
;
; These registers are always accessed at 3.58 MHz! That means that any channels
; that are not currently in use can have their registers repurposed for a small
; amount of fast RAM.


; DMA channels
; ------------
;
; The SNES contains 8 separate DMA "channels" - each one contains a set of
; parameters to configure a DMA transfers. They are configured with registers in
; the $4300-$437f range, where the first 16 addresses correspond to the first
; register, the second 16 addresses correspond to the next, and so on.
;
; These channels are shared by both DMA and HDMA.
;
;
; MDMAEN - Start DMA transfer ($420B write)
;
; 7  bit  0
; ---- ----
; 7654 3210
; |||| ||||
; |||| |||+- Channel 0 select
; |||| ||+-- Channel 1 select
; |||| |+--- Channel 2 select
; |||| +---- Channel 3 select
; |||+------ Channel 4 select
; ||+------- Channel 5 select
; |+-------- Channel 6 select
; +--------- Channel 7 select
;
; On power-on: MDMAEN = $00
; On reset:    MDMAEN = $00
;
; Upon writing to this register, a DMA transfer is started for each bit that was
; set, starting with the lowest selected channel number up toward the highest. The
; CPU is stopped until all transfers have completed. If an HDMA transfer happens
; while the DMA transfer is going, the DMA transfer will be temporarily paused to
; allow the HDMA transfer to happen.
;
; After writing this register, DMA will normally take place in the middle of the
; following instruction, just after its opcode is read from memory.
;
; After the DMA completes, the DMA byte count (DASnL/DASnH) will be equal to 0,
; and the DMA source address (A1TnL/A1TnH) will have been incremented by the
; number of bytes read if address increment was used (DMAPn), though the bank
; address (A1Bn) will not change, so the source address will wrap at a bank
; boundary. This means:
;
;   - Since it is automatically returned to 0, a DMA byte count must normally be
;     rewritten before each DMA.
;
;   - If the source data for the next DMA begins at the end of the previous one,
;     it may not be necessary to set a new source address.
MDMAEN = $420B

; HDMAEN - Enable HDMA transfers ($420C write)
;
; 7  bit  0
; ---- ----
; 7654 3210
; |||| ||||
; |||| |||+- Channel 0 HDMA enable
; |||| ||+-- Channel 1 HDMA enable
; |||| |+--- Channel 2 HDMA enable
; |||| +---- Channel 3 HDMA enable
; |||+------ Channel 4 HDMA enable
; ||+------- Channel 5 HDMA enable
; |+-------- Channel 6 HDMA enable
; +--------- Channel 7 HDMA enable
;
; On power-on: HDMAEN = $00
; On reset:    HDMAEN = $00
; This register enables HDMA for the selected channels.
HDMAEN = $420C

; DMA channel registers
; ---------------------

; DMAPn - DMA/HDMA parameters ($43n0 read/write) (n = 0..7)
;
; 7  bit  0
; ---- ----
; DIxA APPP
; |||| ||||
; |||| |+++- Transfer pattern (see below)
; |||+-+---- Address adjust mode (DMA only):
; |||         0:   Increment A bus address after copy
; |||         1/3: Fixed
; |||         2:   Decrement A bus address after copy
; ||+------- (Unused)
; |+-------- Indirect (HDMA only)
; +--------- Direction: 0=Copy from A to B, 1=Copy from B to A
;
; On power-on: DMAPn = $FF
;
; The transfer pattern (P) controls the address pattern for different register
; types, and for HDMA also the number of bytes delivered per table entry.
;
; For example: pattern 1 allows a DMA copy to VRAM, which requires writing to two
; alternating addresses.
DMAP0 = $4300
DMAP1 = $4310
DMAP2 = $4320
DMAP3 = $4330
DMAP4 = $4340
DMAP5 = $4350
DMAP6 = $4360
DMAP7 = $4370

; DMA transfer patterns
; ---------------------
;
; Pattern  HDMA bytes  B Bus addr   Usage example
; 0        1           +0           WRAM, Mode 7 graphics/tilemap
; 1        2           +0 +1        VRAM
; 2        2           +0 +0        OAM, CGRAM
; 3        4           +0 +0 +1 +1  Scroll positions, Mode 7 parameters
; 4        4           +0 +1 +2 +3  Window
; 5        4           +0 +1 +0 +1  (Undocumented)
; 6        2           +0 +0        (Same as 2, undocumented)
; 7        4           +0 +0 +1 +1  (Same as 3, undocumented)
;
; A fixed address adjust mode (A) can be used to fill the DMA target with a single
; repeated byte of data.

; BBADn - B-bus address ($43n1 read/write) (n = 0..7)
;
; 7  bit  0
; ---- ----
; AAAA AAAA
; |||| ||||
; ++++-++++- Selects a hardware register to read or write from, in the $2100-$21ff range
;
; On power-on: BBADn = $FF
;
; This can be used with various PPU registers, and also WMDATA.
;
; Avoid starting a transfer with BBADn = $00. This causes some CPU revisions to
; silently fail to start a transfer. Instead, if controlling INIDISP through HDMA,
; use BBADn = $FF and pattern 1 to write the same value to $21FF and $2100.
BBAD0 = $4301
BBAD1 = $4311
BBAD2 = $4321
BBAD3 = $4331
BBAD4 = $4341
BBAD5 = $4351
BBAD6 = $4361
BBAD7 = $4371

; UNUSEDn - Unused byte ($43nB and $43nF read/write) (n = 0..7)
;
; 7  bit  0
; ---- ----
; NNNN NNNN
; |||| ||||
; ++++-++++- One unused byte available through two different addresses
;
; On power-on: UNUSEDn = $FF
;
; Seems to have no effect on DMA or HDMA, and this register cannot be used as a
; source for a DMA fill.


; Configuration registers (DMA)
; -----------------------------

; A1TnL, A1TnH, A1Bn - DMA Current Address ($43n2, $43n3, $43n4 read/write) (n = 0..7)
;
;   A1Bn        A1TnH       A1TnL
;   $43n4       $43n3       $43n2
; 7  bit  0   7  bit  0   7  bit  0
; ---- ----   ---- ----   ---- ----
; BBBB BBBB   HHHH HHHH   LLLL LLLL
; |||| ||||   |||| ||||   |||| ||||
; ++++-++++---++++-++++---++++-++++- Address on the A bus
;
; On power-on: A1Tn = $FFFFFF
;
; The low 16-bits of this address change as the DMA happens, but the bank byte is
; fixed. DMA can not cross banks.
;
; DMA cannot access A-bus addresses that overlap MMIO registers: $2100-$21FF,
; $4000-$41FF, $4200-$421F, $4300-$437F.
;
; HDMA uses these registers for its table address instead.
A1T0L = $4302
A1T1L = $4312
A1T2L = $4322
A1T3L = $4332
A1T4L = $4342
A1T5L = $4352
A1T6L = $4362
A1T7L = $4372

A1T0H = $4303
A1T1H = $4313
A1T2H = $4323
A1T3H = $4333
A1T4H = $4343
A1T5H = $4353
A1T6H = $4363
A1T7H = $4373

A1B0 = $4304
A1B1 = $4314
A1B2 = $4324
A1B3 = $4334
A1B4 = $4344
A1B5 = $4354
A1B6 = $4364
A1B7 = $4374

; DASnL, DASnH - DMA Byte-Counter ($43n5, $43n6 read/write) (n = 0..7)
;
;   DASnH       DASnL
;   $43n6       $43n5
; 7  bit  0   7  bit  0
; ---- ----   ---- ----
; HHHH HHHH   LLLL LLLL
; |||| ||||   |||| ||||
; ++++-++++---++++-++++- 16-bit number that indicates how many bytes to transfer
;
; On power-on: DASn = $FFFF
;
; A byte count of zero means 65536 bytes.
;
; This byte count is not affected by the DMA pattern. The SNES will stop before a
; pattern is completed if it runs out of bytes.
;
; Once the DMA finishes, these registers will be zero.
;
; HDMA uses these registers for its indirect address instead.
DAS0L = $4305
DAS1L = $4315
DAS2L = $4325
DAS3L = $4335
DAS4L = $4345
DAS5L = $4355
DAS6L = $4365
DAS7L = $4375

DAS0H = $4306
DAS1H = $4316
DAS2H = $4326
DAS3H = $4336
DAS4H = $4346
DAS5H = $4356
DAS6H = $4366
DAS7H = $4376

; Configuration registers (HDMA)
; ------------------------------
;
; A1TnL, A1TnH, A1Bn - HDMA Table Start Address ($43n2, $43n3, $43n4 read/write) (n = 0..7)
;
;   A1Bn        A1TnH       A1TnL
;   $43n4       $43n3       $43n2
; 7  bit  0   7  bit  0   7  bit  0
; ---- ----   ---- ----   ---- ----
; BBBB BBBB   HHHH HHHH   LLLL LLLL
; |||| ||||   |||| ||||   |||| ||||
; ++++-++++---++++-++++---++++-++++- 24-bit little-endian address on the A bus
;
; On power-on: A1Tn = $FFFFFF
;
; These registers control where the channel's HDMA table is. During initiation of
; HDMA this address gets copied into A2AnL/A2AnH ($43n8/$43n9).
;
;
; DASBn - Indirect HDMA Bank ($43n7 read/write) (n = 0..7)
;
; 7  bit  0
; ---- ----
; BBBB BBBB
; |||| ||||
; ++++-++++- High byte (bank) of HDMA indirect address
;
; On power-on: DASBn = $FF
;
; This must be set manually by the program to control the indirect HDMA
; bank. (DASnL/DASnH are automatically updated from the table.)
DASB0 = $4307
DASB1 = $4317
DASB2 = $4327
DASB3 = $4337
DASB4 = $4347
DASB5 = $4357
DASB6 = $4367
DASB7 = $4377

; Other HDMA registers
; --------------------
;
; These keep track of each channel's state as HDMA is happening.
;
;
; DASnL, DASnH, DASBn - Indirect HDMA Address ($43n5, $43n6, $43n7 read/write) (n = 0..7)
;
;   DASBn       DASnH       DASnL
;   $43n7       $43n6       $43n5
; 7  bit  0   7  bit  0   7  bit  0
; ---- ----   ---- ----   ---------
; BBBB BBBB   HHHH HHHH   LLLL LLLL
; |||| ||||   |||| ||||   |||| ||||
; ++++-++++---++++-++++---++++-++++- The current indirect DMA address.
;
; On power-on: DASn = $FFFFFF
;
; With indirect HDMA, if the repeat bit is set in the table entry, then the SNES
; will continue to read increasing addresses starting from the one given in the
; table, using these registers to keep track of where it currently is.
;
; The low 16 bits are automatically copied from the table, but the bank byte DASBn
; must be manually set by the program. The bank byte is fixed, and HDMA will not
; cross banks.
;
;
; A2AnL, A2AnH - HDMA Table Current Address ($43n8, $43n9 read/write) (n = 0..7)
;
;   A2AnH       A2AnL
;   $43n9       $43n8
; 7  bit  0   7  bit  0
; ---- ----   ---- ----
; HHHH HHHH   LLLL LLLL
; |||| ||||   |||| ||||
; ++++-++++---++++-++++- Low 16 bits of the current address within the HDMA table
;
; On power-on: A2An = $FFFF
;
; Bank byte is taken from $43n4, as it does not change.
;
;
; NLTRn - HDMA Line-Counter ($43nA read/write) (n = 0..7)
;
; 7  bit  0
; ---- ----
; RLLL LLLL
; |||| ||||
; |+++-++++- Number of scanlines left
; +--------- Repeat flag
;
; On power-on: NLTRn = $FF
;
; Automatically loaded from the table. Scanline count is decremented every
; scanline until it hits zero.
;
;
; HDMA table format
; -----------------
;
; HDMA tables specify what values to write to the selected B bus register, as well
; as which scanlines to write the values on. The tables can either directly
; contain the values (Direct mode) or specify 16-bit pointers that are then used
; to get the values (Indirect mode).
;
;
; Direct HDMA table entries
; -------------------------
;
; 1 byte - Line count, and repeat mode
; N bytes - Data
;
; The number of bytes in each Data section is determined by the pattern chosen for
; the channel in register $43n0.
;
; - If repeat mode is off, one pattern's worth of bytes are written, then the SNES
;   waits for the specified number of scanlines before continuing onto the next
;   table entry.
;
; - If repeat mode is on, then the total size of the data section is the number of
;   scanlines multiplied by the number of bytes in the pattern. One pattern's
;   worth of bytes are written for however many scanlines indicated in the table.
;
;
; Indirect HDMA table entries
; ---------------------------
;
; 1 byte - Line count, and repeat mode
; 2 bytes - Pointer to access the data through
;
; If repeat mode is off, one pattern's worth of bytes are written from an address
; starting from the pointer given.
;
; If repeat mode is on, the SNES continues to progress through the bytes that the
; pointer points to, for however many scanlines are indicated in the table.
;
;
; Line count, repeat mode byte
; ----------------------------
;
; Possible values for the "Line count, and repeat mode" byte are as follows:
;
; - $00: Stop processing HDMA on that channel for the rest of the frame.
; - $01-$80: Write once, then wait for X scanlines
; - $81-$FF: Write every scanline for X-$80 scanlines, repeat mode
;
; Note that the first entry in the HDMA table is executed at the end of scanline
; 0, which is always hidden in blanking.
;
; For example: if you wanted the second HDMA entry to apply to scanline 24, use
; X=23 for the first entry, which causes the next entry to apply at the end of
; scanline 23, so the change will be visible on scanline 24.

.endif