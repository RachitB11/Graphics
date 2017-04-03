#include "raster_tools.h"
#include <assert.h>
#include <stdlib.h> // malloc and free are defined here
#include <string.h> // string.h contains the prototype for memset()
#include <assert.h> // needed to use the assert() function for debugging
#include <math.h>

// Create a new image of specified size.
img_t *new_img(int w, int h) {
  assert(w > 0);
  assert(h > 0);
  img_t *img = (img_t *) malloc(sizeof(img_t));

  // now initialize img appropriately
  img->w = w;
  img->h = h;

  // allocate memory for the image pixels
  img->data = (pixel_t *) malloc(w * h * sizeof(pixel_t));

  // zero out all the image pixels so they don't contain garbage
  memset(img->data, 0, w * h * sizeof(pixel_t));

  return img;
}

// Destroy an image and free up its memory
void destroy_img(img_t **img) {
  // step 1: free the image pixels
  free((*img)->data); // as long as you allocated img->data with malloc, free knows how big it is
  (*img)->data = NULL; // this is a sanity check to make sure we don't try and access img->data after deleting it
  free(*img); // now free the img_t structure
  *img = NULL; // finally, set the img pointer provided by the caller to NULL so the caller
  // can't accidentally access the freed memory
}

// Read in a PPM file
img_t *read_ppm(const char *fname) {
  int w, h;

  assert(fname != NULL); // crash if fname is NULL
  FILE *f = fopen(fname, "rb"); // open the ppm for reading in binary mode
  assert(f != NULL); // crash if the file didn't open

  fscanf(f, "P6 %d %d 255%*c", &w, &h); // read in the header and image width and height

  img_t *img = new_img(w, h); // create an empty image of the correct size

  fread(img->data, 3, w * h, f); // read the image data into img->data

  fclose(f); // close the file

  return img;
}

// Write out a PPM file
void write_ppm(const img_t *img, const char *fname) {
  assert(img != NULL); // crash if img is NULL
  assert(fname != NULL); // crash if fname is NULL

  FILE *f = fopen(fname, "wb"); // open fname for writing in binary mode; clobbers any existing file
  assert(f != NULL); // crash if file did not open

  fprintf(f, "P6\n%d %d 255\n", img->w, img->h); // write the image header
  fwrite(img->data, img->w * img->h, 3, f); // write the image data

  fclose(f);
}

// Read camera file and find perspective matrix
cam_dat get_permat(float *params){

    cam_dat cam;

    float left, right, top, bottom, near, far, eye_x, eye_y, eye_z, center_x, center_y, center_z, up_x, up_y, up_z;

    left =  params[0];
    right =  params[1];
    top =  params[2];
    bottom =  params[3];
    near =  params[4];
    far =  params[5];
    eye_x =  params[6];
    eye_y =  params[7];
    eye_z =  params[8];
    center_x =  params[9];
    center_y =  params[10];
    center_z =  params[11];
    up_x =  params[12];
    up_y =  params[13];
    up_z =  params[14];

//    Estimating aspect ratio and tan of (FOV / 2)
//    float aspect = ( right - left ) / ( top  - bottom );

//    float t_fov = ( top  - bottom )/(2*near);

//    Building the projection matrix
    mat4 proj_mat(vec4( ((2 * near) / (right - left)), 0, 0, 0 ),
                  vec4( 0, ((2 * near) / (top - bottom)), 0, 0),
                  vec4( ((right + left) / (right - left)), ((top + bottom) / (top - bottom)), (far / (far - near)), 1),
                  vec4( 0, 0, ( -(far * near) / (far-near)), 0));

    cam.proj_mat = proj_mat;

//    Finding the 3 vectors that make up the rotation matrix (Make sure to normalize)
    vec4 rig;
    vec4 look(center_x - eye_x, center_y - eye_y, center_z - eye_z, 0);
    look.norm();
    vec4 up(up_x, up_y, up_z, 0);
    up.norm();
    rig  = cross(look,up).normalize();

//    Make rotation matrix by transposing the matrix whose columns are the vectors calculated above
    mat4 rot_mat(rig, up,look, vec4(0, 0, 0, 1));
    rot_mat = rot_mat.transpose();

//    Calculate translation matrix
    mat4 trans_mat = (mat4::trans(-eye_x, -eye_y, -eye_z));
    cam.rot_mat = rot_mat;
    cam.trans_mat = trans_mat;

//    Calculate the perspective matrix
    cam.per_mat = proj_mat * rot_mat * trans_mat;

//    Return Camera Data
    return cam;
}

