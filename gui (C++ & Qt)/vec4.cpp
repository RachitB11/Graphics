#include "vec4.h"
#include <assert.h>
#include "math.h"

///----------------------------------------------------------------------
/// Constructors
///----------------------------------------------------------------------
vec4::vec4(void){
    for(int i=0 ; i<4 ; i++){
        data[i] = 0;
    }
}

vec4::vec4(float x, float y, float z, float w){
    data[0]=x;
    data[1]=y;
    data[2]=z;
    data[3]=w;
}

vec4::vec4(const vec4 &v2){
    for(int i=0 ; i<4 ; i++){
        data[i] = v2.data[i];
    }
}

///----------------------------------------------------------------------
/// Getters/Setters
///----------------------------------------------------------------------

/// Returns the value at index
/// Does NOT check the array bound because doing so is slow
float vec4::operator[](unsigned int index) const{
    return data[index];
}
/// Returns a reference to the value at index
/// Does NOT check the array bound because doing so is slow
float &vec4::operator[](unsigned int index){
    return data[index];
}

/// Returns the value at index
/// DOES check the array bound with assert even though is slow
float vec4::operator()(unsigned int index) const{
    assert(index<4);
    return data[index];
}

/// Returns a reference to the value at index
/// DOES check the array bound with assert even though is slow
float &vec4::operator()(unsigned int index){
    assert(index<4);
    return data[index];
}

/////----------------------------------------------------------------------
///// Operator Methods
/////----------------------------------------------------------------------

///// Assign v2 to this and return a reference to this
vec4 &vec4::operator=(const vec4 &v2){
    for(int i=0 ; i<4 ; i++){
        data[i] = v2[i];
    }
    return *this;
}

/// Test for equality
bool vec4::operator==(const vec4 &v2) const{   //Component-wise comparison
    for(int i=0 ; i<4 ; i++){
        //If there is even one data mismatch, return false
        if(v2[i] != data[i]){
                return false;
        }
    }
    return true;
}

/// Test for inequality
bool vec4::operator!=(const vec4 &v2) const{ //Component-wise comparison
    //Return not of the return of the equality operator
    return !(*this==v2);
}

/// Arithmetic:
/// e.g. += adds v2 to this and return this (like regular +=)
///      +  returns a new vector that is sum of this and v2
vec4 &vec4::operator+=(const vec4 &v2){
    for(int i=0 ; i<4 ; i++){
        data[i] += v2[i];
    }
    return *this;
}

vec4 &vec4::operator-=(const vec4 &v2){
    for(int i=0 ; i<4 ; i++){
        data[i] -= v2[i];
    }
    return *this;
}

vec4 &vec4::operator*=(float c){// multiplication by a scalar
    for(int i=0 ; i<4 ; i++){
        data[i] *= c;
    }
    return *this;
}

vec4 &vec4::operator/=(float c){// division by a scalar
    for(int i=0 ; i<4 ; i++){
        data[i] /= c;
    }
    return *this;
}


vec4 vec4::operator+(const vec4 &v2) const{ //Element wise addition
    vec4 result;
    for(int i=0 ; i<4 ; i++){
        result[i] = data[i] + v2[i];
    }
    return result;
}

vec4 vec4::operator-(const vec4 &v2) const{ //Element wise subtraction
    vec4 result;
    for(int i=0 ; i<4 ; i++){
        result[i] = data[i] - v2[i];
    }
    return result;
}

vec4 vec4::operator*(float c) const{  // multiplication by a scalar
    vec4 result;
    for(int i=0 ; i<4 ; i++){
        result[i] = data[i] * c;
    }
    return result;
}

vec4 vec4::operator/(float c) const{ // division by a scalar
    vec4 result;
    for(int i=0 ; i<4 ; i++){
        result[i] = data[i] / c;
    }
    return result;
}

///----------------------------------------------------------------------
/// Other Methods
///----------------------------------------------------------------------

/// Returns the geometric length of the input vector
float vec4::length() const{
    float n = 0.0;
    // Length is the root of sum of squares of the elements
    for(int i=0 ; i<4 ; i++){
        n += pow(data[i],2.0);
    }
    return sqrt(n);
}

/// return a new vec4 that is a normalized (unit-length) version of this one
vec4 vec4::normalize() const{
    // Return vector divided by its length
    float n = (*this).length();
    return *this/n;
}

/// noralize this vector in place
void vec4::norm(){
    // Reassign the vector to its normalized form
    *this = (*this).normalize();
}

///----------------------------------------------------------------------
/// Other Functions (not part of the vec4 class)
///------

/// Dot Product
float dot(const vec4 &v1, const vec4 &v2){
    float sum = 0.0;
    // Dot product is sum of element wise product of the 2 vectors
    for(int i=0 ; i<4 ; i++){
        sum += v1[i]*v2[i];
    }
    return sum;
}

/// Cross Product
vec4 cross(const vec4 &v1, const vec4 &v2){
    //Compute the result of v1 x v2 using only their X, Y, and Z elements.
    //In other words, treat v1 and v2 as 3D vectors, not 4D vectors.
    //The fourth element of the resultant vector should be 0.
    vec4 result; // Initialize zero result vector
    //Implement cross product formula
    result[0] = v1[1]*v2[2] - v2[1]*v1[2];
    result[1] = v1[2]*v2[0] - v2[2]*v1[0];
    result[2] = v1[0]*v2[1] - v2[0]*v1[1];
    return result;
}

/// Scalar Multiplication (c * v)
vec4 operator*(float c, const vec4 &v){
    vec4 result;
    for(int i=0 ; i<4 ; i++){
        result[i] = c*v[i];
    }
    return result;
}

/// Prints the vector to a stream in a nice format for integration with cout
std::ostream &operator<<(std::ostream &o, const vec4 &v){
    // Return as "Vector : v1 , v2 , v3 , v4" format
    o << "Vector : " << v[0] << " , " << v[1] << " , " << v[2] << " , " << v[3];
    return o;
}
