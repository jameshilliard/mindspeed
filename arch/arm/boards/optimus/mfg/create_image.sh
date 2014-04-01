#!/bin/bash

# Assemble NOR image for pre-programming on NOR chips

# nor_parts="128k(uloader)ro,512k(loader0)ro,512k(loader1)ro,128k(env),1M(hnvram),8M(kernel),8M(initrd)"

OUTPUT=optimus.pvt.nor.image

python -c "import sys; sys.stdout.write(chr(0xff)*1024*(128+512+512+128))" > $OUTPUT

dd if=uloader.bin of=$OUTPUT seek=0 bs=1K conv=nocreat,notrunc
dd if=barebox.mfg.bin of=$OUTPUT seek=128 bs=1K conv=nocreat,notrunc
