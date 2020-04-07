#!/bin/python3
########################################
#           Compile Hydrogen           #
########################################
import os
print("Compiling Hydrogen")
os.chdir("./build")
os.system("ninja")
if (os.path.isfile("./Result.txt")):
    os.system("rm ./Result.txt")
if (os.path.isfile("./MVICFG.dot")):
    os.system("rm ./MVICFG.dot")
os.chdir("../")
########################################
#           Path Settings              #
########################################
demarcator = ":: "
pathToDir = "./TestPrograms/"
pathToModule1 = pathToDir + "Buggy/Prog.bc "
pathToModule2 = pathToDir + "Buggy2/Prog.bc "
pathToModule3 = pathToDir + "Correct/Prog.bc "
filesModule1 = ["Buggy/Prog.c"]
filesModule2 = ["Buggy2/Prog.c"]
filesModule3 = ["Correct/Prog.c"]
pathToFilesModule1 = ""
pathToFilesModule2 = ""
pathToFilesModule3 = ""
for file in filesModule1:
    pathToFilesModule1 = pathToFilesModule1 + os.path.abspath(pathToDir) + "/" + file + " "
for file in filesModule2:
    pathToFilesModule2 = pathToFilesModule2 + os.path.abspath(pathToDir) + "/" + file + " "
for file in filesModule3:
    pathToFilesModule3 = pathToFilesModule3 + os.path.abspath(pathToDir) + "/" + file + " "
pathAbs = os.path.abspath(pathToModule1) + os.path.abspath(pathToModule2) + os.path.abspath(
    pathToModule3) + demarcator + os.path.abspath(pathToFilesModule1) + demarcator + os.path.abspath(
        pathToFilesModule2) + demarcator + os.path.abspath(pathToFilesModule3)
print("Argument:\n" + pathAbs)
########################################
#         Running Hydrogen             #
########################################
print("Running Hydrogen")
os.chdir("./build")
hydrogenCall = "./Hydrogen.out " + pathAbs
os.system(hydrogenCall)
os.chdir("../")
########################################
#        Collecting Results            #
########################################
resultMove = "cp ./build/Result.txt " + pathToDir + "Result.txt"
os.system(resultMove)
resultMove = "cp ./build/MVICFG.dot " + pathToDir + "MVICFG.dot"
os.system(resultMove)
