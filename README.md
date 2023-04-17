# Reaction Time  

## Compiling  

`emcc reactionTime.cpp -o index.html --shell-file template.html -s USE_SDL=2 -s use_SDL_GFX=2 -s NO_EXIT_RUNTIME=1 -s "Exported_RUNTIME_METHOS=['ccall']"`