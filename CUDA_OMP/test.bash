nvcc -v ../src/driver.cpp ../src/hgf.cpp ../src/hgfArrays.cpp ../src/hgfPP.cpp ../src/hgfMeshCu.cu ../src/hgfBC.cpp ../src/hgfIB.cpp ../src/hgfPoreNetwork.cpp ../src/hgfStokes.cpp -O3 -o hgfTest -Xcompiler -fopenmp -lcuda -lparalution -DCUDA_BUILD=1 -I../src/
