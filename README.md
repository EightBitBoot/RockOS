# Rock OS

This is an operating system made as part of a college group project.  
My contributions are the virtual fs layer, kstring library, slab allocator, and a large portion of the userspace shell.  
  
The original systems the build system was writen on contained outdated versions of gcc, ld, and qemu-system-x86 so no commits before the tag `build_on_modern_machines` are guaranteed to build or run.

# Prerequisites
- The following instructions are for `x86_64 Debian 12`

- Packages
    - `git`
    - `gcc`
    - `gcc-multilib`
    - `make`
    - `qemu-system-x86`

- [x32 ABI](https://wiki.debian.org/X32Port)
    - The build process requires a kernel built with the config option `CONFIG_X86_X32_ABI=y`
    - If the kernel is also built with `CONFIG_X86_X32_DISABLED=y` (Debian 12 _is_)
        1. Edit `/etc/default/grub` to contain the following line
            - `GRUB_CMDLINE_LINUX_DEFAULT="syscall.x32=y quiet"`
        2. Run `sudo update-grub2`
        3. Reboot the machine

# Building

```bash
git clone https://github.com/EightBitBoot/RockOS.git
cd rock_os
make
```

# Running

```bash
./QRUN
```

# Versions
These are the various program versions building and running the was tested on.

| Program           | Version |
|:------------------|:-------:|
| `gcc`             | 12.2.0  |
| `gcc-multilib`    | 12.2.0  |
| `ld`              | 2.4.0   |
| `qemu-system-x86` | 10.0.0  |
