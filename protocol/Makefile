all: Network_Connect.png Network_Disconnect.png Network_Move.png

clean:
  rm *.png

%.png:
	dot -Tpng -o $*.png $*.gv;
