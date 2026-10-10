# drag

A super lightweight C utility that allows you to initiate a drag-and-drop event from
the command line on Linux. It supports Wayland (and only) natively, with no GTK 
or Qt dependencies - just pure Wayland libraries.

## Build
```
git clone https://github.com/chocomilkest/drag
cd drag
make
```
after that link/move the binary urself to /usr/local/bin/ (will make a make install later)

## Usage

```bash
drag /path/to/your/file.png
```

1. Move your mouse slightly. A window will appear under your cursor displaying the file name.
2. Drag (you have to keep your mouse clicked) and drop it into another application (browser, Discord, file manager, etc.).

### about this fork

this fork focuses on supporting wayland primarily because its what i mostly use and familiar with :P
changes i made:
- [x] remove x11
- [x] multi-output support (currently popup gets stuck when cursor moves out of the output drag was opened in)
- [x] multi-file support (drag multiple files)
- [ ] reorganize include folder
- [ ] region to normal layer surface to be able to focus during non drag
