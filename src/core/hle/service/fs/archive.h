// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/unordered_map.hpp>
#include "common/common_types.h"
#include "core/file_sys/archive_backend.h"
#include "core/hle/result.h"
#include "core/hle/service/fs/directory.h"
#include "core/hle/service/fs/file.h"

/// The unique system identifier hash, also known as ID0
static constexpr char SYSTEM_ID[]{"00000000000000000000000000000000"};
/// The scrambled SD card CID, also known as ID1
static constexpr char SDCARD_ID[]{"00000000000000000000000000000000"};

namespace Loader {
class AppLoader;
}

namespace Core {
class System;
}

namespace Service::FS {

/// Supported archive types
enum class ArchiveIdCode : u32 {
    SelfNCCH = 0x00000003,
    SaveData = 0x00000004,
    ExtSaveData = 0x00000006,
    SharedExtSaveData = 0x00000007,
    SystemSaveData = 0x00000008,
    SDMC = 0x00000009,
    SDMCWriteOnly = 0x0000000A,
    NCCH = 0x2345678A,
    OtherSaveDataGeneral = 0x567890B2,
    OtherSaveDataPermitted = 0x567890B4,
};

/// Media types for the archives
enum class MediaType : u32 { NAND = 0, SDMC = 1, GameCard = 2 };

MediaType GetMediaTypeFromPath(std::string_view path);

enum class SpecialContentType : u8 {
    Update = 1,
    Manual = 2,
    DLPChild = 3,
};

typedef u64 ArchiveHandle;

struct ArchiveResource {
    u32 sector_size_in_bytes;
    u32 cluster_size_in_bytes;
    u32 partition_capacity_in_clusters;
    u32 free_space_in_clusters;
};
static_assert(sizeof(ArchiveResource) == 0x10, "ArchiveResource has incorrect size");

using FileSys::ArchiveBackend;
using FileSys::ArchiveFactory;

class ArchiveManager {
public:
    explicit ArchiveManager(Core::System& system);

    /**
     * Opens an archive
     * @param id_code IdCode of the archive to open
     * @param archive_path Path to the archive, used with Binary paths
     * @param program_id the program ID of the client that requests the operation
     * @return Handle to the opened archive
     */
    ResultVal<ArchiveHandle> OpenArchive(ArchiveIdCode id_code, FileSys::Path& archive_path,
                                         u64 program_id);

    /**
     * Closes an archive
     * @param handle Handle to the archive to close
     */
    ResultCode CloseArchive(ArchiveHandle handle);

    /**
     * Open a File from an Archive
     * @param archive_handle Handle to an open Archive object
     * @param path Path to the File inside of the Archive
     * @param mode Mode under which to open the File
     * @return Tuple of the opened File object and the open delay
     */
    std::tuple<ResultVal<std::shared_ptr<File>>, std::chrono::nanoseconds> OpenFileFromArchive(
        ArchiveHandle archive_handle, const FileSys::Path& path, const FileSys::Mode mode);

    /**
     * Delete a File from an Archive
     * @param archive_handle Handle to an open Archive object
     * @param path Path to the File inside of the Archive
     * @return Whether deletion succeeded
     */
    ResultCode DeleteFileFromArchive(ArchiveHandle archive_handle, const FileSys::Path& path);

    /**
     * Rename a File between two Archives
     * @param src_archive_handle Handle to the source Archive object
     * @param src_path Path to the File inside of the source Archive
     * @param dest_archive_handle Handle to the destination Archive object
     * @param dest_path Path to the File inside of the destination Archive
     * @return Whether rename succeeded
     */
    ResultCode RenameFileBetweenArchives(ArchiveHandle src_archive_handle,
                                         const FileSys::Path& src_path,
                                         ArchiveHandle dest_archive_handle,
                                         const FileSys::Path& dest_path);

    /**
     * Delete a Directory from an Archive
     * @param archive_handle Handle to an open Archive object
     * @param path Path to the Directory inside of the Archive
     * @return Whether deletion succeeded
     */
    ResultCode DeleteDirectoryFromArchive(ArchiveHandle archive_handle, const FileSys::Path& path);

    /**
     * Delete a Directory and anything under it from an Archive
     * @param archive_handle Handle to an open Archive object
     * @param path Path to the Directory inside of the Archive
     * @return Whether deletion succeeded
     */
    ResultCode DeleteDirectoryRecursivelyFromArchive(ArchiveHandle archive_handle,
                                                     const FileSys::Path& path);

    /**
     * Create a File in an Archive
     * @param archive_handle Handle to an open Archive object
     * @param path Path to the File inside of the Archive
     * @param file_size The size of the new file, filled with zeroes
     * @return File creation result code
     */
    ResultCode CreateFileInArchive(ArchiveHandle archive_handle, const FileSys::Path& path,
                                   u64 file_size);

    /**
     * Create a Directory from an Archive
     * @param archive_handle Handle to an open Archive object
     * @param path Path to the Directory inside of the Archive
     * @return Whether creation of directory succeeded
     */
    ResultCode CreateDirectoryFromArchive(ArchiveHandle archive_handle, const FileSys::Path& path);

