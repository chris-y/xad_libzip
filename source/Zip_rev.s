VERSION = 1
REVISION = 1

.macro DATE
.ascii "16.4.2026"
.endm

.macro VERS
.ascii "Zip 1.1"
.endm

.macro VSTRING
.ascii "Zip 1.1 (16.4.2026)"
.byte 13,10,0
.endm

.macro VERSTAG
.byte 0
.ascii "$VER: Zip 1.1 (16.4.2026)"
.byte 0
.endm
