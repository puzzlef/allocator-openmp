#!/usr/bin/env bash
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --ntasks-per-node=1
#SBATCH --cpus-per-task=64
#SBATCH --exclusive
#SBATCH --job-name slurm
#SBATCH --output=slurm.out
# source scl_source enable gcc-toolset-11
# module load hpcx-2.7.0/hpcx-ompi
# module load openmpi/4.1.5
src="allocator-openmp"
out="$HOME/Logs/$src.log"
ulimit -s unlimited
printf "" > "$out"

# Download program
if [[ "$DOWNLOAD" != "0" ]]; then
  rm -rf $src
  git clone https://github.com/puzzlef/$src
fi
cd $src

# Fixed config
: "${MAX_THREADS:=64}"
: "${REPEAT_METHOD:=1}"
# Define macros (dont forget to add here)
DEFINES=(""
"-DMAX_THREADS=$MAX_THREADS"
"-DREPEAT_METHOD=$REPEAT_METHOD"
)

# Compile
g++ ${DEFINES[*]} -std=c++17 -march=native -O3 -fopenmp main.cxx  # -Wall -Wno-unknown-pragmas

# Run on all inputs
runAll() {
stdbuf --output=L ./a.out  2>&1 | tee -a "$out"
}

# Run 5 times
for i in {1..5}; do
  runAll
done

# Signal completion
curl -X POST "https://maker.ifttt.com/trigger/puzzlef/with/key/${IFTTT_KEY}?value1=$src$1"
