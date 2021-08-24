uDOS which stands for Unified Data Operation System. A system whose primary goal
is to provide an opensource system for documenting S390X architectures - in
order to promote hobby operating systems on this obscure archtiecture.

A list of goals is provided below:
- Multiarchitecture
- Monolhitic modular kernel
- JCL interpreter out-of-box; integrated tightly with the system
- Integration with graphical windows management systems
- NetKernel-only operation (no graphical user interface and pure networking)
- POSIX ports
- UDI integration with the kernel
- RAM and CPU hot(un)plugging on the run

********************************************************************************
Compiling
********************************************************************************
uDOS uses autotools as the main build system generator because it's very based

In order to generate the configuration scripts you only need to run:
"autoreconf"

Use "./configure --host=s390-linux --target=s390-linux" in order to configure
for a MVS-compatible target.

After the configuration script configures stuff you only need to do "make -j"
to compile the kernel itself.

In order to run the system just do
"qemu-system-s390x -kernel src/kernel -serial stdio -d guest_errors,unimp"
