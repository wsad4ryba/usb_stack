/*
 * Copyright  Onplick <info@onplick.com> - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */
#ifndef _MTP_DB_H
#define _MTP_DB_H

struct mtp_db;

struct mtp_db *mtp_db_alloc(void);
void mtp_db_free(struct mtp_db *db);
uint32_t mtp_db_add(struct mtp_db *db, const char *filename);
void mtp_db_update(struct mtp_db *db, uint32_t handle, const char *key);
const char *mtp_db_get(struct mtp_db *db, uint32_t handle);
void mtp_db_del(struct mtp_db *db, uint32_t handle);

#endif /* _MTP_DB_H */