To build this program on Linux:

gcc main.c gl_frontEnd.c -lm -lGL -lglut -lpthread -o whateverName


To build on macOS, using the deprecated built-in GLUT framework

gcc main.c gl_frontEnd.c -lm -framework OpenGL -framework GLUT -lpthread -o whateverName
