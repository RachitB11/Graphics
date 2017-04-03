#include "mat4.h"
#include <assert.h>
#include "math.h"

# define M_PI           3.14159265358979323846

///----------------------------------------------------------------------
/// Constructors
///----------------------------------------------------------------------

/// Default Constructor.  Initialize to identity matrix.
mat4::mat4(void){
    data[0] = vec4(1,0,0,0);
    data[1] = vec4(0,1,0,0);
    data[2] = vec4(0,0,1,0);
    data[3] = vec4(0,0,0,1);
}

/// Initializes the diagonal values of the matrix to diag. All other values are 0.
mat4::mat4(float diag){
    data[0] = vec4(diag,0,0,0);
    data[1] = vec4(0,diag,0,0);
    data[2] = vec4(0,0,diag,0);
    data[3] = vec4(0,0,0,diag);
}

/// Initializes matrix with each vector representing a column in the matrix
mat4::mat4(const vec4 &col0, const vec4 &col1, const vec4 &col2, const vec4& col3){
    data[0] = col0;
    data[1] = col1;
    data[2] = col2;
    data[3] = col3;
}

/// copy constructor; copies values of m2 into this
mat4::mat4(const mat4 &m2){
    // Copy the data column wise
    for(int i=0 ; i<4 ; i++){
        data[i] = m2.data[i];
    }
}

///----------------------------------------------------------------------
/// Getters
///----------------------------------------------------------------------

/// Returns the values of the column at the index
/// Does NOT check the array bound because doing so is slow
const vec4 &mat4::operator[](unsigned int index) const{
    return data[index];
}

/// Returns a reference to the column at the index
/// Does NOT check the array bound because doing so is slow
vec4 &mat4::operator[](unsigned int index){
    return data[index];
}

/// Returns the values of the column at the index
/// DOES check the array bound because doing so is slow
const vec4 &mat4::operator()(unsigned int index) const{
    assert(index<4);
    return data[index];
}

/// Returns a reference to the column at the index
/// DOES check the array bound because doing so is slow
vec4 &mat4::operator()(unsigned int index){
    assert(index<4);
    return data[index];
}

///----------------------------------------------------------------------
/// Static Initializers
///----------------------------------------------------------------------

/// Creates a 3-D rotation matrix.
/// Takes an angle in degrees and an axis represented by its xyz components, and outputs a 4x4 rotation matrix
mat4 mat4::rot(float angle, float x, float y, float z){
    mat4 result;
    float n = sqrt(pow(x,2.0)+pow(y,2.0)+pow(z,2.0));
    // Normalise axis components
    x = x/n;
    y = y/n;
    z = z/n;
    // Convert angle to radians
    angle *= (M_PI / 180.0);
    // Make the skew symmetrix matrix
    mat4 omega(vec4(0,z,-y,0),vec4(-z,0,x,0),vec4(y,-x,0,0),vec4(0,0,0,0));
    // Set identity matrix using the constructor
    mat4 identity;
    // Implement the rodrigues formula
    result = identity + (omega*sin(angle)) + (omega*omega*(1-cos(angle)));
    return result;
}

/// Takes an xyz displacement and outputs a 4x4 translation matrix
mat4 mat4::trans(float x, float y, float z){
    mat4 result;
    result[0] = vec4(1,0,0,0);
    result[1] = vec4(0,1,0,0);
    result[2] = vec4(0,0,1,0);
    result[3] = vec4(x,y,z,1);
    return result;
}

/// Takes an xyz scale and outputs a 4x4 scale matrix
mat4 mat4::scale(float x, float y, float z){
    mat4 result;
    result[0] = vec4(x,0,0,0);
    result[1] = vec4(0,y,0,0);
    result[2] = vec4(0,0,z,0);
    result[3] = vec4(0,0,0,1);
    return result;
}


///----------------------------------------------------------------------
/// Operator Functions
///----------------------------------------------------------------------

/// Assign m2 to this and return this
mat4 &mat4::operator=(const mat4 &m2){
    for(int i=0 ; i<4 ; i++){
        data[i] = m2.data[i];
    }
    return *this;
}

/// Test for equality
bool mat4::operator==(const mat4 &m2) const{
    // Check columnwise equality. If even one fails return false.
    for(int i=0 ; i<4 ; i++){
        if(m2[i] != data[i]){
                return false;
        }
    }
    return true;
}

