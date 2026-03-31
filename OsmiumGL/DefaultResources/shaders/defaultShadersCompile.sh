pushd $(dirname $0)
for file in *.vert; do
  echo "$file"
  glslc "$file" -o "$file.spv"
done
for file in *.frag; do
  echo "$file"
  glslc "$file" -o "$file.spv"
done
popd