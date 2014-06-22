objs = ggpplayer.o client.o dependgraph.o montecarloplayer.o node.o prover.o reader.o relation.o statemachine.o
ggp : $(objs) 
	g++ -o ggp $(objs)
ggpplayer.o : reader.h relation.h prover.h dependgraph.h statemachine.h montecarloplayer.h client.h config.h
	g++ -c -std=c++11 ggpplayer.cpp
client.o : client.h reader.h relation.h 
	g++ -c -std=c++11 client.cpp
dependgraph.o : dependgraph.h relation.h reader.h
	g++ -c -std=c++11 dependgraph.cpp
montecarloplayer.o : montecarloplayer.h dependgraph.h node.h client.h
	g++ -c -std=c++11 montecarloplayer.cpp
node.o : node.h client.h dependgraph.h
	g++ -c -std=c++11 node.cpp
prover.o : prover.h relation.h reader.h dependgraph.h 
	g++ -c -std=c++11 prover.cpp
reader.o : reader.h relation.h
	g++ -c -std=c++11 reader.cpp
relation.o : relation.h
	g++ -c -std=c++11 relation.cpp
statemachine.o : statemachine.h prover.h relation.h client.h
	g++ -c -std=c++11 statemachine.cpp
clean:
	rm ggp $(objs)
