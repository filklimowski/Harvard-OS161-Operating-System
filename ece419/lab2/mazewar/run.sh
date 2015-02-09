#!/bin/bash
JAVA_HOME=/cad2/ece419s/java/jdk1.6.0/

# $1 = hostname of where MazeWarServer is located
# $2 = port # where MazeWarServer is listening

${JAVA_HOME}/bin/java Mazewar $1 $2
