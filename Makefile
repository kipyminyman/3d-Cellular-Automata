WIN_PROGRAM_NAME = cellautoma3d.exe
PROGRAM_NAME = cellautoma3d

win:
	g++ *.cpp -o $(WIN_PROGRAM_NAME) -I"C:\WinCPP\include" -L"C:\WinCPP\lib" -std=c++11 -lsfml-system -lsfml-graphics -lsfml-window -lglew32 -lfreeglut -lopengl32

lin:
	g++ *.cpp -std=c++0x -o $(PROGRAM_NAME) -lsfml-system -lsfml-graphics -lsfml-window -lGLEW -lGLU -lGL -lglut -lstdc++

clean:
	rm $(PROGRAM_NAME) $(WIN_PROGRAM_NAME)
