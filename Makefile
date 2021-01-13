progname = Ryuutai

builddir = build
srcdir = src

libs = -Llib -lGL -lGLEW -lglfw

#imguidir = ${srcdir}/external/imgui

cppflags =  -std=c++11 -Iinclude 
#cppflags += -I${imguidir} -I${imguidir}/backends -DIMGUI_IMPL_OPENGL_LOADER_GLEW

sourcefiles =  ${srcdir}/*.cpp
#sourcefiles += ${imguidir}/imgui.cpp ${imguidir}/imgui_draw.cpp ${imguidir}/imgui_widgets.cpp
#sourcefiles += ${imguidir}/backends/imgui_impl_glfw.cpp ${imguidir}/backends/imgui_impl_opengl3.cpp

makebuilddir= if [ ! -e ${builddir} ];then mkdir ${builddir};fi

.PHONY: build debug clean

default: build

build:
	${makebuilddir}
	g++ -O2 ${cppflags} -o ${builddir}/${progname} ${sourcefiles} ${libs}
	printf "\nTo run:\n\t./run.sh\n\n"

debug:
	${makebuilddir}
	g++ -g -fno-eliminate-unused-debug-symbols ${cppflags} -o ${builddir}/${progname}_debug ${sourcefiles} ${libs}
	printf "\nTo debug :\n\tgdb ${builddir}/${progname}_debug\n\n"

clean:
	rm build/*
