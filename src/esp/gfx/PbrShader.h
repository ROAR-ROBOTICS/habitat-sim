// Copyright (c) Facebook, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#ifndef ESP_GFX_PBRSHADER_H_
#define ESP_GFX_PBRSHADER_H_

#include <initializer_list>
#include <vector>

#include <Corrade/Containers/ArrayView.h>
#include <Corrade/Containers/EnumSet.h>
#include <Magnum/GL/AbstractShaderProgram.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/Shaders/Generic.h>

#include "esp/core/esp.h"

namespace esp {

namespace gfx {

class PbrShader : public Magnum::GL::AbstractShaderProgram {
 public:
  // ==== Attribute definitions ====
  /**
   * @brief vertex positions
   */
  typedef Magnum::Shaders::Generic3D::Position Position;

  /**
   * @brief normal direction
   */
  typedef Magnum::Shaders::Generic3D::Normal Normal;

  /**
   * @brief 2D texture coordinates
   *
   * Used only if at least one of
   * @ref Flag::BaseColorTexture, @ref Flag::NormalTexture and
   * @ref Flag::RoughnessTexture @ref Flag::MetallicTexture is set.
   */
  typedef Magnum::Shaders::Generic3D::TextureCoordinates TextureCoordinates;

  /**
   * @brief Tangent direction with the fourth component indicating the handness.
   *
   * T = Tangent, B = BiTangent, N = Normal
   *
   * 1.0 means T, B, N form a right-handed coordinate;
   * -1.0 means T, B, N form a left-handed coordinate;
   *
   * Used only if @ref Flag::NormalTexture is set.
   */
  typedef Magnum::Shaders::Generic3D::Tangent4 Tangent4;

  enum : Magnum::UnsignedInt {
    /**
     * Color shader output. @ref shaders-generic "Generic output",
     * present always. Expects three- or four-component floating-point
     * or normalized buffer attachment.
     */
    ColorOutput = 0,

    /**
     * Object ID shader output. @ref shaders-generic "Generic output",
     * present only if @ref Flag::ObjectId is set. Expects a
     * single-component unsigned integral attachment. Writes the value
     * set in @ref setObjectId() there.
     */
    ObjectIdOutput = 1,
  };

  /**
   * @brief Flag
   *
   * @see @ref Flags, @ref flags()
   */
  enum class Flag : Magnum::UnsignedShort {
    /**
     * Multiply base color with the baseColor texture.
     * @see @ref setBaseColor(), @ref bindBaseColorTexture()
     */
    BaseColorTexture = 1 << 0,

    /**
     * Multiply roughness with the roughness texture.
     * This flag term means the roughness texture is independent, and
     * "roughness" is stored in the R channel of it.
     * NOTE:
     * if NoneRoughnessMetallicTexture or OcclusionRoughnessMetallicTexture are
     * presented, this texture will be ignored.
     * @see @ref setRoughness(), @ref bindRoughnessTexture()
     */
    RoughnessTexture = 1 << 1,

    /**
     * Multiply metallic with the metallic texture.
     * This flag term means the metallic texture is independent, and "metalness"
     * is stored in the R channel of it.
     * NOTE:
     * if NoneRoughnessMetallicTexture or OcclusionRoughnessMetallicTexture are
     * presented, this texture will be ignored.
     * @see @ref setMetallic(), @ref bindMetallicTexture()
     */
    MetallicTexture = 1 << 2,

    /*
     * Roughness and Metalness are packed together in one texture, with
     * Roughness taking G channel and metalness occupying B channel.
     * R and Alpha channels are not used
     * NOTE:
     * if OcclusionRoughnessMetallicTexture is presented, this texture will be
     * ignored.
     */
    NoneRoughnessMetallicTexture = 1 << 3,
    /*
     * Occlusion, Roughness and Metalness are packed together in one texture,
     * with Occlusion in R channel, Roughness in G channel and metalness in B
     * channel. Alpha channels is not used
     */
    OcclusionRoughnessMetallicTexture = 1 << 4,

    /**
     * Modify normals according to a texture. Requires the
     * @ref Tangent4 attribute to be present.
     */
    NormalTexture = 1 << 5,

    /**
     * Enable normal texture scale
     * the shader expects that
     * @ref Flag::NormalTexture is enabled as well.
     * @see @ref setNormalTextureScale
     */
    NormalTextureScale = 1 << 6,

    /**
     * Enable texture coordinate transformation. If this flag is set,
     * the shader expects that at least one of
     * @ref Flag::BaseColorTexture, @ref Flag::RoughnessTexture,
     * @ref Flag::MetallicTexture, @ref Flag::NormalTexture,
     * @ref Flag::NoneRoughnessMetallicTexture or
     * @ref Flag::OcclusionRoughnessMetallicTexture is enabled as well.
     * @see @ref setTextureMatrix()
     */
    TextureTransformation = 1 << 7,

    /**
     * TODO: Do we need instanced object? (instanced texture, istanced id etc.)
     */

