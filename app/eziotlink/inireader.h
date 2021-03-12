/*******************************************************************************
 * Copyright © 2017-2021 Ezviz Inc.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *******************************************************************************/
 
#ifndef _TEST_EZDEVICE_INIREADER_H
#define _TEST_EZDEVICE_INIREADER_H

#include <map>
#include <string.h>

namespace ezviz_test_ezdevice
{

enum INI_RES  
{  
    INI_SUCCESS,        ///< success
    INI_ERROR,          ///< general error
    INI_OPENFILE_ERROR, ///< open file failed
    INI_NO_ATTR         ///< attribute/key not exist
};  
  
// map table for detail Attribute/Key and value
typedef std::map<std::string, std::string> INIAttrValueMap;

// map tabel for main Key and detail attribute map
typedef std::map<std::string, INIAttrValueMap> INIMainKeyMap;

class CIniReader
{
public:
    CIniReader();
    ~CIniReader();

public:  
    int LoadKeyMap(const char* iniFileName);
    int GetInteger(const char* mainKey, const char* attribute, int& nValue);
    int GetString(const char* mainKey, const char* attribute, std::string& strValue);

    int ParserByteArray(std::string& strValue, unsigned char* szArray, int len);

private:
    int GetAttrValue(const char* mainKey, const char* attribute);

private:
    std::string m_strAttrValue;
    INIMainKeyMap m_iniMap;
};

}

#endif // _TEST_EZDEVICE_INIREADER_H
