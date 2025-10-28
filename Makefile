all : 
	g++ *.cpp -o parakeet -std=c++17 -O3 -Wall -Wextra -pedantic -I/usr/include -I/usr/include/hdf5/serial -L/usr/lib/x86_64-linux-gnu/hdf5/serial -lhdf5 -lhdf5_cpp -I/usr/include/boost -lssl -lcrypto -lpthread -pthread -lboost_system -lboost_thread -lboost_filesystem -I/usr/local/include -L/usr/local/lib -lwt -lwthttp -lwtdbo -lwtdbosqlite3 -lsqlite3

init_db : 
	g++ init/initDatabase.cpp init/User.cpp init/File.cpp -o initDatabase -std=c++17 -O3 -Wall -Wextra -pedantic -I/usr/include -I/usr/include/boost -lssl -lcrypto -lpthread -pthread -lboost_system -lboost_thread -lboost_filesystem -I/usr/local/include -L/usr/local/lib -lwt -lwthttp -lwtdbo -lwtdbosqlite3 -lsqlite3

clean :
	rm parakeet
