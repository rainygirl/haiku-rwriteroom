#!/bin/sh
# Cross-compiles R WriteRoom for Haiku and leaves dist/<arch>/RWriteRoom,
# resources attached (rc + xres from the Haiku build tools). Nothing has to
# run on Haiku itself.
#
#   tools/cross-build.sh arm64    container with a Haiku arm64 build tree
#   tools/cross-build.sh x86      gcc2 hybrid's modern x86 compiler, on the
#                                 build host that has an x86_gcc2h tree
#   tools/cross-build.sh x86_64   haiku/cross-compiler:x86_64-r1beta4 image
#
#   ARM64_CONTAINER   default haiku-builder
#   ARM64_GENERATED   default /root/renku-arm64-work/generated.arm64
#   X86_HOST          default rainygirl@MacMiniM4 (ssh; Docker Desktop there)
#   X86_CONTAINER     default vaio-p-builder
#   X86_GENERATED     default /vaio-p/work/generated.x86_gcc2h
#   X86_VIA=image     build x86 here with haiku/cross-compiler:x86_gcc2h-r1beta4
#                     instead (when the x86 build host is not available)
set -e

ARCH="$1"
HERE="$(cd "$(dirname "$0")/.." && pwd)"
OUT="$HERE/dist/$ARCH"
mkdir -p "$OUT"

# One line: it is expanded inside the remote sh -c script.
SRCS="App EditorView Settings Strings WriteWindow"
CXXFLAGS="-O2 -Wall -Wno-multichar -std=c++17"
LIBS_COMMON="-lbe -ltracker -lstdc++"

# Compile, link, then attach the resources. $CXX, $INC, $LIBS, $RC and
# $XRES are set by the caller inside the container.
BUILD='
	for s in '"$SRCS"'; do
		$CXX -c src/$s.cpp '"$CXXFLAGS"' -Isrc $INC -o $s.o >&2
	done
	$CXX -o RWriteRoom *.o -Xlinker -soname=_APP_ $LIBS >&2
	$RC -o app.rsrc src/app.rdef >&2
	$XRES -o RWriteRoom app.rsrc >&2
	tar cf - RWriteRoom'

# The arm64 tree's host rc/xres also serve the x86_64 build.
ARM64_CONTAINER="${ARM64_CONTAINER:-haiku-builder}"
ARM64_GENERATED="${ARM64_GENERATED:-/root/renku-arm64-work/generated.arm64}"

# Resources for a binary built where there is no rc/xres: the arm64 tree's
# host tools handle any ELF.
attach_resources()
{
	( cd "$HERE" && tar cf - src/app.rdef -C "$OUT" RWriteRoom ) \
		| docker exec -i "$ARM64_CONTAINER" sh -c "
			set -e
			W=/tmp/rwr-res-\$\$; mkdir -p \$W && cd \$W && tar xf -
			T=$ARM64_GENERATED/objects/linux/arm64/release/tools
			RC=\$T/rc/rc; [ -x \$RC ] || RC=\$T/rc
			export LD_LIBRARY_PATH=$ARM64_GENERATED/objects/linux/lib
			\$RC -o app.rsrc src/app.rdef >&2
			\$T/xres -o RWriteRoom app.rsrc >&2
			tar cf - RWriteRoom; rm -rf \$W" \
		| tar xf - -C "$OUT"
}

case "$ARCH" in
arm64)
	GEN="$ARM64_GENERATED"
	P="$GEN/objects/haiku/arm64/packaging/packages_build/minimum"
	O="$GEN/objects/haiku/arm64/release"
	H="$(dirname "$GEN")/haiku"
	WORK="/tmp/rwr-cross-$$"
	( cd "$HERE" && tar cf - src ) \
		| docker exec -i "$ARM64_CONTAINER" bash -c "
			set -e
			mkdir -p $WORK/sysroot/boot/system/develop/lib && cd $WORK && tar xf -
			S=$WORK/sysroot/boot/system
			if [ -d $P/hpkg_-haiku_devel.hpkg/contents/develop/headers ]; then
				rmdir \$S/develop/lib \$S/develop
				ln -sfn $P/hpkg_-haiku_devel.hpkg/contents/develop \$S/develop
				LIBDIRS=\"-L$P/hpkg_-haiku.hpkg/contents/lib\"
			else
				# The tree is between builds and its haiku_devel is not
				# unpacked: headers from the source tree, the kits and glue
				# from the object tree, start_dyn.o compiled here.
				for d in os posix glibc config cpp; do ln -s $H/headers/\$d \$S/develop/headers/\$d 2>/dev/null || { mkdir -p \$S/develop/headers; ln -s $H/headers/\$d \$S/develop/headers/\$d; }; done
				ln -s $H/headers/compatibility/bsd \$S/develop/headers/bsd
				ln -s $H/headers/compatibility/gnu \$S/develop/headers/gnu
				cp $O/system/glue/arch/arm64/crti.o $O/system/glue/arch/arm64/crtn.o \
					$O/system/glue/init_term_dyn.o $O/system/glue/haiku_version_glue.o \
					$O/kits/libbe.so $O/system/libroot/libroot.so \
					$O/kits/tracker/libtracker.so \$S/develop/lib/
				$GEN/cross-tools-arm64/bin/aarch64-unknown-haiku-gcc --sysroot=$WORK/sysroot \
					-O2 -fPIC -I$H/headers/private/system \
					-I$H/headers/private/system/arch/arm64 \
					-c $H/src/system/glue/start_dyn.c -o \$S/develop/lib/start_dyn.o
				LIBDIRS=\"-L\$S/develop/lib\"
			fi
			SYSLIBS=\$(ls -d $GEN/build_packages/gcc_syslibs-*-arm64/lib | head -1)
			CXX=\"$GEN/cross-tools-arm64/bin/aarch64-unknown-haiku-g++ --sysroot=$WORK/sysroot\"
			INC=
			LIBS=\"\$LIBDIRS -L\$SYSLIBS $LIBS_COMMON\"
			T=$GEN/objects/linux/arm64/release/tools
			RC=\$T/rc/rc; [ -x \$RC ] || RC=\$T/rc
			XRES=\$T/xres
			export LD_LIBRARY_PATH=$GEN/objects/linux/lib
			$BUILD
			rm -rf $WORK" \
		| tar xf - -C "$OUT"
	;;
