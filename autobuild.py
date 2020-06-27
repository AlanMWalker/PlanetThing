import os

# Path to MSBuild.exe which handles invoking compiling & linking
msBuildPath = '"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\MSBuild\\Current\\Bin\\MsBuild.exe"'

# Clean the build 
returnVal = os.system(msBuildPath + ' .\\PlantThing.sln /p:Configuration=Debug -t:CPPClean')
returnVal = os.system(msBuildPath + ' .\\PlanetThing.sln /p:Configuration=Release -t:CPPClean')

print("Building debug version of game")

returnVal = os.system(msBuildPath + ' .\\PlanetThing.sln /p:Configuration=Debug')
if returnVal >= 1:
    exit(1)

print("Building release version of game")
returnVal = os.system(msBuildPath + ' .\\PlanetThing.sln /p:Configuration=Release')

if returnVal >= 1:
    exit(1)


exit(0)