// Convert the vertices to pixel coordinates (Also calculate Z in [0,1]) and return vector of triangles
vector<face> world_to_im(tinyobj::shape_t &shapes, vector <vec4> &homo_coord, vector <vec4> &normals){

    // Initialize containers to store data about triangles, index and depth of the 3 vertices
    vector<face> triangles;
    unsigned int ind;
    float z1,z2,z3;

    // Loop through to store the triangles data
    for(unsigned int i = 0; i < shapes.mesh.indices.size(); i += 3){
        face temp;

        // Use index data to find the 3 vertices
        ind = shapes.mesh.indices[i];
        temp.p1 = homo_coord[ind];
        z1 = temp.p1[2];
//        cout<<temp.p1<<endl;

        ind = shapes.mesh.indices[i+1];
        temp.p2 = homo_coord[ind];
        z2 = temp.p2[2];

        ind = shapes.mesh.indices[i+2];
        temp.p3 = homo_coord[ind];
        z3 = temp.p3[2];

        // Check if all the depth values are within the near and far distance
        if (!((z1 > 1 && z2 > 1 && z3 > 1) || (z1 < 0 && z2 < 0 && z3 < 0))){

            // Add normal data to the temporary face container
            temp.n1 = normals[shapes.mesh.indices[i]];
            temp.n2 = normals[shapes.mesh.indices[i+1]];
            temp.n3 = normals[shapes.mesh.indices[i+2]];

            // Add face container to the list of triangles
            triangles.push_back(temp);
//            cout<<temp.p1<<endl<<temp.p2<<endl<<temp.p3<<endl<<endl;

        }

    }

    return triangles;

}

// Given the pixels of triangles find the bounding box for each of them
vector<bbox> get_bbox(vector<face> &pix_triangles, int w, int h){

    // Initialize containers to store info about minimum x and y values for each face.
    vector <bbox> bboxes;
    float min_x, min_y, max_x, max_y;
    bbox temp;

    // List to store indexes of removed triangles
    vector <int> rem;

    int count = 0;

    for( face v : pix_triangles){

        // Find minimum x and y coordinates (clamp with 0 limit)
        min_x = max(min(min(v.p1[0],v.p2[0]),v.p3[0]),(float)0.0);
        min_y = max(min(min(v.p1[1],v.p2[1]),v.p3[1]),(float)0.0);

        // If the minimum coordinates are out of the plane add its index to the removed list
        if((min_x > w) || (min_y > h)){
            rem.insert(rem.begin(),count);
            count++;
            continue;}

        // Find minimum x and y coordinates (clamp with w/h limit)
        max_x = min(max(max(v.p1[0],v.p2[0]),v.p3[0]),(float)w);
        max_y = min(max(max(v.p1[1],v.p2[1]),v.p3[1]),(float)h);

        // If the maximum coordinates are out of the plane add its index to the removed list
        if((max_x < 0.0) || (max_y < 0.0)){
            rem.insert(rem.begin(),count);
            count++;
            continue;}

        // Estimate bounding box using the minimum and maximum pixel values
        temp.x = min_x;
        temp.y = min_y;
        temp.w = max_x - min_x;
        temp.h = max_y - min_y;
        bboxes.push_back(temp);
        count++;

    }

    // Removing faces that are not completely in the screen
    for(unsigned int i = 0; i < rem.size(); i++){

        pix_triangles.erase(pix_triangles.begin() + rem[i]);

    }
return bboxes;

}


