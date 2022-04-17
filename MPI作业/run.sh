g++ mpi_pi.cpp -o a.out -fopenmp -l msmpi -L "C:\Program Files (x86)\Microsoft SDKs\MPI\Lib\x64" -I "C:\Program Files (x86)\Microsoft SDKs\MPI\Include"
&&
mpiexec -n 4 a.out