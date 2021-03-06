// Copyright (c) Facebook, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#ifndef ESP_METADATA_MANAGERS_ATTRIBUTESMANAGERBASE_H_
#define ESP_METADATA_MANAGERS_ATTRIBUTESMANAGERBASE_H_

/** @file
 * @brief Class Template @ref esp::metadata::managers::AttributesManager
 */

#include "esp/metadata/attributes/AttributesBase.h"

#include "esp/core/ManagedContainer.h"
#include "esp/io/io.h"

namespace Cr = Corrade;

namespace esp {
namespace core {
class ManagedContainerBase;
}
namespace metadata {
namespace managers {

/**
 * @brief Class template defining responsibilities and functionality for
 * managing @ref esp::metadata::attributes::AbstractAttributes constructs.
 * @tparam T the type of managed attributes a particular specialization
 * of this class works with.  Must inherit from @ref
 * esp::metadata::attributes::AbstractAttributes.
 */
template <class T>
class AttributesManager : public esp::core::ManagedContainer<T> {
 public:
  static_assert(std::is_base_of<attributes::AbstractAttributes, T>::value,
                "AttributesManager :: Managed object type must be derived from "
                "AbstractAttributes");

  typedef std::shared_ptr<T> AttribsPtr;

  AttributesManager(const std::string& attrType, const std::string& JSONTypeExt)
      : esp::core::ManagedContainer<T>::ManagedContainer(attrType),
        JSONTypeExt_(JSONTypeExt) {}
  virtual ~AttributesManager() = default;

  /**
   * @brief Load all file-based templates given string list of template file
   * locations.
   *
   * This will take the list of file names specified and load the referenced
   * templates.  It is assumed these files are JSON files currently.
   * @param tmpltFilenames list of file names of templates
   * @param saveAsDefaults Set these templates as un-deletable from library.
   * @return vector holding IDs of templates that have been added
   */
  std::vector<int> loadAllFileBasedTemplates(
      const std::vector<std::string>& tmpltFilenames,
      bool saveAsDefaults);

  /**
   * @brief Load file-based templates for all @ref JSONTypeExt_
   * files from the provided file or directory path.
   *
   * This will take the passed @p path string and either treat it as a file
   * name or a directory, depending on what is found in the filesystem. If @p
   * path does not end with @ref JSONTypeExt_, it will append this and check to
   * see if such a file exists, and load it. It will also check if @p path
   * exists as a directory, and if so will perform a shallow search to find any
   * files ending in @ref JSONTypeExt_ and load those that are found.
   *
   * @param path A global path to configuration files or a directory containing
   * such files.
   * @param saveAsDefaults Set the templates loaded as undeleteable default
   * templates.
   * @return A list of template indices for loaded valid configs
   */
  std::vector<int> loadAllConfigsFromPath(const std::string& path,
                                          bool saveAsDefaults = false);

  /**
   * @brief This builds a list of paths to this type of attributes's file from a
   * JSON element.  It then will load all the configs it finds at each path.
   * @param configDir The directory to use as a root to search in - may be
   * different than the config already listed in this manager.
   * @param jsonPaths The json array element
   */

  void buildCfgPathsFromJSONAndLoad(const std::string& configDir,
                                    const io::JsonGenericValue& jsonPaths);

  /**
   * @brief Check if currently configured primitive asset template library has
   * passed handle.
   * @param handle String name of primitive asset attributes desired
   * @return whether handle exists or not in asset attributes library
   */
  virtual bool isValidPrimitiveAttributes(const std::string& handle) = 0;

  /**
   * @brief Parse passed JSON Document for @ref
   * esp::metadata::attributes::AbstractAttributes. It always returns a
   * valid @ref esp::metadata::attributes::AbstractAttributes shared
   * pointer.
   *
   * @param templateName The desired name for this @ref
   * esp::metadata::attributes::AbstractAttributes template.
   * @param jsonConfig json document to parse
   * @return a reference to the desired template.
   */
  virtual AttribsPtr buildObjectFromJSONDoc(
      const std::string& templateName,
      const io::JsonGenericValue& jsonConfig) override {
    // Construct a ObjectAttributes and populate with any
    // AbstractObjectAttributes fields found in json.
    auto attributes = this->initNewObjectInternal(templateName);
    // set the values for this attributes from the json config.
    this->setValsFromJSONDoc(attributes, jsonConfig);
    return attributes;
  }  // ObjectAttributesManager::buildObjectFromJSONDoc

  /**
   * @brief Method to take an existing attributes and set its values from passed
   * json config file.
   * @param attribs (out) an existing attributes to be modified.
   * @param jsonConfig json document to parse
   */
  virtual void setValsFromJSONDoc(AttribsPtr attribs,
                                  const io::JsonGenericValue& jsonConfig) = 0;

 protected:
  /**
   * @brief Called intenrally from createObject.  This will create either a file
   * based AbstractAttributes or a default one based on whether the passed file
   * name exists and has appropriate string tag/extension for @ref
   * esp::metadata::attributes::AbstractAttributes.
   *
   * @param filename the file holding the configuration of the object
   * @param msg reference to progress message
   * @param registerObj whether the new object should be registered in library
   * @return the create @ref esp::metadata::attributes::AbstractAttributes.
   */
  AttribsPtr createFromJsonOrDefaultInternal(const std::string& filename,
                                             std::string& msg,
                                             bool registerObj);

  // ======== Typedefs and Instance Variables ========
  /**
   * @brief The string extension for json files for this manager's attributes
   * types
   */
  const std::string JSONTypeExt_;

