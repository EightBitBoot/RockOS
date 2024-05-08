# Adin

- [x] System to register filesystems
- [ ] Syscalls
  - [x] open
  - [x] close
  - [x] read
  - [x] write
  - [x] listdir
  - [ ] create
  - [ ] delete
  - [x] ioctl

  - [ ] chdir (ability to change cwd of process; not mentioned in presentation but probably necessary)
  - [x] seek (not mentioned in the presentation, but probably necessary)
  - [ ] getcwd (get the absolute path of a process's cwd; not mentioned in presentation but probably useful for the shell)

  - [ ] move / rename (optional)
  - [ ] mount (optional; probably outside the scope of this project)
- [ ] "." and ".." (relative path) support in namey()
- [x] Read and write locks
- [ ] direntry cache
- [x] Open file table in PCBs
- [x] _SMALL_ pass fopen flags to open driver call?
- [ ] Documentation
- [ ] Mounting system
- [ ] _REALLY FUCKING BASIC_ (maybe even read-only) sysfs
- [ ] stdin / stdout system
  - [ ] Automatic opening of first two fds (0 and 1)
  - [ ] Hooking into tty driver
  - [ ] Blocking reads and writes? (probably optional)
  - [ ] Userland input and print functions (writing to and reading from std{in|out})
- [ ] Shell
  - [ ] Cursor movement
  - [ ] Command history (optional)
  - [ ] stdio capabilities

### Shelved

- [ ] Syscalls
  - [ ] stat (not mentioned in the presentation, but probably necessary [_SPECIFICALLY_ for the shell to tell which file  s in the current directory it can navigate to])
      - [x] Could also be part of listdir (above)

- [ ] Buddy system allocator (semi optional: discussed in presentation but "in the middle of implementing it" so can probaly get away with using a slab cache for filenames)
- [ ] Error checking in all major functions