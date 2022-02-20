# MikeOS BASIC Compiler

A compiler written in C which compiles MikeOS' BASIC dialect to executables for
this operating system. For documentation on the language itself, please read the
[article](http://mikeos.sourceforge.net/handbook-appdev-basic.html) on MikeOS'
[website](http://mikeos.sourceforge.net/).

## Building

For release build:
```
make all 	(default)
make release	(explicit release)
```

For debug build:
```
make debug
```

For cleanup (deletes all object files):
```
make wincln	(for Windows with MINGW)
make nixcln	(for *nixes and CYGWIN)
```

## Licensing

For full text of the license, see file [LICENSE](LICENSE).

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.

## Documentation

See `docs/`, especially [docs.md](docs/docs.md).

## Contributing