// Scan along each row and find left and right edge intersections.
vector<corn_pts> get_corners(vector<face> &pix_triangle, vector<bbox> &bboxes){

    // Initialize containers to hold corners, current bbox, current face and vector to store triangle line data for current face.
    vector<corn_pts> corners;
    bbox b;
    face f;
    vector <line_dat> triang_line;

    // Store limits of the scan lines
    float y_start, y_end;

    // Looping through number of bounding boxes
    for(unsigned int i = 0; i < bboxes.size(); i++){

        corn_pts c;

        b = bboxes[i];
        f = pix_triangle[i];

        // Define limits of the scan lines
        y_start = floor(b.y);
        y_end = ceil(b.y + b.h);
//        cout<<y_start<<" "<<y_end<<endl;

        line_dat line;

        // Check the vertex which has the bigger x coordinate and store it as p2 of the line. Smaller one as p1.
        // Do this for all the 3 lines of the triangle
        // Also store the slope of each line
        if(f.p1[0] >= f.p2[0]){
            line.p1 = f.p2;
            line.p2 = f.p1;
        }
        else{
            line.p1 = f.p1;
            line.p2 = f.p2;
        }
        line.m = ((line.p2[1] - line.p1[1]) / (line.p2[0] - line.p1[0]));
        triang_line.push_back(line);

        if(f.p2[0] >= f.p3[0]){
            line.p1 = f.p3;
            line.p2 = f.p2;
        }
        else{
            line.p1 = f.p2;
            line.p2 = f.p3;
        }
        line.m = ((line.p2[1] - line.p1[1]) / (line.p2[0] - line.p1[0]));
        triang_line.push_back(line);

        if(f.p1[0] >= f.p3[0]){
            line.p1 = f.p3;
            line.p2 = f.p1;
        }
        else{
            line.p1 = f.p1;
            line.p2 = f.p3;
        }
        line.m = ((line.p2[1] - line.p1[1]) / (line.p2[0] - line.p1[0]));
        triang_line.push_back(line);

        //Vector to store intersection points for a particular scan line
        vector <vec4> in;

        //Float to store x value during scanning
        float x;

        // Loop for each scan
        for(float j = y_start; j <= y_end; j+=1.0){

            // Loop to find intersections of the scan line with the sides of the triangle
            for( line_dat l : triang_line){

                // Deal with horizontal lines
                // Check if slope is zero
                // Check if the line has the same y axis as the scan line
                if(l.m == 0){

                   if(j == l.p1[1]){

                       // If scan line is concurrent with the line, push the end points along with index information of the end points
                       in.push_back(vec4(l.p1[0],l.p1[1],l.p1[3],l.p2[3]));
                       in.push_back(vec4(l.p2[0],l.p2[1],l.p1[3],l.p2[3]));
                        break;

                   }

               }

               // Deal with vertical lines
               // If slope is infinity, check if the height of the scan line is between the y coordinates of the end points
               if(isinf(l.m)){

                   if( (j<=max(l.p1[1],l.p2[1])) && (j>=min(l.p1[1],l.p2[1])) ){

                       // If it lies in between push the point and the indexes of the end points
                       in.push_back(vec4(l.p1[0],j,l.p1[3],l.p2[3]));
                       continue;

                   }

               }

               // Deal with normal lines
               // If the scan line lies between the y coordinates of the end points it intersects it.
               if( (j<=max(l.p1[1],l.p2[1])) && (j>=min(l.p1[1],l.p2[1])) ){

                   // Calculate the x coordinate of intersection point
                   x = (j - l.p1[1] + (l.m*l.p1[0])) / l.m;
                   in.push_back(vec4(x,j,l.p1[3],l.p2[3]));

               }

            }

            // NOTE: If the scan line touches the vertex, the same point will be added to the 'in' vector twice. So no need to handle that case.

            // If the number of intersection points is greater than 2 remove one of the equal pair
            if(in.size()>2){
                float eps = 0.001;
                if((in[0][0] - in[1][0] < eps) && (in[0][1] - in[1][1] < eps)){in.erase(in.begin());}
                else if((in[0][0] - in[2][0] < eps) && (in[0][1] - in[2][1] < eps)){in.erase(in.begin());}
                else{in.erase(in.begin()+2);}
            }

            // Sometimes it may happen that the scan line may pass through without intersection
            if(in.size() == 0){
                continue;
            }

            // Check which intersection point has a greater x value and store it as the left intersection
            if(in[0][0] < in[1][0]){
                c.lef.push_back(in[0]);
                c.rig.push_back(in[1]);
            }

            else{
                c.lef.push_back(in[1]);
                c.rig.push_back(in[0]);
            }

            in.clear();

        }
//        cout<<c.lef.size()<<" "<<c.rig.size()<<endl;

        corners.push_back(c);
        triang_line.clear();
    }

    return corners;
}

// Filling the image using intersection points and other info as required by a particular option.
img_t *fill_img(img_t *img, vector<face> triangles, vector<corn_pts> &corner_pts,
                tinyobj::material_t &materials, vector <float> &z,  vector <vec4> &homo_coord, vector <vec4> &normals,
                char *opt){

    // Check the option and run the required function
    if (strcmp(opt,"--default") == 0){

        img = default_col(img, corner_pts,materials, z, homo_coord);

    }
    else if (strcmp (opt, "--white") == 0) {

        img = white_col(img, corner_pts, z, homo_coord);

    }
    else if (strcmp (opt, "--norm_flat") == 0) {

        img = flat_col(img, triangles, corner_pts, z, homo_coord);

    }
    else if (strcmp (opt, "--norm_gouraud") == 0) {

        img = gouraud_col(img, corner_pts, z, homo_coord, normals);

    }
    else if (strcmp (opt, "--norm_bary") == 0) {

        img = bary_col(img, triangles, corner_pts, z);

    }
    else if (strcmp (opt, "--norm_gouraud_z") == 0) {

        img = gouraud_col_z(img, corner_pts, z, homo_coord, normals);

    }
    else if (strcmp (opt, "--norm_bary_z") == 0) {

        img = bary_col_z(img, triangles, corner_pts, z);

    }

    return img;

}