x86)
	if [ "${X86_VIA:-}" = image ]; then
		( cd "$HERE" && tar cf - src ) \
			| docker run --rm -i --platform linux/amd64 haiku/cross-compiler:x86_gcc2h-r1beta4 sh -c "
				set -e
				mkdir -p /work && cd /work && tar xf -
				CXX=/tools/cross-tools-x86/bin/i586-pc-haiku-g++
				for s in $SRCS; do
					\$CXX -c src/\$s.cpp $CXXFLAGS -Isrc -o \$s.o >&2
				done
				\$CXX -o RWriteRoom *.o -Xlinker -soname=_APP_ $LIBS_COMMON >&2
				tar cf - RWriteRoom" \
			| tar xf - -C "$OUT"
		attach_resources
		file "$OUT/RWriteRoom"
		exit 0
	fi
	# Runs on another machine over ssh, so the build steps travel as a script
	# file inside the tar stream rather than through two layers of quoting.
	HOST="${X86_HOST:-rainygirl@MacMiniM4}"
	CONTAINER="${X86_CONTAINER:-vaio-p-builder}"
	GEN="${X86_GENERATED:-/vaio-p/work/generated.x86_gcc2h}"
	P="$GEN/objects/haiku/x86_gcc2/packaging/packages_build/regular"
	STAGE="$(mktemp -d)"
	cp -R "$HERE/src" "$STAGE/"
	cat > "$STAGE/build.sh" <<SCRIPT
set -e
W=\$(pwd)
TOOLS=\$(ls -d $GEN/objects/linux/*/release/tools | head -1)
RC=\$TOOLS/rc/rc; [ -x \$RC ] || RC=\$TOOLS/rc
XRES=\$TOOLS/xres
export LD_LIBRARY_PATH=$GEN/objects/linux/lib
mkdir -p \$W/syslibs
for p in gcc_x86_syslibs_devel gcc_x86_syslibs; do
	PKG=\$TOOLS/package/package; [ -x \$PKG ] || PKG=\$TOOLS/package
	\$PKG extract -C \$W/syslibs \$(ls $GEN/download/\$p-*.hpkg | head -1) >&2
done
# develop/lib/x86 holds relative links into lib/x86 of another package, so
# the sysroot is real directories with both in their usual places.
S=\$W/sysroot/boot/system
mkdir -p \$S/develop/lib
ln -sfn $P/hpkg_-haiku_devel.hpkg/contents/develop/headers \$S/develop/headers
cp -a $P/hpkg_-haiku_x86_devel.hpkg/contents/develop/lib/x86 \$S/develop/lib/
ln -sfn $P/hpkg_-haiku_x86.hpkg/contents/lib \$S/lib
X86DEV=$P/hpkg_-haiku_x86_devel.hpkg/contents/develop
CXX="$GEN/cross-tools-x86/bin/i586-pc-haiku-g++ --sysroot=\$W/sysroot"
INC="-I\$X86DEV/headers"
LIBS="-B\$S/develop/lib/x86/ -L\$S/develop/lib/x86 -L\$W/syslibs/develop/lib/x86 -L\$W/syslibs/lib/x86 $LIBS_COMMON -lgcc_s"
$BUILD
SCRIPT
	( cd "$STAGE" && tar cf - src build.sh ) \
		| ssh "$HOST" "/usr/local/bin/docker --context desktop-linux exec -i $CONTAINER sh -c 'W=/tmp/rwr-cross-\$\$; mkdir -p \$W && cd \$W && tar xf - && sh build.sh; rc=\$?; rm -rf \$W; exit \$rc'" \
		| tar xf - -C "$OUT"
	rm -rf "$STAGE"
	;;
x86_64)
	# The image has Haiku's headers and kits but no rc/xres: the resources
	# are attached afterwards with the arm64 tree's host tools.
	( cd "$HERE" && tar cf - src ) \
		| docker run --rm -i --platform linux/amd64 haiku/cross-compiler:x86_64-r1beta4 sh -c "
			set -e
			mkdir -p /work && cd /work && tar xf -
			for s in $SRCS; do
				x86_64-unknown-haiku-g++ -c src/\$s.cpp $CXXFLAGS -Isrc -o \$s.o >&2
			done
			x86_64-unknown-haiku-g++ -o RWriteRoom *.o -Xlinker -soname=_APP_ \
				$LIBS_COMMON >&2
			tar cf - RWriteRoom" \
		| tar xf - -C "$OUT"
	attach_resources
	;;
*)
	echo "usage: $0 arm64|x86|x86_64" >&2
	exit 1
	;;
esac

file "$OUT/RWriteRoom"
