/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb.h"
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <sys/statvfs.h>
#include <purefs/filesystem_paths.hpp>
#include <log/log.hpp>
#include <log/Logger.hpp>

#include "mtp_file_system_adapter.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DATE_YEAR_SHIFT  (9U)
#define DATE_YEAR_MASK   (0xFE00U)
#define DATE_MONTH_SHIFT (5U)
#define DATE_MONTH_MASK  (0x01E0U)
#define DATE_DAY_SHIFT   (0U)
#define DATE_DAY_MASK    (0x001FU)

#define TIME_HOUR_SHIFT   (11U)
#define TIME_HOUR_MASK    (0xF800U)
#define TIME_MINUTE_SHIFT (5U)
#define TIME_MINUTE_MASK  (0x07E0U)
#define TIME_SECOND_SHIFT (0U)
#define TIME_SECOND_MASK  (0x001FU)

extern "C" {

typedef struct
{
    FILE *file;
    uint8_t flags;
} usb_device_mtp_file_instance_t;

typedef struct
{
    DIR *dir;
    const uint16_t *dirPath;
    uint8_t flags;
} usb_device_mtp_dir_instance_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
// static usb_device_mtp_file_instance_t s_FileInstance[USB_DEVICE_MTP_FILE_INSTANCE];
// static usb_device_mtp_dir_instance_t s_DirInstance[USB_DEVICE_MTP_DIR_INSTANCE];

/*******************************************************************************
 * Code
 ******************************************************************************/

// static uint32_t USB_DeviceMtpAllocateFileHandle(void)
// {
//     uint32_t i;

//     for (i = 0; i < USB_DEVICE_MTP_FILE_INSTANCE; i++)
//     {
//         if (s_FileInstance[i].flags == 0U)
//         {
//             s_FileInstance[i].flags = 1U;
//             LOG_INFO("Allocated file handle: %lu", i);
//             return i;
//         }
//     }
//     LOG_ERROR("Failed to alocate file handle");
//     return (uint32_t)-1;
// }

// static usb_status_t USB_DeviceMtpFreeFileHandle(uint32_t i)
// {
//     if (s_FileInstance[i].flags != 0U)
//     {
//         s_FileInstance[i].flags = 0U;
//         s_FileInstance[i].file = NULL;
//         LOG_INFO("Freed file handle: %lu", i);
//         return kStatus_USB_Success;
//     }
//     LOG_ERROR("Failed to free file handle");
//     return kStatus_USB_Busy;
// }

// static uint32_t USB_DeviceMtpAllocateDirHandle(void)
// {
//     uint32_t i;

//     for (i = 0; i < USB_DEVICE_MTP_DIR_INSTANCE; i++)
//     {
//         if (s_DirInstance[i].flags == 0U)
//         {
//             s_DirInstance[i].flags = 1U;
//             LOG_INFO("Allocated dir handle: %lu", i);
//             return i;
//         }
//     }
//     LOG_ERROR("Failed to allocate dir handle");
//     return (uint32_t)-1;
// }

// static usb_status_t USB_DeviceMtpFreeDirHandle(uint32_t i)
// {
//     if (s_DirInstance[i].flags != 0U)
//     {
//         s_DirInstance[i].flags = 0U;
//         s_DirInstance[i].dir = NULL;
//         s_DirInstance[i].dirPath = NULL;
//         LOG_INFO("Freed file handle: %lu", i);
//         return kStatus_USB_Success;
//     }
//     LOG_ERROR("Failed to free dir handle");

//     return kStatus_USB_Busy;
// }

// static const uint16_t *USB_DeviceMtpGetDirPath(uint32_t i)
// {
//     if ((s_DirInstance[i].flags != 0U) && (s_DirInstance[i].dir))
//     {
//         LOG_INFO("Found dir handle to get path: %lu", i);
//         return s_DirInstance[i].dirPath;
//     }
//     LOG_ERROR("Failed to find dir path");

//     return NULL;
// }

// static usb_status_t USB_DeviceMtpSetDirPath(uint32_t i, const uint16_t *fileName)
// {
//     if ((s_DirInstance[i].flags != 0U) && (s_DirInstance[i].dir))
//     {
//         LOG_INFO("Found dir handle to set path: %lu", i);
//         s_DirInstance[i].dirPath = fileName;
//         return kStatus_USB_Success;
//     }
//     LOG_ERROR("Failed to set dir path");

//     return kStatus_USB_Busy;
// }

usb_status_t USB_DeviceMtpOpen(usb_device_mtp_file_handle_t *file, const uint16_t *fileName, uint32_t flags)
{
    FILE *fil;
    char mode[4] = {'r', 0, };
    uint32_t modeIdx = 0;

    // uint32_t fileSlot = USB_DeviceMtpAllocateFileHandle();

    LOG_INFO("File name: %s", (const uint8_t *)fileName);

    // if (fileSlot == -1)
    // {
    //     return kStatus_USB_Busy;
    // }

    // *file = (usb_device_mtp_file_handle_t)fil;

    // if ((flags & USB_DEVICE_MTP_READ) != 0U)
    // {
    //     mode |= FA_READ;
    // }
    if ((flags & USB_DEVICE_MTP_WRITE) != 0U)
    {
        mode[modeIdx++] = 'w';
    }
    if ((flags & USB_DEVICE_MTP_CREATE_NEW) != 0U)
    {
        mode[modeIdx++] = 'x';
    }
    if ((flags & USB_DEVICE_MTP_CREATE_ALWAYS) != 0U)
    {
        mode[modeIdx++] = '+';
    }

    LOG_INFO("Attempt open file: %s, mode %s", (const uint8_t *)fileName, mode);
    fil = fopen((const char*)fileName, mode);

    if (!fil)
    {
        // USB_DeviceMtpFreeFileHandle(fileSlot);
        LOG_ERROR("Failed to open file: %s", (const uint8_t *)fileName);
        return kStatus_USB_Error;
    }

    *file = (usb_device_mtp_file_handle_t)fil;

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpClose(usb_device_mtp_file_handle_t file)
{
    FILE *fil;

    if (file == NULL)
    {
        return kStatus_USB_InvalidHandle;
    }

    fil = (FILE *)file;

    if (!fclose(fil))
    {
        // USB_DeviceMtpFreeFileHandle(fil);
        return kStatus_USB_Error;
    }

    // USB_DeviceMtpFreeFileHandle(fil);
    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpLseek(usb_device_mtp_file_handle_t file, uint32_t offset)
{
    FILE *fil;

    if (file == NULL)
    {
        return kStatus_USB_InvalidHandle;
    }

    fil = (FILE *)file;

    if (fseek(fil, offset, SEEK_SET))
    {
        return kStatus_USB_Error;
    }

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpRead(usb_device_mtp_file_handle_t file, void *buffer, uint32_t size, uint32_t *actualsize)
{
    FILE *fil;

    if (file == NULL)
    {
        return kStatus_USB_InvalidHandle;
    }

    fil = (FILE *)file;

    LOG_INFO("Attempt read file: %lu", size);

    *actualsize = fread(buffer, 1, size, fil);

    if (*actualsize < size)
    {
        LOG_ERROR("Failed read file: %lu", *actualsize);
        return kStatus_USB_Error;
    }

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpWrite(usb_device_mtp_file_handle_t file, void *buffer, uint32_t size, uint32_t *actualsize)
{
    FILE *fil;

    if (file == NULL)
    {
        return kStatus_USB_InvalidHandle;
    }

    fil = (FILE *)file;

    LOG_INFO("Attempt write file: %lu", size);

    *actualsize = fwrite(buffer, 1, size, fil);

    if (*actualsize < size)
    {
        LOG_ERROR("Failed write file: %lu", *actualsize);
        return kStatus_USB_Error;
    }

    return kStatus_USB_Success;
}

#define IS_READONLY(mode) ((mode & (S_IWUSR | S_IWGRP | S_IWOTH)) == 0)
#define IS_DIRECTORY(mode) (mode & S_IFDIR)

usb_status_t USB_DeviceMtpFstat(const usb_device_mtp_dir_handle_t dir, const uint16_t *filePath, usb_device_mtp_file_info_t *fileInfo)
{
    struct stat fno;
    uint32_t count;
    uint8_t *src;
    uint16_t *dest;
    const char *file = (const char *)filePath;

    LOG_INFO("Attempt stat file: %s", file);

    if (fileInfo == NULL)
    {
        return kStatus_USB_InvalidParameter;
    }

    if (stat(file, &fno) != 0)
    {
        LOG_ERROR("Failed fstat file: %d", errno);
        return kStatus_USB_Error;
    }

    fileInfo->size = fno.st_size;

    struct tm *time = gmtime(&fno.st_mtime);

    fileInfo->dateUnion.dateBitField.year   = time->tm_year + 1900; // ((fno.st_mtime & DATE_YEAR_MASK) >> DATE_YEAR_SHIFT) + 1980U; /* Year origin from 1980 */
    fileInfo->dateUnion.dateBitField.month  = time->tm_mon + 1;  // (fno.st_ & DATE_MONTH_MASK) >> DATE_MONTH_SHIFT;
    fileInfo->dateUnion.dateBitField.day    = time->tm_mday; // (fno.st_mtime & DATE_DAY_MASK) >> DATE_DAY_SHIFT;
    fileInfo->timeUnion.timeBitField.hour   = time->tm_hour; // (fno.ftime & TIME_HOUR_MASK) >> TIME_HOUR_SHIFT;
    fileInfo->timeUnion.timeBitField.minute = time->tm_min;  // (fno.ftime & TIME_MINUTE_MASK) >> TIME_MINUTE_SHIFT;
    fileInfo->timeUnion.timeBitField.second = time->tm_sec;  // ((fno.ftime & TIME_SECOND_MASK) >> TIME_SECOND_SHIFT) << 1U; /* Second / 2 (0...29) */

    fileInfo->attrib = 0U;

    if (IS_DIRECTORY(fno.st_mode))
    {
        fileInfo->attrib |= USB_DEVICE_MTP_DIR;
    }
    if (IS_READONLY(fno.st_mode))
    {
        fileInfo->attrib |= USB_DEVICE_MTP_READ_ONLY;
    }

    /* copy file name, unicode encoding. */
    src   = (uint8_t *)strrchr(file, '/') + 1;
    dest  = (uint16_t *)&fileInfo->name[0];
    count = 0;

    while ((src[count] != 0U) || (src[count + 1U] != 0U) || ((count % 2U) != 0U))
    {
        dest[count] = src[count];
        count++;
    }

    dest[count]     = 0U; /* terminate with 0x00, 0x00 */

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpUtime(const uint16_t *fileName, usb_device_mtp_file_time_stamp_t *timeStamp)
{
    // struct stat fno;

    // fno.fdate = (WORD)((((WORD)timeStamp->year - 1980U) << 9U) | ((WORD)timeStamp->month << 5U) | (WORD)timeStamp->day);
    // fno.ftime =
    //     (WORD)(((WORD)timeStamp->hour << 11U) | ((WORD)timeStamp->minute * 32U) | ((WORD)timeStamp->second / 2U));

    // if (f_utime((const TCHAR *)fileName, &fno) != FR_OK)
    // {
    //     return kStatus_USB_Error;
    // }

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpOpenDir(usb_device_mtp_dir_handle_t *dir, const uint16_t *dirName)
{
    DIR *dir1;

    // if (USB_DeviceMtpAllocateDirHandle(&dir1) != kStatus_USB_Success)
    // {
    //     return kStatus_USB_Busy;
    // }

    LOG_INFO("Attempt open dir: %s", (const uint8_t *)dirName);

    dir1 = opendir((const char *)dirName);

    if (!dir1)
    {
        LOG_ERROR("Failed opend dir: %d", errno);
        // USB_DeviceMtpFreeDirHandle(dir1);
        return kStatus_USB_Error;
    }

    *dir = (usb_device_mtp_dir_handle_t)dir1;

    // USB_DeviceMtpSetDirPath(*dir, dirName);

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpCloseDir(usb_device_mtp_dir_handle_t dir)
{
    DIR *dir1;

    if (dir == NULL)
    {
        return kStatus_USB_InvalidHandle;
    }

    dir1 = (DIR *)dir;

    // USB_DeviceMtpSetDirPath(dir, NULL);

    if (closedir(dir1) != 0)
    {
        // USB_DeviceMtpFreeDirHandle(dir1);
        return kStatus_USB_Error;
    }

    // USB_DeviceMtpFreeDirHandle(dir1);

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpReadDir(usb_device_mtp_dir_handle_t dir, const uint16_t *dirName, usb_device_mtp_file_info_t *fileInfo)
{
    DIR *dir1;
    struct dirent *de;
    char file_path[PATH_MAX];

    if (dir == NULL)
    {
        return kStatus_USB_InvalidHandle;
    }

    if (fileInfo == NULL)
    {
        return kStatus_USB_InvalidParameter;
    }

    dir1 = (DIR *)dir;

    for (;;)
    {
        LOG_INFO("Attempt read dir");

        de = readdir(dir1);

        if (de == NULL)
        {
            LOG_ERROR("Failed read dir: %d", errno);
            return kStatus_USB_Error; /* return on error */
        }

        if (de->d_name == 0U)
        {
            return kStatus_USB_InvalidRequest; /* return on end of dir */
        }

        // Skip . and .. directories
        if ((strcmp(de->d_name, ".") != 0) && (strcmp(de->d_name, "..") != 0))
        {
            // Update the current path
            strcpy(file_path, (const char *)dirName);
            strcat(file_path, "/");
            strcat(file_path, (const char*)de->d_name);

            break;
        }
    }

    return USB_DeviceMtpFstat(dir1, (const uint16_t *)file_path, fileInfo);
}

usb_status_t USB_DeviceMtpMakeDir(const uint16_t *fileName)
{
    LOG_INFO("Attempt create dir: %s", (const char *)fileName);

    if (mkdir((const char *)fileName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
    {
        LOG_ERROR("Failed create dir: %d", errno);
        return kStatus_USB_Error;
    }

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpUnlink(const uint16_t *fileName)
{
    LOG_INFO("Attempt remove dir: %s", (const char *)fileName);

    if (remove((const char *)fileName) != 0)
    {
        LOG_ERROR("Failed remove dir: %d", errno);
        return kStatus_USB_Error;
    }

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpRename(const uint16_t *oldName, const uint16_t *newName)
{
    LOG_INFO("Attempt rename: %s -> %s", (const char *)oldName, (const char *)newName);

    if (rename((const char *)oldName, (const char *)newName) != 0)
    {
        LOG_ERROR("Failed rename: %d", errno);
        return kStatus_USB_Error;
    }

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpGetDiskTotalBytes(const uint16_t *path, uint64_t *totalBytes)
{
    struct statvfs stvfs {};

    if (totalBytes == NULL)
    {
        return kStatus_USB_InvalidParameter;
    }

    LOG_INFO("Attempt get total bytes");

    statvfs( purefs::dir::getRootDiskPath().c_str(), &stvfs);

    *totalBytes = (uint64_t)stvfs.f_frsize * stvfs.f_blocks;

    LOG_INFO("Total bytes in %s : %lu", purefs::dir::getRootDiskPath().c_str(), (uint32_t)*totalBytes);

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpGetDiskFreeBytes(const uint16_t *path, uint64_t *freeBytes)
{
    struct statvfs stvfs {};

    if (freeBytes == NULL)
    {
        return kStatus_USB_InvalidParameter;
    }

    LOG_INFO("Attempt get free bytes");

    statvfs( purefs::dir::getRootDiskPath().c_str(), &stvfs);

    *freeBytes = uint64_t(stvfs.f_bsize) * stvfs.f_bavail;

    LOG_INFO("Free bytes in %s : %lu", purefs::dir::getRootDiskPath().c_str(), (uint32_t)*freeBytes);

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpFSInit(const uint16_t *path)
{
//     BYTE work[FF_MAX_SS];

//     if (FR_OK != f_mount(&s_Fs, (const TCHAR *)path, 1U))
//     {
// #if FF_USE_MKFS
//         if (FR_OK != f_mkfs((const TCHAR *)path, 0, work, sizeof(work)))
//         {
//             return kStatus_USB_Error;
//         }
// #endif /* FF_USE_MKFS */
//     }

    return kStatus_USB_Success;
}

};
 //  extern "C" {