// Coloring using the diffuse property in the materials object
img_t *default_col(img_t *img, vector<corn_pts> &corner_pts, tinyobj::material_t &materials,
                   vector<float> &z, vector <vec4> &homo_coord){
//    cout<<"ok"<<endl;

    int w = img->w;

    // Set color as the color from the diffuse property
    unsigned int color[] = {(unsigned int)round(materials.diffuse[0]*255.0),(unsigned int)round(materials.diffuse[1]*255.0),
                             (unsigned int)round(materials.diffuse[2]*255.0)};

    // Initialize variables
    vec4 temp, p_start, p_stop;
    int start, stop, x_start, x_stop, y;
    float z_start,z_stop,z_cur;
    int count;

    //Loop through the number of triangles = size of corner_pts
    for(unsigned int i = 0; i < corner_pts.size(); i++){

        // Loop through the pairs of intersections
        for(unsigned int j = 0; j < corner_pts[i].lef.size(); j++){

            //Left intersection point in the image
            temp = corner_pts[i].lef[j];
            x_start = max(0,(int)temp[0]);

            // If the left intersection point is greater than the width, then discard the scan line
            if(x_start>=w)  continue;
            y = (int)temp[1];

            // Interpolate the depth value using the vertices of the line in which it lies
            z_start = interp_z(homo_coord[temp[2]],homo_coord[temp[3]],temp[0],temp[1]);

            // Store start vector
            p_start = vec4((int)temp[0],(int)temp[1],z_start,1);

            // Store position of the start point (may not be equal to the start vector xy value)
            start = (y*w) + (x_start);

            //Right intersection point in the image
            temp = corner_pts[i].rig[j];
            x_stop = min((int)temp[0],w-1);

            // If the right intersection point is less than 0, then discard the scan line
            if(x_stop<0) continue;
            y = (int)temp[1];

            // Interpolate the depth value using the vertices of the line in which it lies
            z_stop = interp_z(homo_coord[temp[2]],homo_coord[temp[3]],temp[0],temp[1]);

            // Store stop vector
            p_stop = vec4((int)temp[0],(int)temp[1],z_stop,1);

            // Store position of the start point (may not be equal to the stop vector xy value)
            stop = (y*w) + (x_stop);

            count = 0;

            //Loop to assign pixel value of the row from the start point to the stop point
            for (pixel_t *p = (img->data + start); p <= (img->data + stop); p++) {

                // Find current depth using interpolation of depth of end points
                z_cur = interp_z(p_start,p_stop,x_start + count, y);
                count ++;

                // If the depth is within range and lower than current depth value in the buffer update the pixel color and the Z-buffer
                if((z_cur<z[p-(img->data)]) && (z_cur>0) && (z_cur<1)){
                    z[p-(img->data)] = z_cur;
                    p->r = color[0];
                    p->g = color[1];
                    p->b = color[2];
                }
            }
        }
    }
    return img;
}

// Coloring all pixels in the triangle white
img_t *white_col(img_t *img, vector<corn_pts> &corner_pts, vector <float> &z, vector <vec4> &homo_coord){

    int w = img->w;

    // All pixels are assigned white color
    unsigned int color[] = {255,255,255};

    // Initialize various container variables
    vec4 temp, p_start, p_stop;
    int start, stop, x_start, x_stop, y;
    float z_start,z_stop,z_cur;
    int count;

    //Loop through the number of triangles = size of corner_pts
    for(unsigned int i = 0; i < corner_pts.size(); i++){

        // Loop through the pairs of intersections
        for(unsigned int j = 0; j < corner_pts[i].lef.size(); j++){

            //Left intersection point in the image
            temp = corner_pts[i].lef[j];
            x_start = max(0,(int)temp[0]);

            // If the left intersection point is greater than the width, then discard the scan line
            if(x_start>=w)  continue;
            y = (int)temp[1];

            // Interpolate the depth value using the vertices of the line in which it lies
            z_start = interp_z(homo_coord[temp[2]],homo_coord[temp[3]],temp[0],temp[1]);

            // Store start vector
            p_start = vec4((int)temp[0],(int)temp[1],z_start,1);

            // Store position of the start point (may not be equal to the start vector xy value)
            start = (y*w) + (x_start);

            //Right intersection point in the image
            temp = corner_pts[i].rig[j];
            x_stop = min((int)temp[0],w-1);

            // If the right intersection point is less than 0, then discard the scan line
            if(x_stop<0) continue;
            y = (int)temp[1];

            // Interpolate the depth value using the vertices of the line in which it lies
            z_stop = interp_z(homo_coord[temp[2]],homo_coord[temp[3]],temp[0],temp[1]);

            // Store stop vector
            p_stop = vec4((int)temp[0],(int)temp[1],z_stop,1);

            // Store position of the start point (may not be equal to the stop vector xy value)
            stop = (y*w) + (x_stop);

            count = 0;

            //Loop to assign pixel value of the row from the start point to the stop point
            for (pixel_t *p = (img->data + start); p <= (img->data + stop); p++) {

                // Find current depth using interpolation of depth of end points
                z_cur = interp_z(p_start,p_stop,x_start + count, y);
                count ++;

                // If the depth is within range and lower than current depth value in the buffer update the pixel color and the Z-buffer
                if((z_cur<z[p-(img->data)]) && (z_cur>0) && (z_cur<1)){
                    z[p-(img->data)] = z_cur;
                    p->r = color[0];
                    p->g = color[1];
                    p->b = color[2];
                }
            }
        }
    }
    return img;
}

