/*
 * Copyright (c) 2020 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */
#ifndef __AML_MESSAGE_BASE_H_
#define __AML_MESSAGE_BASE_H_

#include <string>
#include <vector>

class AmlMessageBase {
public:
    AmlMessageBase() {};
    virtual ~AmlMessageBase() {};
    virtual void setInt32(const char *name, int32_t value);
    virtual void setInt64(const char *name, int64_t value);
    virtual void setSize(const char *name, size_t value);
    virtual void setFloat(const char *name, float value);
    virtual void setDouble(const char *name, double value);
    virtual void setPointer(const char *name, void *value);
    virtual void setChar(const char *name, const char *s, ssize_t len = -1);
    virtual void setString(const char *name, const std::string &s);
    virtual void setMessage(const char *name, AmlMessageBase *msg);

    virtual bool contains(const char *name);
    virtual int32_t containsCount();

    virtual bool findInt32(const char *name, int32_t *value);
    virtual bool findInt64(const char *name, int64_t *value);
    virtual bool findSize(const char *name, size_t *value);
    virtual bool findFloat(const char *name, float *value);
    virtual bool findDouble(const char *name, double *value);
    virtual bool findPointer(const char *name, void **value);
    virtual bool findString(const char *name, std::string *value);
    virtual bool findMessage(const char *name, AmlMessageBase **msg);

    virtual size_t findEntryByName(const char* name);
    virtual bool setEntryNameAt(size_t  index, const char *name);

    enum Type {
      kTypeInt32,
      kTypeInt64,
      kTypeSize,
      kTypeFloat,
      kTypeDouble,
      kTypePointer,
      kTypeString,
      kTypeMessage,
    };
};

#endif /* __AML_MESSAGE_BASE_H_ */
