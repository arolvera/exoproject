//
// Created by marvin on 7/21/23.
//

#ifndef COMPONENT_MAP_DEF_H
#define COMPONENT_MAP_DEF_H
#include "storage_class.h"

typedef struct component_t_ {
  int start_address;
  int end_address;
} component_t;

typedef struct component_map_t_ {
  const char *accessor_id;
  component_t component;
  bool open_for_write;
  WRITE_FUNC write_func;
  fileops_t  *fops;
} component_map_t;

extern component_map_t file_map[];
extern const size_t FILE_MAP_SIZE;

#endif //COMPONENT_MAP_DEF_H