    /**
     * TODO: Do we need VertexColor?
     * Multiply diffuse color with a vertex color. Requires either
     * the @ref Color3 or @ref Color4 attribute to be present.
     */

    /*
     * Precomputed tangent as the vertex attribute
     * Otherwise, it will be computed in the fragement shader dynamically
     * see PBR fragement shader code for more details
     */
    PrecomputedTangent = 1 << 10,

    /**
     * Enable object ID output.
     */
    ObjectId = 1 << 11,

    /*
     * TODO: alphaMask
     */
  };

  /**
   * @brief Flags
   */
  typedef Corrade::Containers::EnumSet<Flag> Flags;

  static Flags generateCorrectFlags(Flags originalFlags);

  /**
   * @brief Constructor
   * @param flags         Flags
   * @param lightCount    Count of light sources
   */
  explicit PbrShader(Flags flags = {}, unsigned int lightCount = 1);

  /** @brief Copying is not allowed */
  PbrShader(const PbrShader&) = delete;

  /** @brief Move constructor */
  PbrShader(PbrShader&&) noexcept = default;

  /** @brief Copying is not allowed */
  PbrShader& operator=(const PbrShader&) = delete;

  /** @brief Move assignment */
  PbrShader& operator=(PbrShader&&) noexcept = default;

  /**
   * @brief Get number of lights
   */
  unsigned int lightCount() const { return lightCount_; }

  /** @brief Flags */
  Flags flags() const { return flags_; }

  // ======== texture binding ========
  /**
   * @brief Bind the BaseColor texture
   * @return Reference to self (for method chaining)
   */
  PbrShader& bindBaseColorTexture(Magnum::GL::Texture2D* texture);
  /**
   * @brief Bind the roughness texture
   * @return Reference to self (for method chaining)
   */
  PbrShader& bindRoughnessTexture(Magnum::GL::Texture2D* texture);
  /**
   * @brief Bind the metallic texture
   * @return Reference to self (for method chaining)
   */
  PbrShader& bindMetallicTexture(Magnum::GL::Texture2D* texture);
  /**
   * @brief Bind the normal texture
   * @return Reference to self (for method chaining)
   */
  PbrShader& bindNormalTexture(Magnum::GL::Texture2D* texture);
  /**
   * @brief Bind the BaseColor, roughness, metallic, noneRoughnessMetallic,
   * occlusionRoughnessMetallic normal textures
   * NOTE:
   * can pass nullptr to any input argument
   * @return Reference to self (for method chaining)
   */
  PbrShader& bindTextures(Magnum::GL::Texture2D* baseColor,
                          Magnum::GL::Texture2D* roughness,
                          Magnum::GL::Texture2D* metallic,
                          Magnum::GL::Texture2D* noneRoughnessMetallic,
                          Magnum::GL::Texture2D* occlusionRoughnessMetallic,
                          Magnum::GL::Texture2D* normal);

  /**
   * @brief Bind the noneRoughnessMetallicTexture or
   * occlusionRoughnessMetallicTexture
   * @return Reference to self (for method chaining)
   */
  PbrShader& bindCombinedTexture(Magnum::GL::Texture2D* texture);

  PbrShader& setTextureMatrix(const Magnum::Matrix3& matrix);
  // ======== set uniforms ===========
  /**
   *  @brief Set "modelview and projection" matrix to the uniform on GPU
   *         MVP = proj * view * model
   *  @return Reference to self (for method chaining)
   */
  PbrShader& setMVPMatrix(const Magnum::Matrix4& matrix);

  /**
   *  @brief Set modelview matrix to the uniform on GPU
   *         modelview = view * model
   *  @return Reference to self (for method chaining)
   */
  PbrShader& setTransformationMatrix(const Magnum::Matrix4& matrix);

  /**
   *  @brief Set normal matrix to the uniform on GPU
   *         normal = inverse transpose of the up-left 3x3 matrix of the
   *         modelview matrix
   *  @return Reference to self (for method chaining)
   */
  PbrShader& setNormalMatrix(const Magnum::Matrix3& matrix);

  // -------- materials ---------------
  /**
   *  @brief Set base color to the uniform on GPU
   *  @return Reference to self (for method chaining)
   */
  PbrShader& setBaseColor(const Magnum::Color4& color);
  /**
   *  @brief Set roughness to the uniform on GPU
   *  @return Reference to self (for method chaining)
   */
  PbrShader& setRoughness(float roughness);
  /**
   *  @brief Set metallic to the uniform on GPU
   *  @return Reference to self (for method chaining)
   */
  PbrShader& setMetallic(float metallic);

  /**
   *  @brief Set object id to the uniform on GPU
   *  @return Reference to self (for method chaining)
   */
  PbrShader& setObjectId(unsigned int objectId);

  /**
   * @brief Set light positions or directions
   * @param vectors, an array of the light vectors
   * @return Reference to self (for method chaining)
   *
   * when vec.w == 0, it means vec.xyz is the light direction;
   * when vec.w == 1, it means vec.xyz is the light position;
   * vec is an element in the "vectors" array
   */
  PbrShader& setLightVectors(
      Corrade::Containers::ArrayView<const Magnum::Vector4> vectors);