// Coloring all the pixels using the normal of the 1st vertex
img_t *flat_col(img_t *img, vector <face> &triangles, vector<corn_pts> &corner_pts, vector <float> &z,
                 vector <vec4> &homo_coord){

    int w = img->w;

    // Initialize various container variables
    vec4 temp, p_start, p_stop;
    int start, stop, x_start, x_stop, y;
    float z_start,z_stop,z_cur;
    vec4 norm_cur;
    vector<unsigned int> color = {255,255,255};
    int count;

    //Loop through the number of triangles = size of corner_pts
    for(unsigned int i = 0; i < corner_pts.size(); i++){

        // Extract the first normal from the face
        norm_cur = triangles[i].n1;

        // Get color from the normal
        color = get_color(color,norm_cur);

        // Loop through the pairs of intersections
        for(unsigned int j = 0; j < corner_pts[i].lef.size(); j++){

            //Left intersection point in the image
            temp = corner_pts[i].lef[j];
            x_start = max(0,(int)temp[0]);

            // If the left intersection point is greater than the width, then discard the scan line
            if(x_start>=w)  continue;
            y = (int)temp[1];

            // Interpolate the depth value using the vertices of the line in which it lies
            z_start = interp_z(homo_coord[temp[2]],homo_coord[temp[3]],temp[0],temp[1]);

            // Store start vector
            p_start = vec4((int)temp[0],(int)temp[1],z_start,1);

            // Store position of the start point (may not be equal to the start vector xy value)
            start = (y*w) + (x_start);

            //Right intersection point in the image
            temp = corner_pts[i].rig[j];
            x_stop = min((int)temp[0],w-1);

            // If the right intersection point is less than 0, then discard the scan line
            if(x_stop<0) continue;
            y = (int)temp[1];

            // Interpolate the depth value using the vertices of the line in which it lies
            z_stop = interp_z(homo_coord[temp[2]],homo_coord[temp[3]],temp[0],temp[1]);

            // Store stop vector
            p_stop = vec4((int)temp[0],(int)temp[1],z_stop,1);

            // Store position of the start point (may not be equal to the stop vector xy value)
            stop = (y*w) + (x_stop);

            count = 0;

            //Loop to assign pixel value of the row from the start point to the stop point
            for (pixel_t *p = (img->data + start); p <= (img->data + stop); p++) {

                // Find current depth using interpolation of depth of end points
                z_cur = interp_z(p_start,p_stop,x_start + count, y);
                count ++;

                // If the depth is within range and lower than current depth value in the buffer update the pixel color and the Z-buffer
                if((z_cur<z[p-(img->data)]) && (z_cur>0) && (z_cur<1)){
                    z[p-(img->data)] = z_cur;
                    p->r = color[0];
                    p->g = color[1];
                    p->b = color[2];
                }
            }
        }
    }
    return img;
}

// Coloring using gouraud shading
img_t *gouraud_col(img_t *img, vector<corn_pts> &corner_pts, vector <float> &z,
                 vector <vec4> &homo_coord, vector <vec4> &normals){

    int w = img->w;

    // Initialize various container variables
    vec4 temp, p_start, p_stop;
    int start, stop, x_start, x_stop, y;
    pt_info pt_start, pt_stop, pt_cur;
    vector<unsigned int> color = {255,255,255};
    int count;

    //Loop through the number of triangles = size of corner_pts
    for(unsigned int i = 0; i < corner_pts.size(); i++){


        // Loop through the pairs of intersections
        for(unsigned int j = 0; j < corner_pts[i].lef.size(); j++){

            //Left intersection point in the image
            temp = corner_pts[i].lef[j];
            x_start = max(0,(int)temp[0]);

            // If the left intersection point is greater than the width, then discard the scan line
            if(x_start>=w)  continue;
            y = (int)temp[1];

            // Get interpolated depth and normal values from the values at the vertex
            pt_start = interp_pt(normals[temp[2]],normals[temp[3]],homo_coord[temp[2]],homo_coord[temp[3]],
                    temp[0],temp[1]);

            // Store start vector
            p_start = vec4((int)temp[0],(int)temp[1],pt_start.z,1);

            // Store position of the start point (may not be equal to the start vector xy value)
            start = (y*w) + (x_start);

            //Right intersection point in the image
            temp = corner_pts[i].rig[j];
            x_stop = min((int)temp[0],w-1);

            // If the right intersection point is less than 0, then discard the scan line
            if(x_stop<0) continue;
            y = (int)temp[1];

            // Get interpolated depth and normal values from the values at the vertex
            pt_stop = interp_pt(normals[temp[2]],normals[temp[3]],homo_coord[temp[2]],homo_coord[temp[3]],
                    temp[0],temp[1]);

            // Store start vector
            p_stop = vec4((int)temp[0],(int)temp[1],pt_stop.z,1);

            // Store position of the start point (may not be equal to the start vector xy value)
            stop = (y*w) + (x_stop);

            count = 0;

            //Loop to assign pixel value of the row from the start point to the stop point
            for (pixel_t *p = (img->data + start); p <= (img->data + stop); p++) {

                // Get interpolated depth and normal values from the values at the vertex
                pt_cur = interp_pt(pt_start.norm,pt_stop.norm,p_start, p_stop, x_start + count, y);
                count ++;

                // If the depth is within range and lower than current depth value in the buffer update the pixel color and the Z-buffer
                if((pt_cur.z<z[p-(img->data)]) && (pt_cur.z>0) && (pt_cur.z<1)){
                    z[p-(img->data)] = pt_cur.z;
                    color = get_color(color,pt_cur.norm);
                    p->r = color[0];
                    p->g = color[1];
                    p->b = color[2];
                }
            }
        }
    }
    return img;
}

