TEMPLATE = app
TARGET   = raymini
CONFIG  += qt opengl xml warn_on console release thread
QT *= opengl xml
HEADERS = Window.h \
          GLViewer.h \
          QTUtils.h \
          Vertex.h \
          Triangle.h \
          Mesh.h \
          BoundingBox.h \
          Material.h \
          Object.h \
          Light.h \
          Scene.h \
          RayTracer.h \
          Ray.h \
    	  Vec3D.h \
          KDTree.h \
	  Node.h

SOURCES = Window.cpp \
          GLViewer.cpp \
          QTUtils.cpp \
          Vertex.cpp \
          Triangle.cpp \
          Mesh.cpp \
          BoundingBox.cpp \
          Material.cpp \
          Object.cpp \
          Light.cpp \
          Scene.cpp \ 
          RayTracer.cpp \
          Ray.cpp \
          Main.cpp \
          KDTree.cpp \
	  Node.cpp

    DESTDIR=.

win32 {
    INCLUDEPATH += 'C:\Users\plequ_000\projects\computer-graphics\extern\libQGLViewer-2.3.17'
    LIBS += -L"C:\Users\plequ_000\projects\computer-graphics\extern\libQGLViewer-2.3.17\QGLViewer\release" \
        -lQGLViewer2 \
        -lglu32 \
        -lopengl32 \
        -lglew32
}
unix {
    LIBS += -lGLEW \
        -lQGLViewer \
	-lGLU
}

MOC_DIR = .tmp
OBJECTS_DIR = .tmp

