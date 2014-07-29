objs = ggpplayer.o client.o dependgraph.o montecarloplayer.o node.o prover.o reader.o relation.o statemachine.o component.o combination.o propnet.o
ggp : $(objs) 
	g++ -o ggp $(objs)
ggpplayer.o : ggpplayer.cpp reader.h relation.h prover.h dependgraph.h statemachine.h montecarloplayer.h client.h config.h
	g++ -O3 -c -std=c++11 ggpplayer.cpp
client.o : client.cpp client.h reader.h relation.h montecarloplayer.h prover.h
	g++ -O3 -c -std=c++11 client.cpp
dependgraph.o : dependgraph.cpp dependgraph.h relation.h reader.h
	g++ -O3 -c -std=c++11 dependgraph.cpp
montecarloplayer.o : montecarloplayer.cpp montecarloplayer.h dependgraph.h node.h client.h statemachine.h prover.h
	g++ -O3 -c -std=c++11 montecarloplayer.cpp
node.o : node.cpp node.h client.h dependgraph.h reader.h
	g++ -O3 -c -std=c++11 node.cpp
prover.o : prover.cpp prover.h relation.h reader.h dependgraph.h combination.h
	g++ -O3 -c -std=c++11 prover.cpp
reader.o : reader.cpp reader.h relation.h
	g++ -O3 -c -std=c++11 reader.cpp
relation.o : relation.cpp relation.h
	g++ -O3 -c -std=c++11 relation.cpp
statemachine.o : statemachine.cpp statemachine.h prover.h relation.h client.h propnet.h
	g++ -O3 -c -std=c++11 statemachine.cpp
propnet.o : propnet.cpp component.h relation.h prover.h combination.h
	g++ -O3 -c -std=c++11 propnet.cpp
combination.o : combination.cpp 
	g++ -O3 -c -std=c++11 combination.cpp
component.o : component.cpp
	g++ -O3 -c -std=c++11 component.cpp
clean:
	rm ggp $(objs)
