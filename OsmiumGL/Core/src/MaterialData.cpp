//
// Created by nicolas.gerard on 2025-01-21.
//
#include "MaterialData.h"

MaterialData::~MaterialData() {
    //should be cleaned out by then
    if (!instances->GetCount()) {
        std::cout << "an instance of material data is getting cleaned up with instance resources still aloccated, this is likely causing a memory leak" << std::endl;
    }
    delete instances;
}

MaterialData::MaterialData(): pipeline(nullptr), pipelineLayout(nullptr), descriptorSetLayout(nullptr),
                              PushConstantStride(0) {
    instances = new ResourceArray<MaterialInstanceData,MAX_LOADED_MATERIAL_INSTANCES>();
}
