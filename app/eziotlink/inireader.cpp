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

#include "inireader.h"
#include <stdio.h>
#include <stdlib.h>

using namespace ezviz_test_ezdevice;

#define INI_CONFIGLEN 512

CIniReader::CIniReader( )  
{
    m_iniMap.clear();
}

CIniReader::~CIniReader()  
{  
    m_iniMap.clear();  
}

int CIniReader::LoadKeyMap(const char* iniFileName)  
{  
    FILE* fp = NULL;
    int res = INI_SUCCESS;
    
    do
    {
        fp = fopen(iniFileName, "r");
        if (NULL == fp)
        {
            res = INI_OPENFILE_ERROR;
            printf("open inifile %s error!\n", iniFileName);
            break;
        }
        
        m_iniMap.clear();
        
        std::string strLine, strMainKey;
        char szLine[INI_CONFIGLEN] = {0};
        std::string::size_type indexPos = std::string::npos;
        std::string::size_type  leftPos = std::string::npos;  
        std::string::size_type  rightPos = std::string::npos;
        INIAttrValueMap attrValueMap;
        
        while (fgets(szLine, INI_CONFIGLEN, fp)) 
        {
            strLine.assign(szLine);
            
            if (strLine.size() < 3 || szLine[0] == '#' || szLine[0] == ';')
            {
                continue;
            }
            leftPos = strLine.find("\n");
            if (std::string::npos != leftPos)
            {
                strLine.erase(leftPos, 1);
            }
            leftPos = strLine.find("\r");
            if (std::string::npos != leftPos)
            {
                strLine.erase(leftPos, 1);
            }
            // whether is main key
            leftPos = strLine.find("[");
            rightPos = strLine.find("]");
            if (leftPos != std::string::npos && rightPos != std::string::npos)  
            {
                attrValueMap.clear();
                strMainKey = strLine.substr(leftPos + 1, rightPos - leftPos - 1);
                m_iniMap.insert(std::make_pair(strMainKey, attrValueMap));
                
                printf("mainKey: %s\n", strMainKey.c_str());
                continue;
            }
            
            indexPos = strLine.find("=");
            if (std::string::npos != indexPos)
            {
                std::string strAttr, strValue;
                strAttr = strLine.substr(0, indexPos);
                strValue = strLine.substr(indexPos+1, strLine.size()-indexPos-1);
                m_iniMap[strMainKey].insert(std::make_pair(strAttr, strValue));
                
                printf("attribute: %s, value: %s\n", strAttr.c_str(), strValue.c_str());
            }
        }
        
        if (m_iniMap.empty())
        {
            res = INI_ERROR;
        }
    } while (false);
    
    if (fp)
    {
        fclose(fp);
    }
  
    return res;  
}   
  
int CIniReader::GetInteger(const char* mainKey, const char* attribute, int& nValue)  
{
    int res = INI_SUCCESS;
    
    res = GetAttrValue(mainKey, attribute);
    if( INI_SUCCESS == res)
    {  
        nValue = atoi(m_strAttrValue.c_str());
    }
    
    return res;
}  

int CIniReader::GetString(const char* mainKey, const char* attribute, std::string& strValue)
{
    int res = INI_SUCCESS;
    
    res = GetAttrValue(mainKey, attribute);
    if( INI_SUCCESS == res)
    {  
        strValue = m_strAttrValue;
    }
    
    return res;
}

int CIniReader::ParserByteArray(std::string& strValue, unsigned char* szArray, int len)
{
    if (strValue.empty())
    {
        return 0;
    }
    
    char* szValue = (char*)strValue.data();
    int index = 0;
    bool bQuite = false;
    
    do
    {
        std::string::size_type pos = 0;
        std::string::size_type leftPos = 0;
        std::string::size_type rightPos = 0;
        
        while (pos < strValue.size())
        {
            if (szValue[pos] == '-' || ('0' <= szValue[pos] && szValue[pos] <= '9'))
            {
                leftPos = pos;
                rightPos = strValue.find(',', leftPos);
                if (std::string::npos == rightPos)
                {
                    rightPos = strValue.find('}', leftPos);
                    bQuite = true;
                }
                if (std::string::npos == rightPos)
                {
                    rightPos = strValue.size();
                    bQuite = true;
                }
                szArray[index] = (unsigned char)atoi(strValue.substr(leftPos, rightPos-leftPos).c_str());
                index += 1;

                pos = rightPos + 1;
            }
            else
            {
                pos++;
            }
        }
    } while (index < len && !bQuite);

    return 0;
}

int CIniReader::GetAttrValue(const char* mainKey, const char* attribute)  
{
    int res = INI_SUCCESS;
    m_strAttrValue.clear();
    
    do
    {
        INIMainKeyMap::iterator it = m_iniMap.find(mainKey);
        if (it == m_iniMap.end())
        {
            printf("mainKey not exist: %s\n", mainKey);
            res = INI_NO_ATTR;
            break;
        }
        
        INIAttrValueMap::iterator itAttr = it->second.find(attribute);
        if (itAttr == it->second.end())
        {
            printf("attribute not exist: %s\n", attribute);
            res = INI_NO_ATTR;
            break;
        }
        
        m_strAttrValue = itAttr->second;
    } while (false);
   
    return res;  
}

