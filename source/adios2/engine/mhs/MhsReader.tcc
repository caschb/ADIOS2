/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * MhsReader.tcc
 *
 *  Created on: Aug 04, 2021
 *      Author: Jason Wang jason.ruonan.wang@gmail.com
 */

#ifndef ADIOS2_ENGINE_MHSREADER_TCC_
#define ADIOS2_ENGINE_MHSREADER_TCC_

#include "MhsReader.h"

namespace adios2
{
namespace core
{
namespace engine
{

template <class T>
inline void MhsReader::GetSyncCommon(Variable<T> &variable, T *data)
{
    GetDeferredCommon(variable, data);
    PerformGets();
}

template <>
inline void MhsReader::GetDeferredCommon(Variable<std::string> &variable,
                                         std::string *data)
{
    m_SubEngines[0]->Get(variable, data, Mode::Sync);
}

template <class T>
void MhsReader::GetDeferredCommon(Variable<T> &variable, T *data)
{
    m_SubEngines[0]->Get(variable, data, Mode::Sync);
    for (int i = 1; i < m_Tiers; ++i)
    {
        auto var = m_SubIOs[i]->InquireVariable<T>(variable.m_Name);
        m_SubEngines[i]->Get(*var, data, Mode::Sync);
    }
}

} // end namespace engine
} // end namespace core
} // end namespace adios2

#endif // ADIOS2_ENGINE_MHSREADER_TCC_