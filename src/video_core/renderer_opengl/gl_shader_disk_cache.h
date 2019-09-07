// Copyright 2019 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <bitset>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <glad/glad.h>

#include "common/assert.h"
#include "common/common_types.h"
#include "common/vfs/vfs_vector.h"
#include "video_core/regs.h"
#include "video_core/renderer_opengl/gl_shader_decompiler.h"
#include "video_core/renderer_opengl/gl_shader_gen.h"

namespace Core {
class System;
}

namespace FileUtil {
class IOFile;
}

namespace OpenGL {

struct ShaderDiskCacheDecompiled;
struct ShaderDiskCacheDump;

using RawShaderConfig = Pica::Regs;
using ProgramCode = std::vector<u32>;
using ShaderDecompiledMap = std::unordered_map<u64, ShaderDiskCacheDecompiled>;
using ShaderDumpsMap = std::unordered_map<u64, ShaderDiskCacheDump>;

/// Describes a shader how it's used by the guest GPU
class ShaderDiskCacheRaw {
public:
    explicit ShaderDiskCacheRaw(u64 unique_identifier, ProgramType program_type,
                                RawShaderConfig config, ProgramCode program_code);
    ShaderDiskCacheRaw();
    ~ShaderDiskCacheRaw();

    bool Load(FileUtil::IOFile& file);

    bool Save(FileUtil::IOFile& file) const;

    u64 GetUniqueIdentifier() const {
        return unique_identifier;
    }

    ProgramType GetProgramType() const {
        return program_type;
    }

    const ProgramCode& GetProgramCode() const {
        return program_code;
    }

    const RawShaderConfig& GetRawShaderConfig() const {
        return config;
    }

private:
    u64 unique_identifier{};
    ProgramType program_type{};
    RawShaderConfig config{};
    ProgramCode program_code{};
};

/// Contains decompiled data from a shader
struct ShaderDiskCacheDecompiled {
    ShaderDecompiler::ProgramResult code;
};

/// Contains an OpenGL dumped binary program
struct ShaderDiskCacheDump {
    GLenum binary_format;
    std::vector<u8> binary;
};

class ShaderDiskCache {
public:
    explicit ShaderDiskCache(bool separable);
    ~ShaderDiskCache();

    /// Loads transferable cache. If file has a old version or on failure, it deletes the file.
    std::optional<std::vector<ShaderDiskCacheRaw>> LoadTransferable();

    /// Loads current game's precompiled cache. Invalidates on failure.
    std::pair<ShaderDecompiledMap, ShaderDumpsMap> LoadPrecompiled();

    /// Removes the transferable (and precompiled) cache file.
    void InvalidateTransferable();

    /// Removes the precompiled cache file and clears virtual precompiled cache file.
    void InvalidatePrecompiled();

    /// Saves a raw dump to the transferable file. Checks for collisions.
    void SaveRaw(const ShaderDiskCacheRaw& entry);

    /// Saves a decompiled entry to the precompiled file. Does not check for collisions.
    void SaveDecompiled(u64 unique_identifier, const ShaderDecompiler::ProgramResult& code);

    /// Saves a dump entry to the precompiled file. Does not check for collisions.
    void SaveDump(u64 unique_identifier, GLuint program);

    /// Serializes virtual precompiled shader cache file to real file
    void SaveVirtualPrecompiledFile();

private:
    /// Loads the transferable cache. Returns empty on failure.
    std::optional<std::pair<ShaderDecompiledMap, ShaderDumpsMap>> LoadPrecompiledFile(
        FileUtil::IOFile& file);

    /// Loads a decompiled cache entry from m_precompiled_cache_virtual_file. Returns empty on
    /// failure.
    std::optional<ShaderDiskCacheDecompiled> LoadDecompiledEntry();

    /// Saves a decompiled entry to the passed file. Returns true on success.
    bool SaveDecompiledFile(u64 unique_identifier, const ShaderDecompiler::ProgramResult& code);

    /// Returns if the cache can be used
    bool IsUsable() const;

    /// Opens current game's transferable file and write it's header if it doesn't exist
    FileUtil::IOFile AppendTransferableFile();

    /// Save precompiled header to precompiled_cache_in_memory
    void SavePrecompiledHeaderToVirtualPrecompiledCache();

    /// Create shader disk cache directories. Returns true on success.
    bool EnsureDirectories() const;

    /// Gets current game's transferable file path
    std::string GetTransferablePath();

    /// Gets current game's precompiled file path
    std::string GetPrecompiledPath();

    /// Get user's transferable directory path
    std::string GetTransferableDir() const;

    /// Get user's precompiled directory path
    std::string GetPrecompiledDir() const;

    /// Get user's shader directory path
    std::string GetBaseDir() const;

    /// Get current game's title id as u64
    u64 GetProgramID();

    /// Get current game's title id
    std::string GetTitleID();

    template <typename T>
    bool SaveArrayToPrecompiled(const T* data, std::size_t length) {
        const std::size_t write_length = precompiled_cache_virtual_file.WriteArray(
            data, length, precompiled_cache_virtual_file_offset);
        precompiled_cache_virtual_file_offset += write_length;
        return write_length == sizeof(T) * length;
    }

    template <typename T>
    bool LoadArrayFromPrecompiled(T* data, std::size_t length) {
        const std::size_t read_length = precompiled_cache_virtual_file.ReadArray(
            data, length, precompiled_cache_virtual_file_offset);
        precompiled_cache_virtual_file_offset += read_length;
        return read_length == sizeof(T) * length;
    }

    template <typename T>
    bool SaveObjectToPrecompiled(const T& object) {
        return SaveArrayToPrecompiled(&object, 1);
    }

    bool SaveObjectToPrecompiled(bool object) {
        const auto value = static_cast<u8>(object);
        return SaveArrayToPrecompiled(&value, 1);
    }

    template <typename T>
    bool LoadObjectFromPrecompiled(T& object) {
        return LoadArrayFromPrecompiled(&object, 1);
    }

    // Stores whole precompiled cache which will be read from or saved to the precompiled chache
    // file
    Common::VectorVfsFile precompiled_cache_virtual_file;
    // Stores the current offset of the precompiled cache file for IO purposes
    std::size_t precompiled_cache_virtual_file_offset = 0;

    // Stored transferable shaders
    std::unordered_map<u64, ShaderDiskCacheRaw> transferable;

    // The cache has been loaded at boot
    bool tried_to_load{};

    bool separable{};

    u64 program_id{};
    std::string title_id;
};

} // namespace OpenGL