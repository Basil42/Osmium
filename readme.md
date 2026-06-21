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

Many of these system are design from first principles as an exercice here are cursory breakdowns for the important ones
### Synchronization

The engine has some parallelism, managed through SyncUtils.h.
A high level breakdown:
- Rendering runs on it's own thread and syncs with the render update step
- game loop and render data update run on the same thread sequentially (render update waits for previous frame to complete rendering)
- 2 threads manage loading and unloading assets
- If in editor mode, ImGui rendering happens mostly on its own thread and sync with a rendering step to submit the data

A few notes:
 - Game data writes are all thread safe inside the game loop.
 - If there is a need to write game data from another thread (after a load for example), other thread can queue callbacks to be processed on top of the game loop. This means than chaining loads through callback delays response by one frame per callback chained.
 - Imgui render and render update being readonly operation (with changes through editor being piped to the game loop queue) they can run in parrallel.
 - Rendering is a purely read-only step when it comes to CPU data. 
 - The different libraries have loose dependencies in that they know which process they need to wait for through SyncUtils.

Sync is done by signaling SyncUtils when a task is finished, this increase a timeline semaphore style counter and signal potential tasks that can wait on the completion of the task for this frame.
This system only require the tasks to keep a frame counter to compare to the semaphore counter.

### Render update

The render update send an update to the renderer to swap the data collection it reads from, alternating between two collections (one read, one write).
Then, the update applies the changes that were made to the previous write collection to the new one before the game loop can run its tick (at the end of the render update, the write and read collections are identical in content and layout).

### Game loop operation queue

On top of the simulation loop, the engine processes all function calls requested by other threads. This is mostly used to respond to assets loading.
There is a dedicated queue to operate on a game object taking a matching function pointer and a gameobject pointer. (Game objects are contained in a memory stable way, so unless they are destroyed, pointers to game object are never invalid)

It would be trivial to add additional queues (I just haven't needed them so far)

### Resource Arrays

The engine uses RessourceArray in several place to contain data that changes often and must be traversed often.
Adding something in a RessourceArray returns a handle that will stay valid over the lifetime of the data and can be used to access the entry.
Because the handles can be reused if an entry is removed and then a new one added, they should have a single owner.

The Data itself is stored in a std::vector, and can be access sequentially (RessourceArray provides the interface to use foreach loops on it)

In short, it is a std::vector with stable pseudo pointers in the form of handles.

### Render Data

#### Rendered objects
Object data are stored in RessourceArrays of draw data (texture map indices, model matrix and other things of that nature), with one arrays per mesh, stored in a map using the mesh handle as keys.
#### Lights
Lights are represented on CPU by a single RessourceArray (duplicated for synchronization purposes) per Light type (point, directional or spot), as they share the same light shape mesh, if they use one.
This data is pushed to GPU every frame through pushconstants and are not stored on the VRAM.

#### Meshes
Meshes are stored on GPU naively (1 vertex buffer and 1 indice buffer per mesh).
The graphics library keeps track of them through a Ressource array that contains buffer handles and their respective sizes.
In the core engine , mesh are purely represented by a simple handle (it would be easy to extend the API to let user query data about the meshes).


### Render loop
The renderer uses deferred lighting and does not support several material (but exposes albedo and specular maps for each object).
It uses fairly modern Vulkan paradigms:
- Dynamic rendering
- Bindless texture ressources
- timeline semaphores
Here is a short breakdown of the various passes
#### Normal and specular pass
This pass renders the normal vector and specular power of all geometry to a framebuffer and writes to the depth buffer.
#### Light passes
- Render light shapes without face culling or depth testing to limit the number of light shader invocations (don't do a full screen pass for a tiny point light in the distance).
- Reconstruct position and normal of geometry from the normal and specular buffer and depth.
- compute light contribution and store in accumulation buffers (one for diffuse, one for specular)
#### shading pass
Second geometry pass that compute final shading from the light acculumation buffers (also reuses the depth buffer to discard fragment faster).
This pass outputs to swapchain if ImGui is not enabled (not runnning in editor mode).
#### Optional _ ImGui

If ImGui is enabled, it will pass the output framebuffer of the shading pass to ImGui to be rendered in the viewport.

### Componenent inspectors

To add an inspector to a component, the user declares a specialization of RenderGameObjectComponentInspector<>() and add
```static const bool registered_MyComponent = registerType<MyComponent>("MyComponent name");``` to the source file (outside any function body).
This will register the function to a table during static initilization. Unfortunately the componenent must have a valid copy constructor and =(&&MyComponent) operator, which must not be invoked in other context.

### Collision
Collision are checked for at the end of the Gameloop, when a collision is detected the component will call OnContactStart if there was no contact with this collider last frame, then it will call OnContactOngoing if it was notified this frame that it still overlapped the given collider, other it calls OnContactEnd.
Collision response are very simple (just a text notification)

## Known issue
Creating a new gameobject sometimes put existing gameobject name string in an invalid state that can cause segfault if access by functions (except ImGui, somehow)