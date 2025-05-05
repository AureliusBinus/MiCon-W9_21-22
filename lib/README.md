# Project Libraries

This directory is intended for project specific (private) libraries.
PlatformIO will compile them to static libraries and link into the executable file.

The source code of each library should be placed in a separate directory
("lib/your_library_name/[Code]").

For example, see the structure of the following example libraries `Foo` and `Bar`:

```
lib/
├── Bar/
│   ├── docs/
│   ├── examples/
│   ├── src/
│   │   ├── Bar.c
│   │   └── Bar.h
│   └── library.json (optional, for custom build options)
│
├── Foo/
│   ├── Foo.c
│   └── Foo.h
│
└── README.md (this file)

platformio.ini
src/
└── main.c
```

For more information about the library configuration, see the [library.json documentation](https://docs.platformio.org/page/librarymanager/config.html).

Example contents of `src/main.c` using Foo and Bar:

```c
#include <Foo.h>
#include <Bar.h>

int main (void)
{
  ...
}
```

The PlatformIO Library Dependency Finder will find automatically dependent
libraries by scanning project source files.

More information about PlatformIO Library Dependency Finder:

- [Library Dependency Finder](https://docs.platformio.org/page/librarymanager/ldf.html)