    /**
     * Rename a Directory between two Archives
     * @param src_archive_handle Handle to the source Archive object
     * @param src_path Path to the Directory inside of the source Archive
     * @param dest_archive_handle Handle to the destination Archive object
     * @param dest_path Path to the Directory inside of the destination Archive
     * @return Whether rename succeeded
     */
    ResultCode RenameDirectoryBetweenArchives(ArchiveHandle src_archive_handle,
                                              const FileSys::Path& src_path,
                                              ArchiveHandle dest_archive_handle,
                                              const FileSys::Path& dest_path);

    /**
     * Open a Directory from an Archive
     * @param archive_handle Handle to an open Archive object
     * @param path Path to the Directory inside of the Archive
     * @return The opened Directory object
     */
    ResultVal<std::shared_ptr<Directory>> OpenDirectoryFromArchive(ArchiveHandle archive_handle,
                                                                   const FileSys::Path& path);

    /**
     * Get the free space in an Archive
     * @param archive_handle Handle to an open Archive object
     * @return The number of free bytes in the archive
     */
    ResultVal<u64> GetFreeBytesInArchive(ArchiveHandle archive_handle);

    /**
     * Erases the contents of the physical folder that contains the archive
     * identified by the specified id code and path
     * @param id_code The id of the archive to format
     * @param format_info Format information about the new archive
     * @param path The path to the archive, if relevant.
     * @param program_id the program ID of the client that requests the operation
     * @return ResultCode 0 on success or the corresponding code on error
     */
    ResultCode FormatArchive(ArchiveIdCode id_code, const FileSys::ArchiveFormatInfo& format_info,
                             const FileSys::Path& path, u64 program_id);

    /**
     * Retrieves the format info about the archive of the specified type and path.
     * The format info is supplied by the client code when creating archives.
     * @param id_code The id of the archive
     * @param archive_path The path of the archive, if relevant
     * @param program_id the program ID of the client that requests the operation
     * @return The format info of the archive, or the corresponding error code if failed.
     */
    ResultVal<FileSys::ArchiveFormatInfo> GetArchiveFormatInfo(ArchiveIdCode id_code,
                                                               FileSys::Path& archive_path,
                                                               u64 program_id);

    /**
     * Creates a blank SharedExtSaveData archive for the specified extdata ID
     * @param media_type The media type of the archive to create (NAND / SDMC)
     * @param high The high word of the extdata id to create
     * @param low The low word of the extdata id to create
     * @param smdh_icon the SMDH icon for this ExtSaveData
     * @param format_info Format information about the new archive
     * @param program_id the program ID of the client that requests the operation
     * @return ResultCode 0 on success or the corresponding code on error
     */
    ResultCode CreateExtSaveData(MediaType media_type, u32 high, u32 low,
                                 const std::vector<u8>& smdh_icon,
                                 const FileSys::ArchiveFormatInfo& format_info, u64 program_id);

    /**
     * Deletes the SharedExtSaveData archive for the specified extdata ID
     * @param media_type The media type of the archive to delete (NAND / SDMC)
     * @param high The high word of the extdata id to delete
     * @param low The low word of the extdata id to delete
     * @return ResultCode 0 on success or the corresponding code on error
     */
    ResultCode DeleteExtSaveData(MediaType media_type, u32 high, u32 low);

    /**
     * Deletes the SystemSaveData archive folder for the specified save data id
     * @param high The high word of the SystemSaveData archive to delete
     * @param low The low word of the SystemSaveData archive to delete
     * @return ResultCode 0 on success or the corresponding code on error
     */
    ResultCode DeleteSystemSaveData(u32 high, u32 low);

    /**
     * Creates the SystemSaveData archive folder for the specified save data id
     * @param high The high word of the SystemSaveData archive to create
     * @param low The low word of the SystemSaveData archive to create
     * @return ResultCode 0 on success or the corresponding code on error
     */
    ResultCode CreateSystemSaveData(u32 high, u32 low);

    /**
     * Returns capacity and free space information about the given media type.
     * @param media_type The media type to obtain the information about.
     * @return The capacity information of the media type, or an error code if failed.
     */
    ResultVal<ArchiveResource> GetArchiveResource(MediaType media_type) const;

    /// Registers a new NCCH file with the SelfNCCH archive factory
    void RegisterSelfNCCH(Loader::AppLoader& app_loader);

private:
    Core::System& system;

    /**
     * Registers an Archive type, instances of which can later be opened using its IdCode.
     * @param factory File system backend interface to the archive
     * @param id_code Id code used to access this type of archive
     */
    ResultCode RegisterArchiveType(std::unique_ptr<FileSys::ArchiveFactory>&& factory,
                                   ArchiveIdCode id_code);

    /// Register all archive types
    void RegisterArchiveTypes();

    ArchiveBackend* GetArchive(ArchiveHandle handle);

    /**
     * Map of registered archives, identified by id code. Once an archive is registered here, it is
     * never removed until UnregisterArchiveTypes is called.
     */
    std::unordered_map<ArchiveIdCode, std::unique_ptr<ArchiveFactory>> id_code_map;

    /**
     * Map of active archive handles to archive objects
     */
    std::unordered_map<ArchiveHandle, std::unique_ptr<ArchiveBackend>> handle_map;
    ArchiveHandle next_handle = 1;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int) {
        ar& id_code_map;
        ar& handle_map;
        ar& next_handle;
    }
    friend class boost::serialization::access;
};

} // namespace Service::FS
