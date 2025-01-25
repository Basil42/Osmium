//
// Created by Shadow on 1/19/2025.
//

#ifndef CONFIG_H
#define CONFIG_H
//keeping these numbers very low for now
#define MAX_FRAMES_IN_FLIGHT 2
#define MAX_LOADED_MATERIALS 10
#define MAX_LOADED_MATERIAL_INSTANCES 50
#define MAX_LOADED_MESHES 255

//built in scene descriptors
#define BUILT_IN_DESCRIPTOR_POOL_SIZE_COUNT 1//it should be possible to know this at compile time even with custom descriptors

#endif //CONFIG_H