// Coloring using barycentric coordinates
img_t *bary_col(img_t *img, vector<face> &triangles, vector<corn_pts> &corner_pts, vector <float> &z){

    int w = img->w;

    // Initialize various container variables
    vec4 temp;
    int start, stop, x_start, x_stop, y;
    pt_info pt_cur;
    vector<unsigned int> color = {255,255,255};
    int count;
    face f;

    //Loop through the number of triangles = size of corner_pts
    for(unsigned int i = 0; i < corner_pts.size(); i++){

        // Set f to current face
        f = triangles[i];

        // Loop through the pairs of intersections
        for(unsigned int j = 0; j < corner_pts[i].lef.size(); j++){

            //Left intersection point in the image
            temp = corner_pts[i].lef[j];
            x_start = max(0,(int)temp[0]);

            // If the left intersection point is greater than the width, then discard the scan line
            if(x_start>=w)  continue;
            y = (int)temp[1];

            // Store position of the start point (may not be equal to the start vector xy value)
            start = (y*w) + (x_start);

            //Right intersection point in the image
            temp = corner_pts[i].rig[j];
            x_stop = min((int)temp[0],w-1);

            // If the right intersection point is less than 0, then discard the scan line
            if(x_stop<0) continue;
            y = (int)temp[1];

            // Store position of the start point (may not be equal to the start vector xy value)
            stop = (y*w) + (x_stop);

            count = 0;

            //Loop to assign pixel value of the row from the start point to the stop point
            for (pixel_t *p = (img->data + start); p <= (img->data + stop); p++) {

                // Estimate the depth and normal for some point in the triangle using barycentric coordinates
                pt_cur = interp_barypt(f, x_start + count, y);

                count ++;

                // If the depth is within range and lower than current depth value in the buffer update the pixel color and the Z-buffer
                if((pt_cur.z<z[p-(img->data)]) && (pt_cur.z>0) && (pt_cur.z<1)){
                    z[p-(img->data)] = pt_cur.z;
                    color = get_color(color,pt_cur.norm);
                    p->r = color[0];
                    p->g = color[1];
                    p->b = color[2];
                }
            }
        }
    }
    return img;
}

// Coloring using gouraud shading and perspective corrected depth
img_t *gouraud_col_z(img_t *img, vector<corn_pts> &corner_pts, vector <float> &z,
                 vector <vec4> &homo_coord, vector <vec4> &normals){
    int w = img->w;

    // Initialize various container variables
    vec4 temp, p_start, p_stop;
    int start, stop, x_start, x_stop, y;
    pt_info pt_start, pt_stop, pt_cur;
    vector<unsigned int> color = {255,255,255};
    int count;

    //Loop through the number of triangles = size of corner_pts
    for(unsigned int i = 0; i < corner_pts.size(); i++){


        // Loop through the pairs of intersections
        for(unsigned int j = 0; j < corner_pts[i].lef.size(); j++){

            //Left intersection point in the image
            temp = corner_pts[i].lef[j];
            x_start = max(0,(int)temp[0]);

            // If the left intersection point is greater than the width, then discard the scan line
            if(x_start>=w)  continue;
            y = (int)temp[1];

            // Get interpolated depth and normal values from the values at the vertex
            pt_start = interp_pt_z(normals[temp[2]],normals[temp[3]],homo_coord[temp[2]],homo_coord[temp[3]],
                    temp[0],temp[1]);

            // Store start vector
            p_start = vec4((int)temp[0],(int)temp[1],pt_start.z,1);

            // Store position of the start point (may not be equal to the start vector xy value)
            start = (y*w) + (x_start);

            //Right intersection point in the image
            temp = corner_pts[i].rig[j];
            x_stop = min((int)temp[0],w-1);

            // If the right intersection point is less than 0, then discard the scan line
            if(x_stop<0) continue;
            y = (int)temp[1];

            // Get interpolated depth and normal values from the values at the vertex
            pt_stop = interp_pt_z(normals[temp[2]],normals[temp[3]],homo_coord[temp[2]],homo_coord[temp[3]],
                    temp[0],temp[1]);

            // Store start vector
            p_stop = vec4((int)temp[0],(int)temp[1],pt_stop.z,1);

            // Store position of the start point (may not be equal to the start vector xy value)
            stop = (y*w) + (x_stop);

            count = 0;

            //Loop to assign pixel value of the row from the start point to the stop point
            for (pixel_t *p = (img->data + start); p <= (img->data + stop); p++) {

                // Get interpolated depth and normal values from the values at the vertex
                pt_cur = interp_pt_z(pt_start.norm,pt_stop.norm,p_start, p_stop, x_start + count, y);
                count ++;

                // If the depth is within range and lower than current depth value in the buffer update the pixel color and the Z-buffer
                if((pt_cur.z<z[p-(img->data)]) && (pt_cur.z>0) && (pt_cur.z<1)){
                    z[p-(img->data)] = pt_cur.z;
                    color = get_color(color,pt_cur.norm);
                    p->r = color[0];
                    p->g = color[1];
                    p->b = color[2];
                }
            }
        }
    }
    return img;
}

