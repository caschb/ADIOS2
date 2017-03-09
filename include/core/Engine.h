/*
 * Engine.h
 *
 *  Created on: Nov 7, 2016
 *      Author: wfg
 */

#ifndef ENGINE_H_
#define ENGINE_H_


/// \cond EXCLUDE_FROM_DOXYGEN
#include <vector>
#include <string>
#include <memory> //std::shared_ptr
#include <map>
#include <utility> //std::pair
#include <complex> //std::complex
/// \endcond

#ifdef ADIOS_NOMPI
  #include "mpidummy.h"
#else
  #include <mpi.h>
#endif

#include "ADIOS.h"
#include "core/Method.h"
#include "core/Variable.h"
#include "core/VariableCompound.h"
#include "core/Transform.h"
#include "core/Transport.h"
#include "core/Capsule.h"
#include "core/Profiler.h"

namespace adios
{

/**
 * Base class for Engine operations managing shared-memory, and buffer and variables transform and transport operations
 */
class Engine
{

public:

    MPI_Comm m_MPIComm = MPI_COMM_SELF;

    const std::string m_EngineType; ///< from derived class
    const std::string m_Name; ///< name used for this engine
    const std::string m_AccessMode; ///< accessMode for buffers used by this engine
    const Method& m_Method; ///< associated method containing engine metadata

    int m_RankMPI = 0; ///< current MPI rank process
    int m_SizeMPI = 1; ///< current MPI processes size

    const std::string m_HostLanguage = "C++"; ///< default host language

    /**
     * Unique constructor
     * @param adios
     * @param engineType
     * @param name
     * @param accessMode
     * @param mpiComm
     * @param method
     * @param debugMode
     * @param cores
     * @param endMessage
     */
    Engine( ADIOS& adios, const std::string engineType, const std::string name, const std::string accessMode,
            MPI_Comm mpiComm, const Method& method, const bool debugMode, const unsigned int cores,
            const std::string endMessage );

    virtual ~Engine( );


    /**
     * Needed for DataMan Engine
     * @param callback function passed from the user
     */
    virtual void SetCallBack( std::function<void( const void*, std::string, std::string, std::string, Dims )> callback );

    /**
     * Write function that adds static checking on the variable to be passed by values
     * It then calls its corresponding derived class virtual function
     * This version uses m_Group to look for the variableName.
     * @param variable name of variable to the written
     * @param values pointer passed from the application
     */
    template< class T >
    void Write( Variable<T>& variable, const T* values )
    {
        Write( variable, values );
    }

    /**
     * String version
     * @param variableName
     * @param values
     */
    template< class T >
    void Write( const std::string variableName, const T* values )
    {
        Write( variableName, values );
    }

    /**
     * Single value version
     * @param variable
     * @param values
     */
    template< class T >
    void Write( Variable<T>& variable, const T values )
    {
        const T val = values;
        Write( variable, &val );
    }

    /**
     * Single value version using string as variable handlers, allows rvalues to be passed
     * @param variableName
     * @param values
     */
    template< class T >
    void Write( const std::string variableName, const T values )
    {
        const T val = values;
        Write( variableName, &val );
    }


    /**
     * Indicates that a new step is going to be written as new variables come in.
     */
    virtual void Advance( );


    //Read API
    /**
     * Inquires and (optionally) allocates and copies the contents of a variable
     * If success: it returns a pointer to the internal stored variable object in ADIOS class.
     * If failure: it returns nullptr
     * @param name variable name to look for
     * @param readIn if true: reads the full variable and payload, allocating values in memory, if false: internal payload is nullptr
     * @return success: it returns a pointer to the internal stored variable object in ADIOS class, failure: nullptr
     */
    virtual Variable<void>* InquireVariable( const std::string name, const bool readIn = true );
    virtual Variable<char>* InquireVariableChar( const std::string name, const bool readIn = true );
    virtual Variable<unsigned char>* InquireVariableUChar( const std::string name, const bool readIn = true );
    virtual Variable<short>* InquireVariableShort( const std::string name, const bool readIn = true );
    virtual Variable<unsigned short>* InquireVariableUShort( const std::string name, const bool readIn = true );
    virtual Variable<int>* InquireVariableInt( const std::string name, const bool readIn = true );
    virtual Variable<unsigned int>* InquireVariableUInt( const std::string name, const bool readIn = true );
    virtual Variable<long int>* InquireVariableLInt( const std::string name, const bool readIn = true );
    virtual Variable<unsigned long int>* InquireVariableULInt( const std::string name, const bool readIn = true );
    virtual Variable<long long int>* InquireVariableLLInt( const std::string name, const bool readIn = true );
    virtual Variable<unsigned long long int>* InquireVariableULLInt( const std::string name, const bool readIn = true );
    virtual Variable<float>* InquireVariableFloat( const std::string name, const bool readIn = true );
    virtual Variable<double>* InquireVariableDouble( const std::string name, const bool readIn = true );
    virtual Variable<long double>* InquireVariableLDouble( const std::string name, const bool readIn = true );
    virtual Variable<std::complex<float>>* InquireVariableCFloat( const std::string name, const bool readIn = true );
    virtual Variable<std::complex<double>>* InquireVariableCDouble( const std::string name, const bool readIn = true );
    virtual Variable<std::complex<long double>>* InquireVariableCLDouble( const std::string name, const bool readIn = true );
    virtual VariableCompound* InquireVariableCompound( const std::string name, const bool readIn = true );

