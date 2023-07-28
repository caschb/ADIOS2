/*
 * Distributed under the OSI-approved Apache License, Version 2.0.  See
 * accompanying file Copyright.txt for details.
 *
 * DaosEngine.cpp
 *
 */

#include "DaosEngine.h"

#include "adios2/common/ADIOSMacros.h"
#include "adios2/common/ADIOSTypes.h" //PathSeparator
#include "adios2/core/IO.h"
#include "adios2/helper/adiosFunctions.h" //CreateDirectory, StringToTimeUnit,

#include <ctime>
#include <iostream>

namespace adios2
{
namespace core
{
namespace engine
{

std::vector<std::string> DaosEngine::GetBPMetadataFileNames(
    const std::vector<std::string> &names) const noexcept
{
    std::vector<std::string> metadataFileNames;
    metadataFileNames.reserve(names.size());
    for (const auto &name : names)
    {
        metadataFileNames.push_back(GetBPMetadataFileName(name));
    }
    return metadataFileNames;
}

std::vector<std::string> DaosEngine::GetBPMetaMetadataFileNames(
    const std::vector<std::string> &names) const noexcept
{
    std::vector<std::string> metaMetadataFileNames;
    metaMetadataFileNames.reserve(names.size());
    for (const auto &name : names)
    {
        metaMetadataFileNames.push_back(GetBPMetaMetadataFileName(name));
    }
    return metaMetadataFileNames;
}

std::string
DaosEngine::GetBPMetadataFileName(const std::string &name) const noexcept
{
    const std::string bpName = helper::RemoveTrailingSlash(name);
    const size_t index = 0; // global metadata file is generated by rank 0
    /* the name of the metadata file is "md.0" */
    const std::string bpMetaDataRankName(bpName + PathSeparator + "md." +
                                         std::to_string(index));
    return bpMetaDataRankName;
}

std::string
DaosEngine::GetBPMetaMetadataFileName(const std::string &name) const noexcept
{
    const std::string bpName = helper::RemoveTrailingSlash(name);
    const size_t index = 0; // global metadata file is generated by rank 0
    /* the name of the metadata file is "md.0" */
    const std::string bpMetaMetaDataRankName(bpName + PathSeparator + "mmd." +
                                             std::to_string(index));
    return bpMetaMetaDataRankName;
}

std::vector<std::string> DaosEngine::GetBPMetadataIndexFileNames(
    const std::vector<std::string> &names) const noexcept
{
    std::vector<std::string> metadataIndexFileNames;
    metadataIndexFileNames.reserve(names.size());
    for (const auto &name : names)
    {
        metadataIndexFileNames.push_back(GetBPMetadataIndexFileName(name));
    }
    return metadataIndexFileNames;
}

std::string
DaosEngine::GetBPMetadataIndexFileName(const std::string &name) const noexcept
{
    const std::string bpName = helper::RemoveTrailingSlash(name);
    /* the name of the metadata index file is "md.idx" */
    const std::string bpMetaDataIndexRankName(bpName + PathSeparator +
                                              "md.idx");
    return bpMetaDataIndexRankName;
}

std::vector<std::string> DaosEngine::GetBPVersionFileNames(
    const std::vector<std::string> &names) const noexcept
{
    std::vector<std::string> versionFileNames;
    versionFileNames.reserve(names.size());
    for (const auto &name : names)
    {
        versionFileNames.push_back(GetBPVersionFileName(name));
    }
    return versionFileNames;
}

std::string
DaosEngine::GetBPVersionFileName(const std::string &name) const noexcept
{
    const std::string bpName = helper::RemoveTrailingSlash(name);
    /* the name of the version file is ".bpversion" */
    const std::string bpVersionFileName(bpName + PathSeparator + ".bpversion");
    return bpVersionFileName;
}

std::string DaosEngine::GetBPSubStreamName(const std::string &name,
                                           const size_t id,
                                           const bool hasSubFiles,
                                           const bool isReader) const noexcept
{
    if (!hasSubFiles)
    {
        return name;
    }

    const std::string bpName = helper::RemoveTrailingSlash(name);
    /* the name of a data file starts with "data." */
    const std::string bpRankName(bpName + PathSeparator + "data." +
                                 std::to_string(id));
    return bpRankName;
}

std::vector<std::string>
DaosEngine::GetBPSubStreamNames(const std::vector<std::string> &names,
                                size_t subFileIndex) const noexcept
{
    std::vector<std::string> bpNames;
    bpNames.reserve(names.size());
    for (const auto &name : names)
    {
        bpNames.push_back(GetBPSubStreamName(name, subFileIndex));
    }
    return bpNames;
}

void DaosEngine::ParseParams(IO &io, struct DAOSParams &Params)
{
    adios2::Params params_lowercase;
    for (auto &p : io.m_Parameters)
    {
        const std::string key = helper::LowerCase(p.first);
        const std::string value = helper::LowerCase(p.second);
        params_lowercase[key] = value;
    }

    auto lf_SetBoolParameter = [&](const std::string key, bool &parameter,
                                   bool def) {
        const std::string lkey = helper::LowerCase(std::string(key));
        auto itKey = params_lowercase.find(lkey);
        parameter = def;
        if (itKey != params_lowercase.end())
        {
            std::string value = itKey->second;
            std::transform(value.begin(), value.end(), value.begin(),
                           ::tolower);
            if (value == "yes" || value == "true" || value == "on")
            {
                parameter = true;
            }
            else if (value == "no" || value == "false" || value == "off")
            {
                parameter = false;
            }
            else
            {
                helper::Throw<std::invalid_argument>(
                    "Engine", "DaosEngine", "ParseParams",
                    "Unknown BP5 Boolean parameter '" + value + "'");
            }
        }
    };

    auto lf_SetFloatParameter = [&](const std::string key, float &parameter,
                                    float def) {
        const std::string lkey = helper::LowerCase(std::string(key));
        auto itKey = params_lowercase.find(lkey);
        parameter = def;
        if (itKey != params_lowercase.end())
        {
            std::string value = itKey->second;
            parameter =
                helper::StringTo<float>(value, " in Parameter key=" + key);
        }
    };

    auto lf_SetSizeBytesParameter = [&](const std::string key,
                                        size_t &parameter, size_t def) {
        const std::string lkey = helper::LowerCase(std::string(key));
        auto itKey = params_lowercase.find(lkey);
        parameter = def;
        if (itKey != params_lowercase.end())
        {
            std::string value = itKey->second;
            parameter = helper::StringToByteUnits(
                value, "for Parameter key=" + key + "in call to Open");
        }
    };

    auto lf_SetIntParameter = [&](const std::string key, int &parameter,
                                  int def) {
        const std::string lkey = helper::LowerCase(std::string(key));
        auto itKey = params_lowercase.find(lkey);
        parameter = def;
        if (itKey != params_lowercase.end())
        {
            parameter = std::stoi(itKey->second);
            return true;
        }
        return false;
    };

    auto lf_SetUIntParameter = [&](const std::string key,
                                   unsigned int &parameter, unsigned int def) {
        const std::string lkey = helper::LowerCase(std::string(key));
        auto itKey = params_lowercase.find(lkey);
        parameter = def;
        if (itKey != params_lowercase.end())
        {
            unsigned long result = std::stoul(itKey->second);
            if (result > std::numeric_limits<unsigned>::max())
            {
                result = std::numeric_limits<unsigned>::max();
            }
            parameter = static_cast<unsigned int>(result);
            return true;
        }
        return false;
    };

    auto lf_SetStringParameter = [&](const std::string key,
                                     std::string &parameter, const char *def) {
        const std::string lkey = helper::LowerCase(std::string(key));
        auto itKey = params_lowercase.find(lkey);
        parameter = def;
        if (itKey != params_lowercase.end())
        {
            parameter = itKey->second;
            return true;
        }
        return false;
    };

    auto lf_SetBufferVTypeParameter = [&](const std::string key, int &parameter,
                                          int def) {
        const std::string lkey = helper::LowerCase(std::string(key));
        auto itKey = params_lowercase.find(lkey);
        parameter = def;
        if (itKey != params_lowercase.end())
        {
            std::string value = itKey->second;
            std::transform(value.begin(), value.end(), value.begin(),
                           ::tolower);
            if (value == "malloc")
            {
                parameter = (int)BufferVType::MallocVType;
            }
            else if (value == "chunk")
            {
                parameter = (int)BufferVType::ChunkVType;
            }
            else
            {
                helper::Throw<std::invalid_argument>(
                    "Engine", "DaosEngine", "ParseParams",
                    "Unknown BP5 BufferVType parameter \"" + value +
                        "\" (must be \"malloc\" or \"chunk\"");
            }
        }
    };

    auto lf_SetAggregationTypeParameter = [&](const std::string key,
                                              int &parameter, int def) {
        const std::string lkey = helper::LowerCase(std::string(key));
        auto itKey = params_lowercase.find(lkey);
        parameter = def;
        if (itKey != params_lowercase.end())
        {
            std::string value = itKey->second;
            std::transform(value.begin(), value.end(), value.begin(),
                           ::tolower);
            if (value == "everyonewrites" || value == "auto")
            {
                parameter = (int)AggregationType::EveryoneWrites;
            }
            else if (value == "everyonewritesserial")
            {
                parameter = (int)AggregationType::EveryoneWritesSerial;
            }
            else if (value == "twolevelshm")
            {
                parameter = (int)AggregationType::TwoLevelShm;
            }
            else
            {
                helper::Throw<std::invalid_argument>(
                    "Engine", "DaosEngine", "ParseParams",
                    "Unknown BP5 AggregationType parameter \"" + value +
                        "\" (must be \"auto\", \"everyonewrites\" or "
                        "\"twolevelshm\"");
            }
        }
    };

    auto lf_SetAsyncWriteParameter = [&](const std::string key, int &parameter,
                                         int def) {
        const std::string lkey = helper::LowerCase(std::string(key));
        auto itKey = params_lowercase.find(lkey);
        parameter = def;
        if (itKey != params_lowercase.end())
        {
            std::string value = itKey->second;
            std::transform(value.begin(), value.end(), value.begin(),
                           ::tolower);
            if (value == "guided" || value == "auto" || value == "on" ||
                value == "true")
            {
                parameter = (int)AsyncWrite::Guided;
            }
            else if (value == "sync" || value == "off" || value == "false")
            {
                parameter = (int)AsyncWrite::Sync;
            }
            else if (value == "naive")
            {
                parameter = (int)AsyncWrite::Naive;
            }
            else
            {
                helper::Throw<std::invalid_argument>(
                    "Engine", "DaosEngine", "ParseParams",
                    "Unknown BP5 AsyncWriteMode parameter \"" + value +
                        "\" (must be \"auto\", \"sync\", \"naive\", "
                        "\"throttled\" "
                        "or \"guided\"");
            }
        }
    };

#define get_params(Param, Type, Typedecl, Default)                             \
    lf_Set##Type##Parameter(#Param, Params.Param, Default);

    DAOS_FOREACH_PARAMETER_TYPE_4ARGS(get_params);
#undef get_params

    if (Params.verbose > 0 && !m_RankMPI)
    {
        std::cout << "---------------- " << io.m_EngineType
                  << " engine parameters --------------\n";
#define print_params(Param, Type, Typedecl, Default)                           \
    lf_Set##Type##Parameter(#Param, Params.Param, Default);                    \
    if (!m_RankMPI)                                                            \
    {                                                                          \
        std::cout << "  " << std::string(#Param) << " = " << Params.Param      \
                  << "   default = " << Default << std::endl;                  \
    }

        DAOS_FOREACH_PARAMETER_TYPE_4ARGS(print_params);
#undef print_params
        std::cout << "-----------------------------------------------------"
                  << std::endl;
    }
};

} // namespace engine
} // namespace core
} // namespace adios2
