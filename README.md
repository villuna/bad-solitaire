# Bad Solitaire

This is a solitaire game I wrote as a project to learn C++ (and to see if I can write a good solitaire bot), which uses SDL for rendering.

To build it, you will need meson and the SDL development libraries. Once those are installed, just run:

```bash
# To build the project
mkdir build
meson setup build
meson compile -C build

# To run it:
./build/bs
```
Then have fun :)

I'm still a bit of a C++ novice so sorry if the code is a little sloppy.