    virtual void Close( const int transportIndex = -1  ) = 0; ///< Closes a particular transport, or all if -1


protected:

    ADIOS& m_ADIOS; ///< reference to ADIOS object that creates this Engine at Open
    std::vector< std::shared_ptr<Transport> > m_Transports; ///< transports managed
    const bool m_DebugMode = false; ///< true: additional checks, false: by-pass checks
    unsigned int m_Cores = 1;
    const std::string m_EndMessage; ///< added to exceptions to improve debugging

    Profiler m_Profiler; ///< engine time and bytes profiler

    std::set<std::string> m_WrittenVariables; ///< contains the names of the variables that are being written

    virtual void Init( ); ///< Initialize m_Capsules and m_Transports, called from constructor
    virtual void InitParameters( ); ///< Initialize parameters from Method, called from Initi in constructor
    virtual void InitTransports( ); ///< Initialize transports from Method, called from Init in constructor


    virtual void Write( Variable<char>& variable,                      const char* values );
    virtual void Write( Variable<unsigned char>& variable,             const unsigned char* values );
    virtual void Write( Variable<short>& variable,                     const short* values );
    virtual void Write( Variable<unsigned short>& variable,            const unsigned short* values );
    virtual void Write( Variable<int>& variable,                       const int* values );
    virtual void Write( Variable<unsigned int>& variable,              const unsigned int* values );
    virtual void Write( Variable<long int>& variable,                  const long int* values );
    virtual void Write( Variable<unsigned long int>& variable,         const unsigned long int* values );
    virtual void Write( Variable<long long int>& variable,             const long long int* values );
    virtual void Write( Variable<unsigned long long int>& variable,    const unsigned long long int* values );
    virtual void Write( Variable<float>& variable,                     const float* values );
    virtual void Write( Variable<double>& variable,                    const double* values );
    virtual void Write( Variable<long double>& variable,               const long double* values );
    virtual void Write( Variable<std::complex<float>>& variable,       const std::complex<float>* values );
    virtual void Write( Variable<std::complex<double>>& variable,      const std::complex<double>* values );
    virtual void Write( Variable<std::complex<long double>>& variable, const std::complex<long double>* values );
    virtual void Write( VariableCompound& variable,                    const void* values );


    /**
     * @brief Write functions can be overridden by derived classes. Base class behavior is to:
     * 1) Write to Variable values (m_Values) using the pointer to default group *m_Group set with SetDefaultGroup function
     * 2) Transform the data
     * 3) Write to all capsules -> data and metadata
     * @param variableName
     * @param values coming from user app
     */
    virtual void Write( const std::string variableName, const char* values );
    virtual void Write( const std::string variableName, const unsigned char* values );
    virtual void Write( const std::string variableName, const short* values );
    virtual void Write( const std::string variableName, const unsigned short* values );
    virtual void Write( const std::string variableName, const int* values );
    virtual void Write( const std::string variableName, const unsigned int* values );
    virtual void Write( const std::string variableName, const long int* values );
    virtual void Write( const std::string variableName, const unsigned long int* values );
    virtual void Write( const std::string variableName, const long long int* values );
    virtual void Write( const std::string variableName, const unsigned long long int* values );
    virtual void Write( const std::string variableName, const float* values );
    virtual void Write( const std::string variableName, const double* values );
    virtual void Write( const std::string variableName, const long double* values );
    virtual void Write( const std::string variableName, const std::complex<float>* values );
    virtual void Write( const std::string variableName, const std::complex<double>* values );
    virtual void Write( const std::string variableName, const std::complex<long double>* values );
    virtual void Write( const std::string variableName, const void* values );


    /**
     * Used to verify parameters in m_Method containers
     * @param itParam iterator to a certain parameter
     * @param parameters map of parameters, from m_Method
     * @param parameterName used if exception is thrown to provide debugging information
     * @param hint used if exception is thrown to provide debugging information
     */
    void CheckParameter( const std::map<std::string, std::string>::const_iterator itParam,
                         const std::map<std::string, std::string>& parameters,
                         const std::string parameterName,
                         const std::string hint ) const;

    bool TransportNamesUniqueness( ) const; ///< checks if transport names are unique among the same types (file I/O)


    /**
     * Throws an exception in debug mode if transport index is out of range.
     * @param transportIndex must be in the range [ -1 , m_Transports.size()-1 ]
     */
    void CheckTransportIndex( const int transportIndex );

};


} //end namespace

#endif /* ENGINE_H_ */