  /**
   * @overload
   */
  PbrShader& setLightVectors(std::initializer_list<Magnum::Vector4> vectors);

  /**
   *  @brief Set the position or direction of a specific light See @ref vec for
   *  details
   *  @param lightIndex, the index of the light, MUST be smaller than
   *                     lightCount_
   *  @param vec, the direction (or position) of the light in *camera* space;
   *              when vec.w == 0, it means vec.xyz is the light direction;
   *              when vec.w == 1, it means vec.xyz is the light position;
   *  @return Reference to self (for method chaining)
   *  Note:
   *  If the light was a directional (point) light, it will be overrided as a
   *  point (directional) light
   */
  PbrShader& setLightVector(unsigned int lightIndex,
                            const Magnum::Vector4& vec);

  /**
   *  @brief Set the position of a specific light.
   *  @param lightIndex, the index of the light, MUST be smaller than
   * lightCount_
   *  @param pos, the position of the light in *camera* space
   *  @return Reference to self (for method chaining)
   *  Note:
   *  If the light was a directional light, it will be overrided as a point
   *  light;
   */
  PbrShader& setLightPosition(unsigned int lightIndex,
                              const Magnum::Vector3& pos);

  /**
   *  @brief Set the direction of a specific light.
   *  @param lightIndex, the index of the light, MUST be smaller than
   * lightCount_
   *  @param dir, the direction of the light in *camera* space
   *  @return Reference to self (for method chaining)
   *  NOTE:
   *  If the light was a point light, it will be overrided as a direction
   * light;
   */
  PbrShader& setLightDirection(unsigned int lightIndex,
                               const Magnum::Vector3& dir);

  /**
   *  @brief Set the range of a specific light.
   *  @param lightIndex, the index of the light, MUST be smaller than
   * lightCount_
   *  @param range, the range of the light
   *  @return Reference to self (for method chaining)
   */
  PbrShader& setLightRange(unsigned int lightIndex, float range);

  /**
   *  @brief Set the color of a specific light.
   *  @param lightIndex, the index of the light, MUST be smaller than
   * lightCount_
   *  @param color, the color of the light
   *  @param intensity, the intensity of the light
   *  @return Reference to self (for method chaining)
   */
  PbrShader& setLightColor(unsigned int lightIndex,
                           const Magnum::Vector3& color,
                           float intensity = 1.0);

  /**
   *  @brief Set the colors of the lights
   *  @param color, the colors of the lights
   *  NOTE: the intensity MUST be included in the color
   *  @return Reference to self (for method chaining)
   */
  PbrShader& setLightColors(
      Corrade::Containers::ArrayView<const Magnum::Color3> colors);

  /**
   * @overload
   */
  PbrShader& setLightColors(std::initializer_list<Magnum::Color3> colors);

  /**
   *  @brief Set the ranges of the lights
   *  @param ranges, the ranges of the lights
   *  @return Reference to self (for method chaining)
   */
  PbrShader& setLightRanges(Corrade::Containers::ArrayView<const float> ranges);

  /**
   * @overload
   */
  PbrShader& setLightRanges(std::initializer_list<float> ranges);

  PbrShader& setNormalTextureScale(float scale);

 protected:
  Flags flags_;
  unsigned int lightCount_;

  // ======= uniforms =======
  // it hurts the performance to call glGetUniformLocation() every frame due
  // to string operations. therefore, cache the locations in the constructor
  // material uniforms
  int modelviewMatrixUniform_ = ID_UNDEFINED;
  int normalMatrixUniform_ = ID_UNDEFINED;
  int mvpMatrixUniform_ = ID_UNDEFINED;
  int baseColorUniform_ = ID_UNDEFINED;  // diffuse color
  int roughnessUniform_ = ID_UNDEFINED;  // roughness of a surface
  int metallicUniform_ = ID_UNDEFINED;
  int baseColorTextureUniform_ = ID_UNDEFINED;
  int roughnessTextureUniform_ = ID_UNDEFINED;
  int metallicTextureUniform_ = ID_UNDEFINED;
  int normalTextureUniform_ = ID_UNDEFINED;
  int objectIdUniform_ = ID_UNDEFINED;
  int textureMatrixUniform_ = ID_UNDEFINED;
  int normalTextureScaleUniform_ = ID_UNDEFINED;

  int lightColorsUniform_ = ID_UNDEFINED;
  int lightRangesUniform_ = ID_UNDEFINED;
  // In the fragement shader, the "LightDirection" is a vec4.
  // when w == 0, it means .xyz is the light direction;
  // when w == 1, it means it is the light position, NOT the direction;
  int lightDirectionsUniform_ = ID_UNDEFINED;
};

CORRADE_ENUMSET_OPERATORS(PbrShader::Flags)

}  // namespace gfx
}  // namespace esp

#endif  // ESP_GFX_PBRSHADER_H_
