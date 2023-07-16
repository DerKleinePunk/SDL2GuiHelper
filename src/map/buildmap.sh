#!/bin/bash

# wget exits with code 8 on "429 Too Many Requests" todo implement this in Script
# https://www.gnu.org/software/wget/manual/html_node/Exit-Status.html

if [ $# -lt 2 ] ; then
	echo "No continent / country specified Region can be defined as 3d Parameter"
	exit 1
fi

CONTINENT=$1
COUNTRY=$2
REGION=$3

echo "try build map for <$CONTINENT> Country <$COUNTRY> Region <$REGION> "

srtmver=2.1
downloadmap=no
BASEDIR=$PWD
OSMSCOUTDIR=/usr/local/bin/

#1sec
RESOLUTION=1
STEP=20
CAT=400,100

#3sec
STEP=100
CAT=1000,500
RESOLUTION=3


mkdir -p tmp
mkdir -p $CONTINENT
mkdir -p $CONTINENT/$COUNTRY

if [ ! -z "$REGION" ]; then
    mkdir -p $CONTINENT/$COUNTRY/$REGION
else
    echo "region is empty"
fi

if [ ! -z "$REGION" ]; then
    wget "http://download.geofabrik.de/$CONTINENT/$COUNTRY/$REGION-latest.osm.pbf.md5" -O$CONTINENT/$COUNTRY/$REGION/$REGION-latest.osm.pbf.md5
    exitCode=$?
else
    wget "http://download.geofabrik.de/$CONTINENT/$COUNTRY-latest.osm.pbf.md5" -O$CONTINENT/$COUNTRY/$COUNTRY-latest.osm.pbf.md5
    exitCode=$?
fi

if [ $exitCode -ne 0 ] ; then
   echo "Error down load md5"
   exit $exitCode
fi

if [ ! -z "$REGION" ]; then
    file1=$CONTINENT/$COUNTRY/$REGION/$REGION-latest.osm.pbf.md5
    file2=$CONTINENT/$COUNTRY/$REGION/current.md5
else
    file1=$CONTINENT/$COUNTRY/$COUNTRY-latest.osm.pbf.md5
    file2=$CONTINENT/$COUNTRY/current.md5
fi

if [ ! -z "$REGION" ]; then
    if [ ! -f $CONTINENT/$COUNTRY/$REGION/current.md5 ] ; then
        echo "$CONTINENT/$COUNTRY/$REGION/current.md5 not found!"
        if [ -f $CONTINENT/$COUNTRY/$REGION/$REGION-latest.osm.pbf ] ; then
            md5sum < $CONTINENT/$COUNTRY/$REGION/$REGION-latest.osm.pbf| cut -c 1-32 > $file2
            truncate --size -1 $file2 # Todo find better way to remove the new line
            printf "  $REGION-latest.osm.pbf\n" >> $file2
        else
            downloadmap=yes
        fi
    else
        if ! cmp "$file1" "$file2"; then
            echo "md5 changed download new"
            downloadmap=yes
        fi
    fi
else
    if [ ! -f $CONTINENT/$COUNTRY/current.md5 ] ; then
        echo "$CONTINENT/$COUNTRY/current.md5 not found!"
        if [ -f $CONTINENT/$COUNTRY/$COUNTRY-latest.osm.pbf ] ; then
            md5sum < $CONTINENT/$COUNTRY/$COUNTRY-latest.osm.pbf| cut -c 1-32 > $file2
            truncate --size -1 $file2 # Todo find better way to remove the new line
            printf "  $COUNTRY-latest.osm.pbf\n" >> $file2
        else
            downloadmap=yes
        fi
    else
        if ! cmp "$file1" "$file2"; then
            echo "md5 changed download new"
            downloadmap=yes
        fi
    fi
fi

echo $downloadmap

if [ $downloadmap == "yes" ] ; then
    if [ ! -z "$REGION" ]; then
        echo "try download"
        wget "http://download.geofabrik.de/$CONTINENT/$COUNTRY/$REGION.poly" -O$CONTINENT/$COUNTRY/$REGION/$REGION.poly
        exitCode=$?
        if [ $exitCode -ne 0 ] ; then
            echo "Error"
            exit $exitCode
        fi
        wget "http://download.geofabrik.de/$CONTINENT/$COUNTRY/$REGION-latest.osm.pbf" -O$CONTINENT/$COUNTRY/$REGION/$REGION-latest.osm.pbf
        exitCode=$?
        if [ $exitCode -ne 0 ] ; then
            echo "Error"
            exit $exitCode
        fi
        md5sum < $CONTINENT/$COUNTRY/$REGION/$REGION-latest.osm.pbf| cut -c 1-32 > $file2
        truncate --size -1 $file2 # Todo find better way to remove the new line
        printf "  $REGION-latest.osm.pbf\n" >> $file2
        if ! cmp --silent "$file1" "$file2"; then
            echo "md5 wrong download failed abort script"
            exit 99
        fi
        echo "map download done"
    else
        echo "try download"
        wget "http://download.geofabrik.de/$CONTINENT/$COUNTRY.poly" -O$CONTINENT/$COUNTRY/$COUNTRY.poly
        exitCode=$?
        if [ $exitCode -ne 0 ] ; then
            echo "Error"
            exit $exitCode
        fi
        wget "http://download.geofabrik.de/$CONTINENT/$COUNTRY-latest.osm.pbf" -O$CONTINENT/$COUNTRY/$COUNTRY-latest.osm.pbf
        exitCode=$?
        if [ $exitCode -ne 0 ] ; then
            echo "Error"
            exit $exitCode
        fi
        md5sum < $CONTINENT/$COUNTRY/$COUNTRY-latest.osm.pbf| cut -c 1-32 > $file2
        truncate --size -1 $file2 # Todo find better way to remove the new line
        printf "  $COUNTRY-latest.osm.pbf\n" >> $file2
        if ! cmp --silent "$file1" "$file2"; then
            echo "md5 wrong download failed abort script"
            exit 99
        fi
        echo "map download done"
    fi
else
    echo "map data up to date"
fi

echo "begin generate contour lines"

# Later implement

echo "starting osmconvert"

# Later implement

echo "deleting tmp files"

if [ ! -z "$REGION" ]; then
    rm tmp/$CONTINENT-$COUNTRY-$REGION*.osm
else
    rm tmp/$CONTINENT-$COUNTRY.osm
fi

#$BASEDIR/$CONTINENT/$COUNTRY/$REGION/$REGION-contours-$RESTEXT.osm.pbf \

if [ ! -z "$REGION" ]; then
    time nice $OSMSCOUTDIR/Import \
        -d \
        --eco true \
        --typefile $BASEDIR/map.ost \
        --rawCoordBlockSize $(( 60 * 1000000 )) \
        --rawWayBlockSize $(( 4 * 1000000 )) \
        --altLangOrder en \
        --destinationDirectory "$CONTINENT/$COUNTRY/$REGION" \
        --bounding-polygon $CONTINENT/$COUNTRY/$REGION/$REGION.poly \
        $IMPORT_ARGS \
        $BASEDIR/$CONTINENT/$COUNTRY/$REGION/$REGION-latest.osm.pbf \
        2>&1 | tee "$BASEDIR/$CONTINENT/$COUNTRY/$REGION/$REGION-import.log"
else
    time nice $OSMSCOUTDIR/Import \
        -d \
        --eco true \
        --typefile $BASEDIR/map.ost \
        --rawCoordBlockSize $(( 60 * 1000000 )) \
        --rawWayBlockSize $(( 4 * 1000000 )) \
        --altLangOrder en \
        --destinationDirectory "$CONTINENT/$COUNTRY" \
        --bounding-polygon $CONTINENT/$COUNTRY/$COUNTRY.poly \
        $IMPORT_ARGS \
        $BASEDIR/$CONTINENT/$COUNTRY/$COUNTRY-latest.osm.pbf \
        2>&1 | tee "$BASEDIR/$CONTINENT/$COUNTRY/$COUNTRY-import.log"
fi