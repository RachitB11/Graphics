#define _USE_MATH_DEFINES
#include "raster_tools.h"
#include <iostream>
#include "math.h"

using namespace std;

int main(int argc, char *argv[])
{
    if(argc<6){
            cout << "Not enough arguments" << endl;
            return 0;
        }
        //Take in data from the command line arguments
        char *obj_file = argv[1];
        char *cam_file = argv[2];
        int w = atoi(argv[3]);
        int h = atoi(argv[4]);
        char *out_file = argv[5];
        char *opt;
        if(argc==7){
            opt = argv[6];
        }
        else{
            opt = NULL;
        }

    // Load object and see contents
    vector<tinyobj::shape_t> shapes;
    vector<tinyobj::material_t> materials;
    string temp_str = LoadObj( shapes, materials, obj_file);

    // Load camera parameters and estimate the entire perspective matrix to convert from world to camera pixel coordinates (& Z (in [0,1]))
    cam_dat cam = get_permat(cam_file);

    // Make vectors to store homogeneous coordinates and normal data so that they can be accessed later using indices
    vector <vector <vec4>> homo_coord;
    vector <vector <vec4>> normals;

    // Setting temporary containers
    vec4 temp;
    vector <vec4> temp_coord;
    vector <vec4> temp_norm;

    // Loop to store data
    for(unsigned int j = 0; j < shapes.size(); j++){

        for(unsigned int i = 0; i < shapes[j].mesh.positions.size(); i += 3){

            temp = vec4(shapes[j].mesh.positions[i], shapes[j].mesh.positions[i+1], shapes[j].mesh.positions[i+2], 1);
            temp = cam.per_mat * temp;

            //Convert to homogeneous coordinates (NDC)
            temp /= temp[3];

            // Convert NDC to pixel coordinates
            temp[0] = (float)((temp[0] + 1) * ((float) w) / 2.0);
            temp[1] = (float)((1 - temp[1]) * ((float) h) / 2.0);
            temp[3] = i/3;
            temp_coord.push_back(temp);

            // Rotate the normals to the camera frame
            temp = cam.rot_mat * vec4(shapes[j].mesh.normals[i], shapes[j].mesh.normals[i+1], shapes[j].mesh.normals[i+2], 1);
            temp[3] = i/3;
            temp_norm.push_back(temp);

        }

        homo_coord.push_back(temp_coord);
        normals.push_back(temp_norm);

    }


    // Container to store face information (vertex coordinates and normals)
    vector< vector <face> > pix_triangles;

    // Loop to store face data
    for(unsigned int i = 0; i < shapes.size(); i++){

        vector <face> shape_triangles = world_to_im(shapes[i], homo_coord[i], normals[i]);
//        cout<<shapes[0].mesh.positions.size()<<endl<<shape_triangles.size()<<endl;
        pix_triangles.push_back(shape_triangles);

    }

    // Calculate the bounding boxes for each triangle using the vertex info
    vector< vector <bbox> > bboxes;

    // Loop to store bounding box data
    for(unsigned int i = 0; i < shapes.size(); i++){

        vector <bbox> bbox_temp = get_bbox(pix_triangles[i], w , h);
//        for(bbox i: bbox_temp){cout<<i.x<<" "<<i.y<<" "<<i.w<<" "<<i.h<<endl;}
        bboxes.push_back(bbox_temp);

    }

    // Scan along each row and find left and right edge intersections.
    // Apply checks and check for special cases and arrive at 1 (when just touching) or 2 coordinates (when passing thru triangle)
    vector <vector <corn_pts>> corner_pts;

    // Loop through to find the intersection points for each face (triangle)
    for(unsigned int i = 0; i < shapes.size(); i++){
        vector <corn_pts> cpts_temp = get_corners(pix_triangles[i], bboxes[i]);
        corner_pts.push_back(cpts_temp);
    }

    // Initialize the image
    img_t *img = new_img(w,h);

    // Set container for Z-buffer. Initialize all values to 2.
    vector <float> z_info((w*h),2.0);

    // Loop to fill the image using face data, corner intersection data and other data depending on the option chosen.
    for(unsigned int i = 0; i < shapes.size(); i++){
        img = fill_img(img, pix_triangles[i],corner_pts[i], materials[i],z_info,homo_coord[i],
                       normals[i], opt);
    }

    // Store the image generated in a file
    write_ppm(img, out_file);
    destroy_img(&img);

    // Destroy the image
    return 0;
}

