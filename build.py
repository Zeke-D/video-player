import subprocess

SOURCE = ["./src/*.c", "./include/glad/glad.c"]

INCLUDE  = ["C:\\Users\\Zeke\\Tools\\FFmpeg\\", "./include/", "./include/SDL/./include/"]
for i in range(0, len(INCLUDE) * 2, 2):
    INCLUDE.insert(i, "-I")

ERROR_FLAGS = ["-Wall", "-Werror", "-Wextra"]
C_FLAGS = ["-std=c99", "-g"] # + ["-D", "MINGW"]

# load libraries
LIBRARIES = ["lib", "lib/FFmpeg/", "./include/SDL/build/Release/"]
for i in range(0, len(LIBRARIES) * 2, 2):
    LIBRARIES.insert(i, "-L")


LD_FLAGS = ["SDL3", "avcodec", "avformat", "avutil", "swscale", "mingw32", "opengl32"]
for i in range(len(LD_FLAGS)):
    LD_FLAGS[i] = "-l" + LD_FLAGS[i]


BUILD_DIR = "build"
OUTPUT_NAME = BUILD_DIR + "/tut.exe"

COMPILER = "gcc"
COMPILE_COMMAND = [COMPILER] + SOURCE + ERROR_FLAGS + C_FLAGS + INCLUDE + LIBRARIES + LD_FLAGS + ["-o", OUTPUT_NAME]

for command in ["rm " + OUTPUT_NAME, "cls", COMPILE_COMMAND]:
    print(" ".join(command))
    subprocess.run(command, shell=True)
