SRC=DeliveryOptimizer.cpp DeliveryPlanner.cpp PointToPointRouter.cpp StreetMap.cpp main.cpp testmain.cpp
CXX=g++
FLAGS= -std=c++14
EXEC=goober

$(EXEC): $(SRC)
	$(CXX) $(SRC) -o $(EXEC) $(FLAGS)

clean:
	rm -rf $(EXEC)
