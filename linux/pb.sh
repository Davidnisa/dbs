SRC_DIR=../code-proc
DST_DIR=../code/pb

if [ -d "$DST_DIR" ]; then
	rm -rf "$DST_DIR"/*
else
	mkdir "$DST_DIR"
fi

for file in $SRC_DIR/*
do
	if test -d $file
	then
		for cfile in $file/*
		do
			if test -f $cfile
			then
				protoc -I=$SRC_DIR --cpp_out=$DST_DIR $cfile
			fi
		done	
	fi
done
