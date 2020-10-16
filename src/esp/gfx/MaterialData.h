// Copyright (c) Facebook, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#ifndef ESP_GFX_MATERIALDATA_H_
#define ESP_GFX_MATERIALDATA_H_

#include <Magnum/GL/Texture.h>
#include <Magnum/Magnum.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Matrix3.h>

#include "esp/core/esp.h"

namespace esp {
namespace gfx {

enum class MaterialDataType {
  /** @brief default, does not represent any material */
  None = 0,
  /** @brief phong material data */
  Phong = 1,
  /** @brief Physically based rendering (PBR) material data */
  Pbr = 2,
};

struct MaterialData {
  MaterialData(MaterialDataType _type = MaterialDataType::None) : type(_type){};

  MaterialDataType type{MaterialDataType::None};

  bool perVertexObjectId = false;

  // construct it using the default constructor. NO initial values, such as
  // identity matrix
  Magnum::Matrix3 textureMatrix;
};

struct PhongMaterialData : public MaterialData {
  PhongMaterialData() : MaterialData(MaterialDataType::Phong){};

  Magnum::Float shininess = 80.f;
  Magnum::Color4 ambientColor{0.1};
  Magnum::Color4 diffuseColor{0.7};
  Magnum::Color4 specularColor{0.2};
  Magnum::GL::Texture2D *ambientTexture = nullptr, *diffuseTexture = nullptr,
                        *specularTexture = nullptr, *normalTexture = nullptr;
  bool vertexColored = false;

  ESP_SMART_POINTERS(PhongMaterialData)
};

struct PbrMaterialData : public MaterialData {
  PbrMaterialData() : MaterialData(MaterialDataType::Pbr){};

  // material with default magic values:
  // when both the material property and the texture (e.g., roughness and
  // roughness texture) are NOT presented, use these default magic values;
  Magnum::Color4 baseColor{0.7f};
  Magnum::Float roughness = 0.9f;
  Magnum::Float metallic = 0.1f;
  Magnum::Color3 emissiveColor{0.0f};
  Magnum::Float normalTextureScale = 1.0f;

  Magnum::GL::Texture2D* baseColorTexture = nullptr;
  Magnum::GL::Texture2D* normalTexture = nullptr;
  Magnum::GL::Texture2D* emissiveTexture = nullptr;

  // Question:
  // What if e.g., both metallicTexture and nonRoughnessMEtallicTexture exist?
  // Answer:
  // Such conflict will be handled in the PbrShader based on the priority of the
  // textures. See generateCorrectFlags() in PbrShader for details.

  // an independent, NOT packed texture to store "metallic" (in R
  // channel)
  Magnum::GL::Texture2D* metallicTexture = nullptr;
  // an independent, NOT packed texture to store "roughness" (in R channel)
  Magnum::GL::Texture2D* roughnessTexture = nullptr;
  // a packed texture to store "roughness" (in G channel) and "metallic" (in B
  // channel)
  Magnum::GL::Texture2D* noneRoughnessMetallicTexture = nullptr;
  // a packed texture to store "occlusion"(in R channel), "roughness" (in G
  // channel) and "metallic" (in B channel)
  Magnum::GL::Texture2D* occlusionRoughnessMetallicTexture = nullptr;
  // TODO:
  // AO texture, emisive texture
  ESP_SMART_POINTERS(PbrMaterialData)
};

// TODO: Ptex material data

}  // namespace gfx
}  // namespace esp

#endif  // ESP_GFX_MATERIALDATA_H_
