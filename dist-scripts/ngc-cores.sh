#!/bin/sh

make -C ../ -f Makefile.griffin platform=ngc clean || exit 1

for f in *_ngc.a ; do
   name=`echo "$f" | sed 's/\(_libretro_ngc\|\).a$//'`
   whole_archive=
   big_stack=
   use_overlays=
   cfg_name=
   if [ $name = "gpspS" ] ; then
      echo "GPSP found, enabling overlays..."
      use_overlays="OVERLAY=1"
      cfg_name="CFG_NAME=1"
   fi
   if [ $name = "nxengine" ] ; then
      echo "NXEngine found, applying whole archive linking..."
      whole_archive="WHOLE_ARCHIVE_LINK=1"
   fi
   if [ $name = "tyrquake" ] ; then
      echo "Tyrquake found, applying big stack..."
      big_stack="BIG_STACK=1"
   fi
   cp -f "$f" ../libretro_ngc.a
   make -C ../ -f Makefile.griffin platform=ngc $cfg_name $use_overlays $whole_archive $big_stack -j3 || exit 1
   mv -f ../retroarch_ngc.dol ../ngc/pkg/${name}_libretro_ngc.dol
   rm -f ../retroarch_ngc.dol ../retroarch_ngc.elf ../retroarch_ngc.elf.map
done
