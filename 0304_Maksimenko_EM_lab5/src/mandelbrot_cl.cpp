#include "mandelbrot_cl.h"

#include <CL/cl.h>
#include <string>
#include <exception>
#include <fstream>
#include <sstream>

namespace // anonymous
{

constexpr size_t maxOclDevices = 64;
constexpr char kernelFile[] = "kernels/mandelbrot.cl";
constexpr char kernelFn[] = "calculate";

void OclProcessRc( cl_int rc, const std::string& filename, size_t line )
{
    if ( CL_SUCCESS == rc )
        return;

    throw std::runtime_error{ "OpenCL error " + std::to_string( rc )
                             + " at " + filename + ":" + std::to_string( line ) };
} // OclProcessRc


#define OCL_CHECK_RC( rc ) OclProcessRc( rc, __FILE__, __LINE__ );


cl_context OclCreateContext( std::array< cl_device_id, maxOclDevices >& devices, cl_uint& devCount )
{
    cl_int rc = -30;

    cl_platform_id platform;
    OCL_CHECK_RC( clGetPlatformIDs( 1, &platform, nullptr ) );

    rc = clGetDeviceIDs( platform, CL_DEVICE_TYPE_GPU, maxOclDevices, devices.data(), &devCount );
    if ( CL_SUCCESS != rc )
    {
        std::cerr << "No GPUs found (error code: " << rc << "). Trying to use CPUs\n";
        OCL_CHECK_RC( clGetDeviceIDs( platform, CL_DEVICE_TYPE_CPU, maxOclDevices, devices.data(), &devCount ) );
    }

    const cl_context_properties contextProperties[] = {
          CL_CONTEXT_PLATFORM, reinterpret_cast< cl_context_properties >( platform )
        , 0
    };

    cl_context context = clCreateContext( contextProperties, devCount, devices.data(), nullptr, nullptr, &rc );
    OCL_CHECK_RC( rc );

    return context;
} // OclCreateContext


cl_program OclCreateProgram(
      cl_context context
    , const std::vector< std::string >& files
    , const std::vector< std::string >& kernelNames
    , const std::array< cl_device_id, maxOclDevices>& devices
    , cl_uint devCount
    , std::vector< cl_kernel >& kernels )
{
    cl_int rc;

    std::vector< std::string > filesContent;
    std::vector< const char* > filesRawContent;
    std::vector< size_t > filesLength;

    for ( const auto& file: files )
    {
        std::ifstream fileStream( file );
        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        filesContent.push_back( buffer.str() );
        filesRawContent.emplace_back( filesContent.back().c_str() );
        filesLength.emplace_back( filesContent.back().length() );
    }

    cl_program program = clCreateProgramWithSource( context, files.size(), filesRawContent.data(), filesLength.data(), &rc );
    OCL_CHECK_RC( rc );

    rc = clBuildProgram( program, devCount, devices.data(), nullptr, nullptr, nullptr );
    if ( CL_SUCCESS != rc )
    {
        size_t buildLogSize = 0;
        for ( size_t i = 0; i < devCount; ++i )
        {
            clGetProgramBuildInfo( program, devices[ i ], CL_PROGRAM_BUILD_LOG, 0, NULL, &buildLogSize );
            std::vector< char > buildLog( buildLogSize );
            clGetProgramBuildInfo( program, devices[ i ], CL_PROGRAM_BUILD_LOG, buildLogSize, buildLog.data(), NULL );
            std::cerr << buildLog.data() << std::endl;
        }
        OCL_CHECK_RC( rc );
    }

    for ( const auto& kernelName: kernelNames )
    {
        cl_kernel kernel = clCreateKernel( program, kernelName.c_str(), &rc );
        OCL_CHECK_RC( rc );
        kernels.push_back( kernel );
    }

    return program;
} // OclCreateProgram

} // anonymous namespace


MandelbrotCl::MandelbrotCl( size_t w, size_t h, size_t iterations )
    : Mandelbrot( w, h, iterations )
{} // MandelbrotCl


void MandelbrotCl::Generate()
{
    const size_t dataSize = width_ * height_;

    cl_int rc;
    std::array< cl_device_id, maxOclDevices > devices;
    cl_uint devCount;
    cl_context context;
    std::array< cl_command_queue, maxOclDevices > commandQueues;
    cl_mem buffer;
    cl_program program;
    std::vector< cl_kernel > kernels;

    try
    {
        context = OclCreateContext( devices, devCount );

        for ( size_t i = 0; i < devCount; ++i )
        {
            commandQueues[ i ] = clCreateCommandQueueWithProperties( context, devices[ i ], nullptr, &rc );
            OCL_CHECK_RC( rc );
        }

        buffer = clCreateBuffer( context, CL_MEM_WRITE_ONLY, dataSize, nullptr, &rc );
        OCL_CHECK_RC( rc );

        program = OclCreateProgram( context, { kernelFile }, { kernelFn }, devices, devCount, kernels );

        int iterations = static_cast< int >( iterations_ );
        for ( cl_kernel kernel: kernels )
        {
            OCL_CHECK_RC( clSetKernelArg( kernel, 0, sizeof( iterations ), &iterations ) );
            OCL_CHECK_RC( clSetKernelArg( kernel, 1, sizeof( buffer ), &buffer ) );
        }

        const size_t dataHeightSize = height_ / devCount;
        for ( size_t i = 0; i < devCount; ++i )
        {
            const size_t currentHeightSize = ( ( i == devCount - 1 ) ? ( height_ - dataHeightSize * devCount ) : 0 ) + dataHeightSize;
            const size_t offset = dataHeightSize * i;
            const size_t workOffset[ 2 ] = { 0, offset };
            const size_t workSize[ 2 ] = { width_, currentHeightSize };

            OCL_CHECK_RC( clEnqueueNDRangeKernel(
                  commandQueues[ i ]
                , kernels[ 0 ]
                , 2
                , workOffset
                , workSize
                , nullptr
                , 0
                , nullptr
                , nullptr
            ) );

            OCL_CHECK_RC( clEnqueueReadBuffer( commandQueues[ i ], buffer, CL_FALSE
                , offset * width_, currentHeightSize * width_, points_.get(), 0, nullptr, nullptr ) );
        }

        for ( cl_kernel kernel: kernels )
        {
            OCL_CHECK_RC( clReleaseKernel( kernel ) );
        }
        OCL_CHECK_RC( clReleaseProgram( program ) );
        OCL_CHECK_RC( clReleaseMemObject( buffer ) );
        for ( size_t i = 0; i < devCount; ++i )
        {
            OCL_CHECK_RC( clReleaseCommandQueue( commandQueues[ i ] ) );
        }
        OCL_CHECK_RC( clReleaseContext( context ) );
    }
    catch ( const std::exception& e )
    {
        std::cerr << e.what() << std::endl;
    }
} // Generate
