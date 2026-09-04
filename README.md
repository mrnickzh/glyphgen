# GlyphGen - simple tool for baking fonts

The tool is mostly needed when you want do draw some text and dont want to take any external dependencies.
The font parsing and generation depends on stb_ttf.

The repo contains:
- tool itself `glyphgen.c`
- a simple wrapper for using it `gg_read.c, gg_read.h`
- a GLFW3 example program to test the tool `main.c`
- a windows .bat build script `build.bat`

To generate font you need to write a "charranges.cfg" config, you can see an example of it in the repo.
The structure of the config is the following, where `start_charN` is charcode of the first char in the range and `range_lenN` is the amount of chars to be taken after the first char.
```
start_char1 range_len1
start_char2 range_len2
start_char3 range_len3
```

The tool outputs different data, depending on the settings
The usage is `Usage: glyphgen.exe atlas_width atlas_height size font.ttf (atlas.txt/atlas.png) (data.txt/data.fdt)`

`(atlas.txt/atlas.png)` setting:
- when `.txt` file is supplied - generates a byte array that contains raw atlas data
 when `.png` file is supplied - generates a .png image of the atlas, using **stb_image_write**

`(data.txt/data.fdt)` setting:
- when `.txt` file is supplied - generates a float array of char data in the following format: `xadvance offy x0 x1 y0 y1 uvx0 uvx1 uvy0 uvy1`
- when `.fdt` file is supplied - generates a plaintext of the followring format: `charcode xadvance offy x0 x1 y0 y1 uvx0 uvx1 uvy0 uvy1`

The tool also automatically generates a code for **gg_get_char_index** function, the code is written to `code.c`
