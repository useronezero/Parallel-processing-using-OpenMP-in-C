#!/bin/bash -l

### COMP328/COMP528 OpenMP lab/s
### example batch script
###           usage: "sbatch run.sh -c N SRC.c"
###         purpose: to compile given input file, SRC.c, and run on requested number of threads, N

### (c) mkbane, University of Liverpool (2020)

# Specific course queue and max wallclock time
#SBATCH -p course -t 5

# Defaults on Barkla (but set to be safe)
## Specify the current working directory as the location for executables/files
#SBATCH -D ./
## Export the current environment to the compute node
#SBATCH --export=ALL

# load module for intel compiler
module load compilers/intel/2019u5 

if [[ $# -ne 1 ]]; then
    echo
    echo ERROR
    echo Usage: sbatch -c NUM_THREADS_TO_USE $0 NAME_OF_INPUT.c
    exit -1
fi


# SLURM terms
## nodes            relates to number of nodes
## ntasks-per-node  relates to MPI processes per node
## cpus-per-task    relates to OpenMP threads (per MPI process)

# determine number of cores requested (NB this is single node implementation)
## further options available via examples: /opt/apps/Slurm_Examples/sbatch*sh
echo "Node list                    : $SLURM_JOB_NODELIST"
echo "Number of nodes allocated    : $SLURM_JOB_NUM_NODES or $SLURM_NNODES"
echo "Number of threads or processes          : $SLURM_NTASKS"
echo "Number of processes per node : $SLURM_TASKS_PER_NODE"
echo "Requested tasks per node     : $SLURM_NTASKS_PER_NODE"
echo "Requested CPUs per task      : $SLURM_CPUS_PER_TASK"
echo "Scheduling priority          : $SLURM_PRIO_PROCESS"

# check expected inputs (OpenMP is only supported on a single node)
if [ "$SLURM_NNODES" -gt "1" ]; then 
    echo more than 1 node not allowed
    exit
fi


# parallel using OpenMP
SRC=$1
EXE=${SRC%%.c}.exe
rm -f ${EXE}
echo compiling $SRC to $EXE
icc -qopenmp -O0 $SRC  -o $EXE 
if test -x $EXE; then
      # set number of threads
      export OMP_NUM_THREADS=${SLURM_CPUS_PER_TASK:-1} # if '-c' not used then default to 1
      echo using ${OMP_NUM_THREADS} OpenMP threads
      # run code one times
      ./${EXE};echo '---'
else
     echo $SRC did not built to $EXE
fi