 public:
  ESP_SMART_POINTERS(AttributesManager<AttribsPtr>)

};  // class AttributesManager

/////////////////////////////
// Class Template Method Definitions
template <class T>
std::vector<int> AttributesManager<T>::loadAllFileBasedTemplates(
    const std::vector<std::string>& paths,
    bool saveAsDefaults) {
  std::vector<int> templateIndices(paths.size(), ID_UNDEFINED);
  for (int i = 0; i < paths.size(); ++i) {
    auto attributesFilename = paths[i];
    LOG(INFO) << "AttributesManager::loadAllFileBasedTemplates : Load "
              << this->objectType_ << " template: " << attributesFilename;
    auto tmplt = this->createObjectFromJSONFile(attributesFilename, true);

    // save handles in list of defaults, so they are not removed, if desired.
    if (saveAsDefaults) {
      std::string tmpltHandle = tmplt->getHandle();
      this->undeletableObjectNames_.insert(tmpltHandle);
    }
    templateIndices[i] = tmplt->getID();
  }
  LOG(INFO)
      << "AttributesManager::loadAllFileBasedTemplates : Loaded file-based "
      << this->objectType_ << " templates: " << std::to_string(paths.size());
  return templateIndices;
}  // AttributesManager<T>::loadAllObjectTemplates

template <class T>
std::vector<int> AttributesManager<T>::loadAllConfigsFromPath(
    const std::string& path,
    bool saveAsDefaults) {
  std::vector<std::string> paths;
  std::vector<int> templateIndices;
  namespace Directory = Cr::Utility::Directory;
  std::string attributesFilepath = path;
  if (!Cr::Utility::String::endsWith(attributesFilepath, this->JSONTypeExt_)) {
    attributesFilepath = path + "." + this->JSONTypeExt_;
  }
  const bool dirExists = Directory::isDirectory(path);
  const bool fileExists = Directory::exists(attributesFilepath);

  if (!dirExists && !fileExists) {
    LOG(WARNING) << "AttributesManager::loadAllConfigsFromPath : Parsing "
                 << this->objectType_ << " : Cannot find " << path << " or "
                 << attributesFilepath << ". Aborting parse.";
    return templateIndices;
  }

  if (fileExists) {
    paths.push_back(attributesFilepath);
  }

  if (dirExists) {
    LOG(INFO) << "AttributesManager::loadAllConfigsFromPath : Parsing "
              << this->objectType_ << " library directory: " + path;
    for (auto& file : Directory::list(path, Directory::Flag::SortAscending)) {
      std::string absoluteSubfilePath = Directory::join(path, file);
      if (Cr::Utility::String::endsWith(absoluteSubfilePath,
                                        this->JSONTypeExt_)) {
        paths.push_back(absoluteSubfilePath);
      }
    }
  }
  // build templates from aggregated paths
  templateIndices = this->loadAllFileBasedTemplates(paths, saveAsDefaults);

  return templateIndices;
}  // AttributesManager<T>::loadAllConfigsFromPath

template <class T>
void AttributesManager<T>::buildCfgPathsFromJSONAndLoad(
    const std::string& configDir,
    const io::JsonGenericValue& jsonPaths) {
  for (rapidjson::SizeType i = 0; i < jsonPaths.Size(); ++i) {
    if (!jsonPaths[i].IsString()) {
      LOG(ERROR)
          << "AttributesManager::buildCfgPathsFromJSONAndLoad : Invalid path "
             "value in configuration array element @ idx "
          << i << ". Skipping.";
      continue;
    }
    std::string absolutePath =
        Cr::Utility::Directory::join(configDir, jsonPaths[i].GetString());
    // load all object templates available as configs in absolutePath
    this->loadAllConfigsFromPath(absolutePath, true);
  }
  LOG(INFO) << "AttributesManager::buildCfgPathsFromJSONAndLoad : "
            << std::to_string(jsonPaths.Size())
            << " paths specified in JSON doc for " << this->objectType_
            << " templates.";
}  // AttributesManager<T>::buildCfgPathsFromJSONAndLoad

template <class T>
auto AttributesManager<T>::createFromJsonOrDefaultInternal(
    const std::string& filename,
    std::string& msg,
    bool registerObj) -> AttribsPtr {
  AttribsPtr attrs;
  std::string strHandle = Cr::Utility::String::lowercase(filename);
  std::string jsonAttrFileName =
      (strHandle.find(Cr::Utility::String::lowercase(this->JSONTypeExt_)) !=
       std::string::npos)
          ? std::string(filename)
          : io::changeExtension(filename, this->JSONTypeExt_);
  bool jsonFileExists = (this->isValidFileName(jsonAttrFileName));
  if (jsonFileExists) {
    // check if stageAttributesHandle corresponds to an actual, existing
    // json stage file descriptor.
    // this method lives in class template.
    attrs = this->createObjectFromJSONFile(jsonAttrFileName, registerObj);
    msg = "JSON File (" + jsonAttrFileName + ") Based";
  } else {
    // if name is not json file descriptor but still appropriate file, or if
    // is not a file .  Currently non-JSON files are treated as new, default
    // attributes.
    attrs = this->createDefaultObject(filename, registerObj);
    bool fileExists = (this->isValidFileName(filename));
    if (fileExists) {
      msg = "File (" + filename + ") not parsed, so new default";
    } else {
      msg = "New default (" + filename + ")";
    }
  }
  return attrs;
}  // AttributesManager<T>::createFromJsonFileOrDefaultInternal

}  // namespace managers
}  // namespace metadata
}  // namespace esp
#endif  // ESP_METADATA_MANAGERS_ATTRIBUTESMANAGERBASE_H_
