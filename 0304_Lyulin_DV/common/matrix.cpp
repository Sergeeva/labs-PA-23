#include <matrix.h>
#include <cstdlib>
#include <cassert>
#include <ostream>
#include <iomanip>


Matrix::Matrix( size_t rows, size_t cols ):
     rows_{ rows }, cols_{ cols }, data_( rows * cols, 0.0f )
{}


size_t Matrix::get_rows() const
{
     return rows_;
}


size_t Matrix::get_cols() const
{
     return cols_;
}


const float *Matrix::data() const
{
     return data_.data();
}


float *Matrix::data()
{
     return data_.data();
}


float Matrix::get( size_t i, size_t j ) const
{
     return data_[ i * cols_ + j ];
}


float& Matrix::get( size_t i, size_t j )
{
     return data_[ i * cols_ + j ];
}


Matrix operator*( const Matrix& lhs, const Matrix& rhs )
{
     assert( lhs.get_cols() == rhs.get_rows() );
     Matrix result( lhs.get_rows(), rhs.get_cols() );
     for ( size_t i = 0; i < result.get_rows(); i++ )
     {
          for ( size_t j = 0; j < result.get_cols(); j++ )
          {
               for ( size_t k = 0; k < lhs.get_cols(); k++ )
               {
                    result.get( i, j ) += lhs.get( i, k ) * rhs.get( k, j );
               }
          }
     }
     return result;
}


Matrix Matrix::generate( size_t i, size_t j )
{
     assert( i );
     assert( j );
     Matrix result( i, j );
     size_t elem_count = i * j;
     for ( size_t k = 0; k < elem_count; k++ )
     {
          result.data_[ k ] = rand_min + std::rand() / ( RAND_MAX / ( rand_max - rand_min ) );
     }
     return result;
}


std::ostream& operator<<( std::ostream& lhs, const Matrix& rhs )
{
     for ( size_t i = 0; i < rhs.get_rows(); i++ )
     {
          for ( size_t j = 0; j < rhs.get_cols(); j++ )
          {
               lhs << std::setprecision( Matrix::output_precision ) << std::fixed
                   << std::setw( 10 ) << rhs.get( i, j );
          }
          lhs << '\n';
     }
     return lhs;
}
