#include <stdio.h>  // paste the contents of stdio.h here, including the prototype for printf()
#include <stdlib.h> // malloc and free are defined here
#include <string.h> // string.h contains the prototype for memset()
#include <assert.h> // needed to use the assert() function for debugging
#include <math.h>
#include "img_proc.h"
#include "raster_tools.h"

#if 1
#define ARRIVED_HERE fprintf(stderr,"Arrived Here: %s : %d\n",__FILE__,__LINE__)
#else
#define ARRIVED_HERE
#endif

#define M_PI 3.14159265358979323846


img_t *process_image(img_t *img, int *applyProc, int win_size, float ang, float sig) {

  int size = img->w * img->h;

  int wide = img->w;
  int height = img->h;

  //Run through the options one by one

  if (applyProc[0] == 1) {
      img = grayscale(img,size);
  }
  if (applyProc[1] == 1) {
      img = flip(img,size);
  }
  if (applyProc[2] == 1) {
      img = flop(img,size);
  }
  if (applyProc[3] == 1) {
      img = transpose(img,size);
  }
  if (applyProc[4] == 1) {
      img = boxblur(img,size,win_size);
  }
  if (applyProc[5] == 1){
      img = median(img,size,win_size);
  }
  if (applyProc[6] == 1) {
      img = gaussian(img,size,win_size,sig);
  }
  if (applyProc[7] == 1) {
      img = rotate(img,ang);
      img = resize(img,wide,height);
  }
  if (applyProc[8] == 1) {
      img = grayscale(img,size);
      img = sobel(img,size);
  }

//  write_ppm(img, outFile);

//  // free up memory
//  destroy_img(&img); // &img is the address in memory where the img variable is stored
//                     // Since img is of type (img *), &img is of type (img **)
  return img;
}


