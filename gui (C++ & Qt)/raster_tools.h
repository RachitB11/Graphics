#ifndef RASTER_TOOLS_H
#define RASTER_TOOLS_H

#include "vec4.h"
#include "mat4.h"
#include "tiny_obj_loader.h"
using namespace std;

/// Pixel Structure
struct pixel_t{
  unsigned char r, g, b; // a pixel contains three bytes, namely r, g, and b
};

/// Image Structure
struct img_t {
  pixel_t *data; // img is a pointer to a block of memory containing pixels, i.e. an array of pixels
  int w, h; // image width and height
};

/// Camera matrices that govern the projection
struct cam_dat{
    mat4 proj_mat; // Projection matrix
    mat4 rot_mat; // Rotation matrix
    mat4 trans_mat; // Translation matrix
    mat4 per_mat; // Perspective matrix = proj_mat*rot_mat*trans_mat
};

/// Structure to store triangle face data
struct face{
  vec4 p1; // Store x, y, and z of point 1.
  vec4 p2; // Store x, y, and z of point 2.
  vec4 p3; // Store x, y, and z of point 3.
  vec4 n1; // Store x, y, and z of normal of point 1.
  vec4 n2; // Store x, y, and z of normal of point 2.
  vec4 n3; // Store x, y, and z of normal of point 3.
};

/// Structure to store bounding box info
struct bbox{
  float x; // Top Left x
  float y; // Top Left y
  float w; // Width of box
  float h; // Height of box
};

/// Structure to store edge intersection info in a particular bbox ((x,y) pixels data , depth and apended by 1 at the end)
struct corn_pts{
  vector <vec4> lef;//Contains info about x y pixel coordinates of left intersection and index of the points that created it.
  vector <vec4> rig;//Contains info about x y pixel coordinates of right intersection and index of the points that created it.
};

/// Structure to store line data
struct line_dat{
  vec4 p1; //Point one (with smaller x coordinate)
  vec4 p2; //Point two (with larger x coordinate)
  float m; //Slope
};

/// Structure to store color and depth info
struct pt_info{
    vec4 norm; // Normal data for a point (used to determine the color of the pixel)
    float z; // Depth of a point
};

/// Initialize Image handler functions
img_t *new_img(int w, int h);  // create a new image of specified width and height
void destroy_img(img_t **img); // delete img from memory
img_t *read_ppm(const char *fname); // read in an image in ppm format
void  write_ppm(const img_t *img, const char *fname); // write an image in ppm format

/// Read camera file and generate camera data
cam_dat get_permat(float *params);

/// Accumulate the triangles using the shape information in the model files and make a vector of triangles
vector<face> world_to_im(tinyobj::shape_t &shapes, vector <vec4> &homo_coord, vector <vec4> &normals);

/// Given the pixels of triangle vertices find the bounding box for each of them
vector<bbox> get_bbox(vector<face> &pix_triangles, int w, int h);

/// Scan along each row and find left and right edge intersections.
vector<corn_pts> get_corners(vector<face> &pix_triangle, vector<bbox> &bboxes);

/// Filling the image points using intersection points and color value derived dependent on the option given
img_t *fill_img(img_t *img, vector<face> triangles, vector<corn_pts> &corner_pts,
                tinyobj::material_t &materials, vector <float> &z,  vector <vec4> &homo_coord, vector <vec4> &normals,
                char *opt);

/// Coloring using the diffuse property in the materials object
img_t *default_col(img_t *img, vector<corn_pts> &corner_pts, tinyobj::material_t &materials,
                   vector<float> &z, vector <vec4> &homo_coord);

/// Coloring all the pixels in the triangle white
img_t *white_col(img_t *img, vector<corn_pts> &corner_pts, vector <float> &z, vector<vec4> &homo_coord);

/// Coloring each face with a single color derived from the first vertex index in the face
img_t *flat_col(img_t *img, vector <face> &triangles, vector<corn_pts> &corner_pts, vector <float> &z,
                 vector <vec4> &homo_coord);

/// Coloring using gouraud shading
img_t *gouraud_col(img_t *img, vector<corn_pts> &corner_pts, vector <float> &z,
                 vector <vec4> &homo_coord, vector <vec4> &normals);

/// Coloring using barycentric shading
img_t *bary_col(img_t *img, vector<face> &triangles, vector<corn_pts> &corner_pts, vector <float> &z);

/// Coloring using gouraud shading but with perspective corrected Z
img_t *gouraud_col_z(img_t *img, vector<corn_pts> &corner_pts, vector <float> &z,
                 vector <vec4> &homo_coord, vector <vec4> &normals);

/// Coloring using barycentric shading but with perspective corrected Z
img_t *bary_col_z(img_t *img, vector<face> &triangles, vector<corn_pts> &corner_pts, vector <float> &z);

/// Interpolate Z values
float interp_z(vec4 p1, vec4 p2, float x, float y);

/// Interpolate norm values and Z values
pt_info interp_pt(vec4 n1, vec4 n2, vec4 p1, vec4 p2, float x, float y);

/// Interpolate norm values and Z values using barycentric coordinates
pt_info interp_barypt(face f, float x, float y);

/// Interpolate point norm values and Z values using perspective corrected depth (correct way)
pt_info interp_pt_z(vec4 n1, vec4 n2, vec4 p1, vec4 p2, float x, float y);

/// Interpolate point norm values and Z values using perspective corrected depth and barycentric coordinates(correct way)
pt_info interp_barypt_z(face f, float x, float y);

/// Get triangle area
float t_area(vec4 p1, vec4 p2, vec4 p3);

/// Get color from normal value
vector <unsigned int> get_color(vector <unsigned int> &color, vec4 normal);

#endif // RASTER_TOOLS_H
