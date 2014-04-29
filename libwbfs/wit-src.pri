# crypto
HEADERS *= \
    wit-src/crypto/md32_common.h \
    wit-src/crypto/sha_locl.h \
    wit-src/crypto/ssl-asm.h \
    wit-src/crypto/wit-sha.h
    
SOURCES *= \
    wit-src/crypto/sha1_one.c \
    wit-src/crypto/sha1dgst.c \
    wit-src/crypto/ssl-asm.S

cygwin*|mingw* {
    SOURCES *= wit-src/crypto/sha1-586-cygwin.s
}

unix {
    isEqual( QMAKE_HOST.arch, x86_64 ) {
        SOURCES *= wit-src/crypto/sha1-x86_64.s
    } else:isEqual( QMAKE_HOST.arch, x86 ) {
        SOURCES *= wit-src/crypto/sha1-586-elf.s
    } else {
        error( "No crypto asm for your arch" )
    }
}

# libbz2
HEADERS *= \
    wit-src/libbz2/bzlib.h \
    wit-src/libbz2/bzlib_private.h

SOURCES *= \
    wit-src/libbz2/blocksort.c \
    wit-src/libbz2/bzlib.c \
    wit-src/libbz2/compress.c \
    wit-src/libbz2/crctable.c \
    wit-src/libbz2/decompress.c \
    wit-src/libbz2/huffman.c \
    wit-src/libbz2/randtable.c

# lzma
HEADERS *= \
    wit-src/lzma/LzFind.h \
    wit-src/lzma/LzHash.h \
    wit-src/lzma/Lzma2Dec.h \
    wit-src/lzma/Lzma2Enc.h \
    wit-src/lzma/LzmaDec.h \
    wit-src/lzma/LzmaEnc.h \
    wit-src/lzma/Types.h

SOURCES *= \
    wit-src/lzma/LzFind.c \
    wit-src/lzma/Lzma2Dec.c \
    wit-src/lzma/Lzma2Enc.c \
    wit-src/lzma/LzmaDec.c \
    wit-src/lzma/LzmaEnc.c \

# libwbfs
HEADERS *= \
    wit-src/libwbfs/cert.h \
    wit-src/libwbfs/file-formats.h \
    wit-src/libwbfs/libwbfs.h \
    wit-src/libwbfs/libwbfs_defaults.h \
    wit-src/libwbfs/libwbfs_os.posix.h \
    wit-src/libwbfs/rijndael.h \
    wit-src/libwbfs/tools.h \
    wit-src/libwbfs/wiidisc.h
    
SOURCES *= \
    wit-src/libwbfs/cert.c \
    wit-src/libwbfs/file-formats.c \
    wit-src/libwbfs/libwbfs.c \
    wit-src/libwbfs/rijndael.c \
    wit-src/libwbfs/tools.c \
    wit-src/libwbfs/wiidisc.c

# lib wrappers
HEADERS *= \
    wit-src/lib-bzip2.h \
    wit-src/lib-ciso.h \
    wit-src/lib-error.h \
    wit-src/lib-lzma.h \
    wit-src/lib-sf.h \
    wit-src/lib-std.h \
    wit-src/lib-wdf.h \
    wit-src/lib-wia.h \
    wit-src/dclib-utf8.h \
    wit-src/libwbfs_os.h
    
SOURCES *= \
    wit-src/lib-bzip2.c \
    wit-src/lib-ciso.c \
    wit-src/lib-file.c \
    wit-src/lib-lzma.c \
    wit-src/lib-sf.c \
    wit-src/lib-std.c \
    wit-src/lib-wdf.c \
    wit-src/lib-wia.c \
    wit-src/dclib-utf8.c

# interfaces
HEADERS *= \
    wit-src/iso-interface.h \
    wit-src/wbfs-interface.h

SOURCES *= \
    wit-src/iso-interface.c \
    wit-src/wbfs-interface.c \
    wit-src/crypt.h

# wit
HEADERS *= \
    wit-src/match-pattern.h \
    wit-src/debug.h \
    wit-src/patch.h \
    wit-src/titles.h \
    wit-src/system.h \
    wit-src/types.h \
    wit-src/ui/ui.h

SOURCES *= \
    wit-src/match-pattern.c \
    wit-src/debug.c \
    wit-src/patch.c \
    wit-src/titles.c \
    wit-src/ui/ui.c

# Not used

# ui
wit-ui {
    HEADERS *= \
        wit-src/ui/ui-wdf.h \
        wit-src/ui/ui-wfuse.h \
        wit-src/ui/ui-wit.h \
        wit-src/ui/ui-wwt.h
    
    SOURCES *= \
        wit-src/ui/ui-wdf.c \
        wit-src/ui/ui-wfuse.c \
        wit-src/ui/ui-wit.c \
        wit-src/ui/ui-wwt.c
}

# generators
generators {
    SOURCES *= \
        wit-src/ui/gen-ui.c \
        wit-src/ui/tab-ui.c
}

# arrays {
arrays {
    HEADERS *= \
        wit-src/ui/tab-wit.inc \
        wit-src/ui/tab-wwt.inc \
        wit-src/ui/tab-wdf.inc \
        wit-src/ui/tab-wfuse.inc
}

# applications
applications {
    SOURCES *= \
        wit-src/wdf.c \
        wit-src/wfuse.c \
        wit-src/wit.c \
        wit-src/wtest.c \
        wit-src/wwt.c \
        wit-src/wwt+wit-cmd.c \
        wit-src/wit-mix.c \
        wit-src/none.c
}
