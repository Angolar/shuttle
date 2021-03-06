#!/bin/bash

the_dir=`dirname "$0"`
the_dir=`cd "$the_dir"; pwd`

nfs_dir=/disk/shuttle
nfs_path=~/Documents

declare -a input_files=( )
declare -a expand_inputs=( )

if [[ $1 == *streaming ]]; then
    files=()
    file_detected=0
    packname=mapred_job_default
    for opt in $@; do
        if [ "$file_detected" = "1" ]; then
            files=(${files[@]} $opt)
            file_detected=0
        fi
        if [ "$opt" = "-file" -o "$opt" = "--file" ]; then
            file_detected=1
        fi
        if [[ $opt = mapred.job.name=* ]]; then
            packname=${opt:16}
        fi
    done
    unset file_detected

    timestamp=`date +%F`
    pack_dirname=$packname-`date +%s`
    mkdir $pack_dirname
    packname=${pack_dirname}".tar.gz"
    if [ ${#files[*]} -gt 0 ]; then
        cp -rf ${files[@]} ${pack_dirname} 
        if [ $? -ne 0 ]; then
            echo 'file options contains inexist file'
            exit -1
        fi
    fi
    tar -czvf "$packname" -C $pack_dirname . >& /dev/null

    $nfs_path/NfsShell mkdir $nfs_dir/$timestamp
    $nfs_path/NfsShell put --override $packname $nfs_dir/$timestamp

    params=( "$@" )
    for i in `seq $#`
    do
        if [ "${params[i]}" = "-input" ]; then
            hdfs_file_path="${params[$((i+1))]}"
            input_files=( "${input_files[@]}" $hdfs_file_path )
            unset params[$i]
            unset params[$((i+1))]
        fi
    done
    for i in `seq $#`; do
        if [ "${params[i]}" = "-file" -o "${params[i]}" = "--file" ]; then
            unset params[$i]
            unset params[$((i+1))]
        fi
    done
    
    set -- "${params[@]}"
    file_param=-file\ $timestamp/$packname

    rm -rf $packname $pack_dirname
fi

source $the_dir/shuttle.conf 2> /dev/null

if [ "$nexus_cluster" ]; then
    nexus_param=-nexus\ $nexus_cluster
else
    if [ `ls -A "$the_dir"/ins.flag 2> /dev/null` ]; then
        nexus_param=-nexus-file\ $the_dir/ins.flag
    fi
fi

if [ "$nexus_root" ]; then
    nexus_root=-nexus-root\ $nexus_root
fi

for file_path in "${input_files[@]}" 
do
    declare -a exp_path=( $(eval "echo $file_path") )
    for epp in "${exp_path[@]}"
    do
        expand_inputs=( "${expand_inputs[@]}" "-input ${epp}" )
    done
done

$the_dir/shuttle $nexus_param $nexus_root $file_param \
    -jobconf mapred.job.input.host=$input_host \
    -jobconf mapred.job.input.port=$input_port \
    -jobconf mapred.job.input.user=$input_user \
    -jobconf mapred.job.input.password=$input_password \
    -jobconf mapred.job.output.host=$output_host \
    -jobconf mapred.job.output.port=$output_port \
    -jobconf mapred.job.output.user=$output_user \
    -jobconf mapred.job.output.password=$output_password \
    "$@" \
    ${expand_inputs[@]}