// Coloring using barycentric coordinates and perspective corrected depth
img_t *bary_col_z(img_t *img, vector<face> &triangles, vector<corn_pts> &corner_pts, vector <float> &z){

    int w = img->w;

    // Initialize various container variables
    vec4 temp;
    int start, stop, x_start, x_stop, y;
    pt_info pt_cur;
    vector<unsigned int> color = {255,255,255};
    int count;
    face f;

    //Loop through the number of triangles = size of corner_pts
    for(unsigned int i = 0; i < corner_pts.size(); i++){

        // Set f to current face
        f = triangles[i];

        // Loop through the pairs of intersections
        for(unsigned int j = 0; j < corner_pts[i].lef.size(); j++){

            //Left intersection point in the image
            temp = corner_pts[i].lef[j];
            x_start = max(0,(int)round(temp[0]));

            // If the left intersection point is greater than the width, then discard the scan line
            if(x_start>=w)  continue;
            y = (int)round(temp[1]);

            // Store position of the start point (may not be equal to the start vector xy value)
            start = (y*w) + (x_start);

            //Right intersection point in the image
            temp = corner_pts[i].rig[j];
            x_stop = min((int)round(temp[0]),w-1);

            // If the right intersection point is less than 0, then discard the scan line
            if(x_stop<0) continue;
            y = (int)round(temp[1]);

            // Store position of the start point (may not be equal to the start vector xy value)
            stop = (y*w) + (x_stop);

            count = 0;

            //Loop to assign pixel value of the row from the start point to the stop point
            for (pixel_t *p = (img->data + start); p <= (img->data + stop); p++) {

                // Estimate the depth and normal for some point in the triangle using barycentric coordinates
                pt_cur = interp_barypt_z(f, x_start + count, y);

                count ++;

                // If the depth is within range and lower than current depth value in the buffer update the pixel color and the Z-buffer
                if((pt_cur.z<z[p-(img->data)]) && (pt_cur.z>0) && (pt_cur.z<1)){
                    z[p-(img->data)] = pt_cur.z;
                    color = get_color(color,pt_cur.norm);
                    p->r = color[0];
                    p->g = color[1];
                    p->b = color[2];
                }
            }
        }
    }
    return img;
}

// Interpolate Z values (incorrect way)
float interp_z(vec4 p1, vec4 p2, float x, float y){
    float z;
    float alp;

    //Estimate alpha
    if(p1[1] == p2[1]){

        //Check if the points are not equal
        if(p1[0] == p2[0]){alp = 1;}

        else{alp = (float)(x-p1[0]) / (float) (p2[0] - p1[0]);}

//        if (alp<0){cout<<"error";}
    }
    else{

        alp = (float)(sqrt(pow((x-p1[0]),2.0)+pow((y-p1[1]),2.0)))/ (float)(sqrt(pow((p2[0]-p1[0]),2.0)+pow((p2[1]-p1[1]),2.0)));

    }

    // Use alpha to find interpolated depth
    z = ((1-alp)*(float)p1[2]) + ((alp)*(float)p2[2]);

    return z;
}

// Interpolate point normal and depth values (incorrect way)
pt_info interp_pt(vec4 n1, vec4 n2, vec4 p1, vec4 p2, float x, float y){

    pt_info pt;
    float alp;

    // Find alpha
    if(p1[1] == p2[1]){

        //Check if the points are not equal
        if(p1[0] == p2[0]){alp = 1;}

        else{alp = (float)(x-p1[0]) / (float) (p2[0] - p1[0]);}

//        if (alp<0){cout<<"error";}

    }
    else{

        alp = (float)(sqrt(pow((x-p1[0]),2.0)+pow((y-p1[1]),2.0)))/ (float)(sqrt(pow((p2[0]-p1[0]),2.0)+pow((p2[1]-p1[1]),2.0)));

    }

    //Find interpolated normal and depth values using alpha
    pt.norm = ((1-alp)*n1) + ((alp)*n2);
    pt.z = ((1-alp)*(float)p1[2]) + ((alp)*(float)p2[2]);
    pt.norm[3] = 0;
    pt.norm.norm();
    return pt;
}

