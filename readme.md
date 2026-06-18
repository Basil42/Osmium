# Osmium
A toy Vulkan based engine made for educational purposes.

# Architecture

## Libraries
Osmium different libraries segregate functionalities but are not really fully independant (except the renderer , that is more or less independent)

### Common
Contains shared functionalities shared between the different libraries, most notably:
 - Serialization functions and mesh and texture serializer
 - Synchronization utilities (using crossGUID as an external dependency)
 - The main resource container class

### Core

Contains the core structure of the engine:
- Game objects
- Components
- Asset loader and manager
- Game loop
- Responsible for starting the renderer

### Editor
Contains the editor, it is responsible for starting the Core library and contains:
- A Game object hierachy used to add and remove game object from the scene
- An inspector that lets the user add and remove component of the selected game object and change exposed values on these
- Functions to draw component inspectors

### Renderer
Contains all Vulkan code used to render the scene. It currently support
- Blinn-phong deffered rendering.
- Configurable Point and Directional lights and Spotlights
- User provided Smoothness, Specular and Albedo maps for objects.
- User provided Meshes for object (currently only from .obj file)
It uses Volk as a third party dependency

### Physics
Contains all collision logic

## Systems

### Synchronization

The engine has some parallelism, managed through SyncUtils.h.
A high level breakdown:
- Rendering runs on it's own thread and syncs with the render update step
- game loop and render data update run on the same thread sequentially (render update waits for previous frame to complete rendering)
- 2 threads manage loading and unloading assets
- If in editor mode, ImGui rendering happens mostly on its own thread and sync with a rendering step to submit the data

```mermaid
    info
```

A few notes:
 - Game data writes are all thread safe inside the game loop.
 - If there is a need to write game data from another thread (after a load for example), other thread can queue callbacks to be processed on top of the game loop. This means than chaining loads through callback delays response by one frame per callback chained.
 - Imgui render and render update being readonly operation (with changes through editor being piped to the game loop queue) they can run in parrallel.
 - Rendering is a purely read-only step when it comes to CPU data.
