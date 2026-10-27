#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
struct SaveLoadInfo;
const char *GameSaveIdentity(void);
bool GameSaveAllowed(void);
bool GameSavePreflight(const void *data, size_t size);
void GameSaveExtra(struct SaveLoadInfo *sli);
void GameLoadExtra(struct SaveLoadInfo *sli, uint32_t version);
void GameStateLoaded(uint32_t version);