// Interpolate point normal and depth values (correct way)
pt_info interp_pt_z(vec4 n1, vec4 n2, vec4 p1, vec4 p2, float x, float y){
    pt_info pt;
    float alp;
    float eps = 0.0001;

    // Find alpha value
    if(p1[1] == p2[1]){

        //Check if the points are not equal
        if(p1[0] == p2[0]){alp = 1;}

        else{alp = (float)(x-p1[0]) / (float) (p2[0] - p1[0]);}

//        if (alp<0){cout<<"error";}

    }
    else{

        alp = (float)(sqrt(pow((x-p1[0]),2.0)+pow((y-p1[1]),2.0)))/ (float)(sqrt(pow((p2[0]-p1[0]),2.0)+pow((p2[1]-p1[1]),2.0)));

    }

    // Estimate perspective corrected depth
    pt.z = 1.0 / (((1-alp)/(float)p1[2]) + ((alp)/(float)p2[2]));

    // Estimate new alpha if the depth values are not equal
    if(fabs(p2[2] - p1[2])>eps){
        alp  = ( pt.z - p1[2] ) / ( p2[2] - p1[2] );
        cout<<alp<<endl;
    }

//    if (alp<0){cout<<"error";}

    // Estimate the interpolated normal value using the perspective corrected depth
    pt.norm = pt.z*((1-alp)*n1/(float)p1[2]) + ((alp)*n2/(float)p2[2]);
    pt.norm[3] = 0;
    pt.norm.norm();

    return pt;
}

// Interpolate point normal and depth values using barycentric(incorrect way)
pt_info interp_barypt(face f, float x, float y){
    pt_info pt;
    float S, S1, S2, S3;

    // Estimate the 4 areas required to estimate the barycentric coordinates
    S1  = t_area(vec4(x,y,0,1),f.p2,f.p3);
    S2  = t_area(vec4(x,y,0,1),f.p3,f.p1);
    S3  = t_area(vec4(x,y,0,1),f.p1,f.p2);
    S  = S1 + S2 + S3;

    //Find the interpolated depth and normal values using the barycentric coordinates
    pt.norm = f.n1*(S1/S) + f.n2*(S2/S) + f.n3*(S3/S);
    pt.z = (float)f.p1[2]*(S1/S) + (float)f.p2[2]*(S2/S) + (float)f.p3[2]*(S3/S);
    pt.norm[3] = 0;
    pt.norm.norm();
    return pt;

}

// Interpolate point normal and depth values using barycentric(correct way)
pt_info interp_barypt_z(face f, float x, float y){
    pt_info pt;
    float S, S1, S2, S3;

    // Estimate the 4 areas required to estimate the barycentric coordinates
    S1  = t_area(vec4(x,y,0,1),f.p2,f.p3);
    S2  = t_area(vec4(x,y,0,1),f.p3,f.p1);
    S3  = t_area(vec4(x,y,0,1),f.p1,f.p2);
    S  = S1 + S2 + S3;

    //Estimate the perspective corrected depth using barycentric coordinates
    pt.z = 1.0 / (((S1/S)/(float)f.p1[2]) + ((S2/S)/(float)f.p2[2]) + ((S3/S)/(float)f.p3[2]));

    //Use the perspective corrected depth to estimate normal values
    pt.norm = pt.z*((f.n1*(S1/S)/f.p1[2]) + (f.n2*(S2/S)/f.p2[2]) + (f.n3*(S3/S)/f.p3[2]));
    pt.norm[3] = 0;
    pt.norm.norm();
    return pt;
}

// Get color from normal value
vector <unsigned int> get_color(vector <unsigned int> &color, vec4 normal){

    for(unsigned int i = 0; i<3; i++){

        // Since the normal ranges from -1 to 1 add 1 and half it before scaling to 255
        color[i] = (unsigned int) round((normal[i]+1)*0.5*255.0);

    }
    return color;
}

// Get triangle area
float t_area(vec4 p1, vec4 p2, vec4 p3){

    //Estimate the area made by 3 points by talking the half of the cross product of the 2 vectors made by the three points
    float v1x,v2x,v1y,v2y;
    v1x = p1[0] - p2[0];
    v1y = p1[1] - p2[1];
    v2x = p3[0] - p2[0];
    v2y = p3[1] - p2[1];
    return 0.5*((v1x*v2y) - (v2x*v1y));
}