//Bubble sort function
unsigned char *bubble_sort(unsigned char *arr, int len){
    for(int i=0;i<(len-1);i++){
        for(int j=0;j<(len-1-i);j++){
            if(arr[j]>arr[j+1]){
                unsigned char temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
    return arr;
}

//Rotate x pixels
double rotx(int row,int col,double th, double c_x,double c_y){
    double cf = ((((double)col)-c_x)*cos(th)) - ((((double)row)-c_y)*sin(th)) + c_x;
    return cf;
}

//Rotate y pixel
double roty(int row,int col,double th, double c_x,double c_y){
    double rf = ((((double)col)-c_x)*sin(th)) + ((((double)row)-c_y)*cos(th)) + c_y;
    return rf;
}

img_t *grayscale(img_t *img,int size){
    for (pixel_t *p = img->data; p < img->data + size; p++) {
        //Implement the grayscale equation over each pixel
        double y = 0.299*p->r+0.587*p->g+0.114*p->b;
        //Set each pixel to same value
        p->r = p->b = p->g = (unsigned char)(round(y));
        //printf("%p\n",p);
    }
    return img;
}

img_t *flip(img_t *img,int size){
    for (pixel_t *p = img->data; p < img->data + size; p++) {
        //int row = (p-img->data)/(img->w);
        int col = (p-img->data)%(img->w);
        if( col<img->w/2){
            //Find location symmetrically opposite horizontally
            pixel_t *opp_location = p+img->w-(2*col)-1;
            //Swap pixel values
            unsigned char temp_r = p->r;
            unsigned char temp_g = p->g;
            unsigned char temp_b = p->b;
            p->r = opp_location->r;
            p->g = opp_location->g;
            p->b = opp_location->b;
            opp_location->r = temp_r;
            opp_location->g = temp_g;
            opp_location->b = temp_b;
        }
    }
    return img;
}

img_t *flop(img_t *img,int size){
    for (pixel_t *p = img->data; p < img->data + size; p++) {
        int row = (p-img->data)/(img->w);
        int col = (p-img->data)%(img->w);
        if( row<img->h/2){
            //Find the symmetrically opposite row
            int opp_row = img->h - row - 1;
            //Find the location symmetrically opposite vertically
            pixel_t *opp_location = img->data + (opp_row*img->w) + col;
            //Swap pixel values
            unsigned char temp_r = p->r;
            unsigned char temp_g = p->g;
            unsigned char temp_b = p->b;
            p->r = opp_location->r;
            p->g = opp_location->g;
            p->b = opp_location->b;
            opp_location->r = temp_r;
            opp_location->g = temp_g;
            opp_location->b = temp_b;
        }
    }
    return img;
}

img_t *transpose(img_t *img,int size){
    // Initialize new image with width and height swapped
    img_t *transf_img = new_img(img->h,img->w);
    for (pixel_t *p = img->data; p < img->data + size; p++) {
        int row = (p-img->data)/(img->w);
        int col = (p-img->data)%(img->w);
        //Row and column in the transposed image are swapped
        int opp_row = col;
        int opp_col = row;
        //Find location in the transposed image that corresponds to current pixel
        pixel_t *opp_location = transf_img->data + (opp_row*transf_img->w) + opp_col;
        //printf("%d,%d,%d,%d\n",row,col,(opp_location-transp_img->data)/(transp_img->w),(opp_location-transp_img->data)%(transp_img->w));
        opp_location->r = p->r;
        opp_location->g = p->g;
        opp_location->b = p->b;
    }
    //Store the address of the new image in the old image pointer. Destroy new image pointer.
    destroy_img(&img);
    img = transf_img;
    transf_img = NULL;

    //Alternate way of copying?
/*
    img->w = transf_img->w;
    img->h = transf_img->h;
    for (int j = 0;  j < size; j++) {
        img->data[j] =  transf_img->data[j];
    }
    destroy_img(&transf_img);
*/
    return img;
}

img_t *boxblur(img_t *img,int size, int n){
    // Initialize new image with same width and height
    img_t *transf_img = new_img(img->w,img->h);
    for (pixel_t *p = img->data; p < img->data + size; p++) {
        int row = (p-img->data)/(img->w);
        int col = (p-img->data)%(img->w);
        //check if window application is possible
        if(col>=n && row>=n && col<(img->w - n) && row<(img->h - n)){
            //initialize variable to store the sum of weights (for normalization)
            double w_sum = 0;
            //initialize the vaiable to store sum of (weight*pixel_value)
            double sum_r = 0;
            double sum_g = 0;
            double sum_b = 0;
            //Initialize variable to store weight, row, column and address information about a pixel in the window
            double we;
            int c_r;
            int c_c;
            pixel_t *loc;
            //loop across row and column of the window
            for(int r = -n; r<=n; r++){
                for(int c = -n; c<=n; c++){
                    we = 1.0;
                    w_sum += we;
                    c_r = row + r;
                    c_c = col + c;
                    loc = img->data + (c_r*img->w) + c_c;
                    sum_r += we*(double)loc->r;
                    sum_g += we*(double)loc->g;
                    sum_b += we*(double)loc->b;
                }
            }
            //Set pixel value in new image
            pixel_t *location = transf_img->data + (p-img->data);
            location->r = (unsigned char)(round(sum_r/w_sum));
            location->g = (unsigned char)(round(sum_g/w_sum));
            location->b = (unsigned char)(round(sum_b/w_sum));
        }
        else{
            //Set pixels where window cannot be applied as green
            pixel_t *location = transf_img->data + (p-img->data);
            location->r = 0;
            location->g = 255;
            location->b = 0;
        }
    }
    //Store the address of the new image in the old image pointer. Destroy new image pointer.
    destroy_img(&img);
    img = transf_img;
    transf_img = NULL;
    return img;
}

img_t *median(img_t *img,int size, int n){
    // Initialize new image with same width and height
    img_t *transf_img = new_img(img->w,img->h);
    unsigned char *arr_r = (unsigned char *) malloc(((2*n+1)*(2*n+1))*sizeof(unsigned char));
    unsigned char *arr_g = (unsigned char *) malloc(((2*n+1)*(2*n+1))*sizeof(unsigned char));
    unsigned char *arr_b = (unsigned char *) malloc(((2*n+1)*(2*n+1))*sizeof(unsigned char));
    for (pixel_t *p = img->data; p < img->data + size; p++) {
        int row = (p-img->data)/(img->w);
        int col = (p-img->data)%(img->w);
        //check if window application is possible
        if(col>=n && row>=n && col<(img->w - n) && row<(img->h - n)){
            //Initialize the array for each pixel
            memset(arr_r, 0, ((2*n+1)*(2*n+1)) * sizeof(unsigned char));
            memset(arr_g, 0, ((2*n+1)*(2*n+1)) * sizeof(unsigned char));
            memset(arr_b, 0, ((2*n+1)*(2*n+1)) * sizeof(unsigned char));
            int c_r;
            int c_c;
            pixel_t *loc;
            int count  = 0;
            //Loop to store the pixel values in the window
            for(int r = -n; r<=n; r++){
                for(int c = -n; c<=n; c++){
                    c_r = row + r;
                    c_c = col + c;
                    loc = img->data + (c_r*img->w) + c_c;
                    arr_r[count] = loc->r;
                    arr_g[count] = loc->g;
                    arr_b[count] = loc->b;
                    count++;
                }
            }
            // Sorting the 3 arrays
            arr_r = bubble_sort(arr_r, (2*n+1)*(2*n+1));
            //for(int k = 0; k<((2*n+1)*(2*n+1));k++){printf("%d,",arr_r[k]);}
            //printf("\n");
            arr_g = bubble_sort(arr_g, (2*n+1)*(2*n+1));
            arr_b = bubble_sort(arr_b, (2*n+1)*(2*n+1));
            // Find location of pixel in transformed images and set it equal to the median (n th element in the arrays)
            pixel_t *location = transf_img->data + (p-img->data);
            location->r = (arr_r[n]);
            location->g = (arr_g[n]);
            location->b = (arr_b[n]);
        }
        // Set boundary pixels as green
        else{
            pixel_t *location = transf_img->data + (p-img->data);
            location->r = 0;
            location->g = 255;
            location->b = 0;
        }
    }

    //Destroy allocated memory
    free(arr_r);
    arr_r = NULL;
    free(arr_g);
    arr_g = NULL;
    free(arr_b);
    arr_b = NULL;
    //Store the address of the new image in the old image pointer. Destroy new image pointer.
    destroy_img(&img);
    img = transf_img;
    transf_img = NULL;
    return img;
}

img_t *gaussian(img_t *img,int size, int n, float s){
    //img_t *transf_img_x = new_img(img->w,img->h);
    // Initializing 3 arrays of double type to store intermediate values after applying the row gaussian
    double *red_x = (double *) malloc(size*sizeof(double));
    double *green_x = (double *) malloc(size*sizeof(double));
    double *blue_x = (double *) malloc(size*sizeof(double));
    memset(red_x, 0, img->w * img->h * sizeof(double));
    memset(green_x, 0, img->w * img->h * sizeof(double));
    memset(blue_x, 0, img->w * img->h * sizeof(double));
    // Create a new image of same width and height to store the final image
    img_t *transf_img = new_img(img->w,img->h);
    // Loop to calculate the intermediate values after applying row gaussian
    for (pixel_t *p = img->data; p < img->data + size; p++) {
        int row = (p-img->data)/(img->w);
        int col = (p-img->data)%(img->w);
        // Check if window can be applied (only column constraint since a row filter is applied first)
        if(col>=n && col<(img->w - n)){
            double w_sum = 0;
            double sum_r = 0;
            double sum_g = 0;
            double sum_b = 0;
            double we;
            int c_r;
            int c_c;
            pixel_t *loc;
            //Loop to calculate sum of weights and sum of (weights* pixel value) in window
            for(int c = -n; c<=n; c++){
                we = exp((-pow((double)c,2.0))/(2*pow(s,2.0)));
                w_sum += we;
                c_r = row;
                c_c = col + c;
                loc = img->data + (c_r*img->w) + c_c;
                sum_r += we*(double)loc->r;
                sum_g += we*(double)loc->g;
                sum_b += we*(double)loc->b;
            }
            //Store the intermediate values in the array
            red_x[p-img->data] = sum_r/w_sum;
            green_x[p-img->data] = sum_g/w_sum;
            blue_x[p-img->data] = sum_b/w_sum;
        }
        else{
            //Set boundary column pixels (not row) as green
            red_x[p-img->data] = 0.0;
            green_x[p-img->data] = 255.0;
            blue_x[p-img->data] = 0.0;
        }
    }
    // Loop to perform column gaussian
    // It involves the same steps as above. We just apply the gaussian column wise and use the intermediate values from the arrays created above.
    // The calculated values are stored in the new image tranf_image created above.
    for (pixel_t *p = img->data; p < img->data + size; p++) {
        int row = (p-img->data)/(img->w);
        int col = (p-img->data)%(img->w);
        //Checking row and column both here since column filter applied and boundary columns are green respectively,
        if(col>=n && row>=n && col<(img->w - n) && row<(img->h - n)){
            double w_sum = 0;
            double sum_r = 0;
            double sum_g = 0;
            double sum_b = 0;
            double we;
            int c_r;
            int c_c;
            int loc;
            for(int r = -n; r<=n; r++){
                we = exp((-pow((double)r,2.0))/(2*pow(s,2.0)));
                w_sum += we;
                c_r = row + r;
                c_c = col;
                // Calculate index in the array to be used
                loc = (c_r*img->w) + c_c;
                //Using the intermediate pixel values from the array above
                sum_r += we*red_x[loc];
                sum_g += we*green_x[loc];
                sum_b += we*blue_x[loc];
            }
            // Store the calculated values to the new image
            pixel_t *location = transf_img->data + (p-img->data);
            location->r = (unsigned char)(round(sum_r/w_sum));
            location->g = (unsigned char)(round(sum_g/w_sum));
            location->b = (unsigned char)(round(sum_b/w_sum));
        }
        else{
            pixel_t *location = transf_img->data + (p-img->data);
            location->r = 0;
            location->g = 255;
            location->b = 0;
        }
    }
    // Destroy the 3 arrays and the old image. Store the address of the new image in the old image pointer. Destroy new image pointer.
    free(red_x);
    red_x = NULL;
    free(green_x);
    green_x = NULL;
    free(blue_x);
    blue_x = NULL;
    destroy_img(&img);
    img = transf_img;
    transf_img = NULL;
    return img;
}

img_t *rotate(img_t *img,float th){
    //Convert theta to radians
     th = (th*M_PI)/180.0;

     //Find old image center
     double c_x_old = (((double)(img->w)+1)/2.0)-1;
     double c_y_old = (((double)(img->h)+1)/2.0)-1;
     //Arrays to hold the 4 corners
     int x_coord[4] = {0,img->w-1,img->w-1,0};
     int y_coord[4] = {0,0,img->h-1,img->h-1};
     double max_x=0.0;
     double min_x=img->w;
     double max_y=0.0;
     double min_y=img->h;
     double cur_x;
     double cur_y;
     //Loop to find rotated coordinates of corner in original image
     for(int i = 0;i<4;i++){
         //Note in the image frame theta rotation is clockwise positive.
         //Thus we rotate clockwise theta to reach original image
         cur_x = rotx(y_coord[i],x_coord[i],th,c_x_old,c_y_old);
         cur_y = roty(y_coord[i],x_coord[i],th,c_x_old,c_y_old);
         //printf("%d,%d\n",x_coord[i],y_coord[i]);
         //printf("%f,%f\n",cur_x,cur_y);
         if(cur_x>max_x){max_x=cur_x;}
         if(cur_x<min_x){min_x=cur_x;}
         if(cur_y>max_y){max_y=cur_y;}
         if(cur_y<min_y){min_y=cur_y;}
     }
     //Find new height and width of the image
     int w_new = round(max_x-min_x);
     int h_new = round(max_y-min_y);
     //printf("%d,%d,%d,%d\n",img->w,img->h,w_new,h_new);

     // Initialize new image depending on the larger parameter
     img_t *transf_img = new_img(w_new,h_new);

     //New center
     double c_x = (((double)(w_new)+1)/2.0)-1;
     double c_y = (((double)(h_new)+1)/2.0)-1;


     for (pixel_t *p = transf_img->data; p < transf_img->data + (transf_img->w*transf_img->h); p++) {
         int row = (p-transf_img->data)/(transf_img->w);
         int col = (p-transf_img->data)%(transf_img->w);

         //Rotate point by -th to get the original point (In the image frame rotation clockwise is positive due to th row column convention)
         double cf = rotx(row,col,th,c_x,c_y);
         double rf = roty(row,col,th,c_x,c_y);

         //Shifting the frame from new to old
         double cf_old = cf-c_x+c_x_old;
         double rf_old = rf-c_y+c_y_old;

         //Defining an epsilon to test double limits
         double eps = 0.00000001;

         if(((rf_old-(double)(img->h - 1))<eps) && ((rf_old-0.0)> (-eps)) && (((cf_old)-(double)(img->w - 1))<eps) && ((cf_old-0.0)>(-eps))){
             int r_old;
             if (rf_old<0.0){r_old = 0;}
             else{r_old = floor(rf_old);}
             int c_old;
             if (cf_old<0.0){c_old = 0;}
             else{c_old = floor(cf_old);}
             double del_r = rf_old-(double)r_old;
             double del_c = cf_old-(double)c_old;
             //Finding the 4 locations from img->data to add for interpolation
             pixel_t *v1 = img->data + (r_old*img->w) + (c_old);
             pixel_t *v2 = img->data + ((r_old + 1)*img->w) + (c_old);
             pixel_t *v3 = img->data + (r_old*img->w) + (c_old + 1);
             pixel_t *v4 = img->data + ((r_old + 1)*img->w) + (c_old + 1);
             //Set value of new pixel using the data from the 4 locations above
             p->r = (unsigned char)round((((double)v1->r)*(1-del_r)*(1-del_c))+
                                    (((double)v2->r)*(del_r)*(1-del_c))+
                                    (((double)v3->r)*(1-del_r)*(del_c))+
                                    (((double)v4->r)*(del_r)*(del_c)));
             p->g = (unsigned char)round((((double)v1->g)*(1-del_r)*(1-del_c))+
                                    (((double)v2->g)*(del_r)*(1-del_c))+
                                    (((double)v3->g)*(1-del_r)*(del_c))+
                                    (((double)v4->g)*(del_r)*(del_c)));
             p->b = (unsigned char)round((((double)v1->b)*(1-del_r)*(1-del_c))+
                                    (((double)v2->b)*(del_r)*(1-del_c))+
                                    (((double)v3->b)*(1-del_r)*(del_c))+
                                    (((double)v4->b)*(del_r)*(del_c)));
         }
         else{
             //printf("%f,%f\n",rf_old,cf_old);
             p->r = 0;
             p->g = 255;
             p->b = 0;
         }
     }
     //Store the address of the new image in the old image pointer. Destroy new image pointer.
     destroy_img(&img);
     img = transf_img;
     transf_img = NULL;
     return img;
}

img_t *sobel(img_t *img,int size){
    //Setting Window radius
    int n=1;
    //img_t *transf_img = new_img(img->w,img->h);
    // Array to hold the temporary Magnitude matrix values
    double *temp_dat = (double *) malloc(size*sizeof(double));
    memset(temp_dat, 0, img->w * img->h * sizeof(double));
    //Defining the sobel filters
    double sob_x[9] = {-1.0,0.0,1.0,-2.0,0.0,2.0,-1.0,0.0,1.0};
    double sob_y[9] = {1.0,2.0,1.0,0.0,0.0,0.0,-1.0,-2.0,-1.0};
    //Defining a variable to store the maximum magnitude value
    double maxim = 0.0;
    for (pixel_t *p = img->data; p < img->data + size; p++) {
        int row = (p-img->data)/(img->w);
        int col = (p-img->data)%(img->w);
        //check if window application is possible
        if(col>=n && row>=n && col<(img->w - n) && row<(img->h - n)){
            //initialize the vaiable to store sum of (weight*pixel_value)
            double sum_x = 0;
            double sum_y = 0;
            //Initialize variable to store weight, row, column and address information about a pixel in the window
            double we_x;
            double we_y;
            int c_r;
            int c_c;
            pixel_t *loc;
            int sob_count = 0;
            //loop across row and column of the window and apply both filters and store the magnitude in the sum variables
            //Applying sobel filter
            for(int r = -n; r<=n; r++){
                for(int c = -n; c<=n; c++){
                    we_x = sob_x[sob_count];
                    we_y = sob_y[sob_count];
                    sob_count += 1;
                    c_r = row + r;
                    c_c = col + c;
                    loc = img->data + (c_r*img->w) + c_c;
                    sum_x += we_x*(double)loc->r;
                    sum_y += we_y*(double)loc->r;
                }
            }
            //Store the magnitude after implementation of the filters in the array formed above
            int location = (p-img->data);
            temp_dat[location] = sqrt(pow(sum_x,2.0)+pow(sum_y,2.0));
            // If condition to find maximum magnitude value
            if(temp_dat[location]>maxim){
                maxim = temp_dat[location];
            }
        }
    }

    // Update the image with the calculated magnitude values normalized with the maximum magnitude calculated above and multiplied by 255
    for (pixel_t *p = img->data; p < img->data + size; p++) {
        int row = (p-img->data)/(img->w);
        int col = (p-img->data)%(img->w);
        //check if window application is possible
        if(col>=n && row>=n && col<(img->w - n) && row<(img->h - n)){
            int location = (p-img->data);
            p->r = (unsigned char)round(temp_dat[location]*255.0/maxim);
            p->g = (unsigned char)round(temp_dat[location]*255.0/maxim);
            p->b = (unsigned char)round(temp_dat[location]*255.0/maxim);
        }
        // Set pixels green at the boundary
        else{
            p->r = 0;
            p->g = 255;
            p->b = 0;
        }
    }

    //Free the array memory and set the pointer to NULL.
    free(temp_dat);
    temp_dat = NULL;
    return img;
}

img_t *resize(img_t *img,int w_new,int h_new){
    // Initialize new image with new width and height
    img_t *transf_img = new_img(w_new,h_new);
    //Get ratios of old image to new
    double s_r = ((double)img->h)/((double)h_new);
    double s_c = ((double)img->w)/((double)w_new);
    for (pixel_t *p = transf_img->data; p < transf_img->data + (w_new*h_new); p++) {
        int row = (p-transf_img->data)/(w_new);
        int col = (p-transf_img->data)%(w_new);
        double rf = ((double)row) * s_r;
        double cf = ((double)col) * s_c;
        double r_old = floor(rf);
        double c_old = floor(cf);
        double del_r = rf-r_old;
        double del_c = cf-c_old;
        //Finding the 4 locations from img->data to add for interpolation
        pixel_t *v1 = img->data + (((int)r_old)*img->w) + ((int)c_old);
        pixel_t *v2 = img->data + (((int)r_old + 1)*img->w) + ((int)c_old);
        pixel_t *v3 = img->data + (((int)r_old)*img->w) + ((int)c_old + 1);
        pixel_t *v4 = img->data + (((int)r_old + 1)*img->w) + ((int)c_old + 1);
        //Set value of new pixel using the data from the 4 locations above
        p->r = (unsigned char)round((((double)v1->r)*(1-del_r)*(1-del_c))+
                               (((double)v2->r)*(del_r)*(1-del_c))+
                               (((double)v3->r)*(1-del_r)*(del_c))+
                               (((double)v4->r)*(del_r)*(del_c)));
        p->g = (unsigned char)round((((double)v1->g)*(1-del_r)*(1-del_c))+
                               (((double)v2->g)*(del_r)*(1-del_c))+
                               (((double)v3->g)*(1-del_r)*(del_c))+
                               (((double)v4->g)*(del_r)*(del_c)));
        p->b = (unsigned char)round((((double)v1->b)*(1-del_r)*(1-del_c))+
                               (((double)v2->b)*(del_r)*(1-del_c))+
                               (((double)v3->b)*(1-del_r)*(del_c))+
                               (((double)v4->b)*(del_r)*(del_c)));
    }
    //Store the address of the new image in the old image pointer. Destroy new image pointer.
    destroy_img(&img);
    img = transf_img;
    transf_img = NULL;
    return img;
}