/// Test for inequality
bool mat4::operator!=(const mat4 &m2) const{
    // Return not of the equality operator
    return !(*this==m2);
}

/// Element-wise arithmetic
/// e.g. += adds the elements of m2 to this and returns this (like regular +=)
///      +  returns a new matrix whose elements are the sums of this and v2
mat4 &mat4::operator+=(const mat4 &m2){
    // Column wise addition
    for(int i=0 ; i<4 ; i++){
        data[i] += m2[i];
    }
    return *this;
}

mat4 &mat4::operator-=(const mat4 &m2){
    // Column wise subtraction
    for(int i=0 ; i<4 ; i++){
        data[i] -= m2[i];
    }
    return *this;
}

mat4 &mat4::operator*=(float c){
    // multiplication of each column by a scalar
    for(int i=0 ; i<4 ; i++){
        data[i] *= c;
    }
    return *this;
}

mat4 &mat4::operator/=(float c){
    // division of each column by a scalar
    for(int i=0 ; i<4 ; i++){
        data[i] /= c;
    }
    return *this;
}

mat4 mat4::operator+(const mat4 &m2) const{
    mat4 result;
    for(int i=0 ; i<4 ; i++){
        result[i] = data[i] + m2[i];
    }
    return result;
}

mat4 mat4::operator-(const mat4 &m2) const{
    mat4 result;
    for(int i=0 ; i<4 ; i++){
        result[i] = data[i] - m2[i];
    }
    return result;
}
mat4 mat4::operator*(float c) const{
    mat4 result;
    for(int i=0 ; i<4 ; i++){// multiplication by a scalar
        result[i] = data[i] * c;
    }
    return result;
}

mat4 mat4::operator/(float c) const{// division by a scalar
    mat4 result;
    for(int i=0 ; i<4 ; i++){
        result[i] = data[i] / c;
    }
    return result;
}

/// Matrix multiplication (m1 * m2)
mat4 mat4::operator*(const mat4 &m2) const{
    mat4 result;
    // Using the matrix by column multiplication for each column in the 2nd matrix to find the columns in result matrix
    for(int i=0; i<4; i++){
        result[i] = (*this)*m2[i];
    }
    return result;
}

/// Matrix/vector multiplication (m * v)
/// Assume v is a column vector (ie. a 4x1 matrix)
vec4 mat4::operator*(const vec4 &v) const{
    vec4 result;
    // Finding the dot product of each row of the matrix with the column vector
    for(int i=0; i<4; i++){
        vec4 row_i(data[0][i],data[1][i],data[2][i],data[3][i]);
        result[i] = dot(row_i,v);
    }
    return result;
}

///----------------------------------------------------------------------
/// Matrix Operations
///----------------------------------------------------------------------

/// Returns the transpose of the input matrix (v_ij == v_ji)
mat4 mat4::transpose() const{
    mat4 result;
    // Set column of the result matrix using row data from the member variable
    for(int i=0; i<4; i++){
        vec4 row_i(data[0][i],data[1][i],data[2][i],data[3][i]);
        result[i] = row_i;
    }
    return result;
}


/// Returns a column of the input matrix
const vec4 &mat4::col(unsigned int index) const{// const version
    return data[index];
}
vec4 &mat4::col(unsigned int index){// non-const version
    return data[index];
}

/// Scalar multiplication (c * m)
mat4 operator*(float c, const mat4 &m){
    mat4 result;
    for(int i=0 ; i<4 ; i++){// multiplication by a scalar
        result[i] = m[i] * c;
    }
    return result;
}

/// Vector/matrix multiplication (v * m)
/// Assume v is a row vector (ie. a 1x4 matrix)
vec4 operator*(const vec4 &v, const mat4 &m){
    vec4 result;
    // Finding the dot product of the vector with each column of the matrix.
    for(int i=0; i<4; i++){
        result[i] = dot(m[i],v);
    }
    return result;
}

/// Prints the matrix to a stream in a nice format
std::ostream &operator<<(std::ostream &o, const mat4 &m){
    // Print result as Matrix : \n m11 , m12 , ... \n m21 , ... \n ... \ ... , m4
    o << "Matrix : "<<std::endl;
    for(int i=0; i<4; i++){
        for(int j=0; j<4; j++){
            o<<m[j][i];
            if(j<3){
                o<<" , ";
            }
            else if(i<3){
                o<<std::endl;
            }
        }
    }
    return o;
}
