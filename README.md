# LuaStarter
This project is intended to the be the base foundations for starting my future projects with the ability to handle porting events, functions, methods, full classes and datatypes easily to lua. It is an environment, interpreter, and binder all in one.

## Compilation

### Windows

You need visual studio 2022 and vcpkg with commit `54cc53c43430c73f489e52af5fadd032c1aced16` ([download](https://github.com/microsoft/vcpkg/archive/54cc53c43430c73f489e52af5fadd032c1aced16.zip)).

Then you install vcpkg dependencies for x86:

#### For x64
```bash
vcpkg install luajit:x64-windows-static physfs:x64-windows-static pugixml:x64-windows-static parallel-hashmap:x64-windows-static
```

#### For x86
```bash
vcpkg install luajit:x86-windows-static physfs:x86-windows-static pugixml:x86-windows-static parallel-hashmap:x86-windows-static
```