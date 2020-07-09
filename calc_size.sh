# Calculates lines of code across project
find . -name '*.h' -not -path './include/box2d/*'  -not -path './include/imgui/*' -not -path './src/box2d/*'| xargs wc -l | sort -nr > header_total.txt
find . -name '*.cpp' -not -path './src/box2d/*' -not -path './include/imgui/*' | xargs wc -l | sort -nr > source_total.txt
echo "Finished calculating, check source_total.txt & header_total.txt"