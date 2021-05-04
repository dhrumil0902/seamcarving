#include "c_img.h"
#include <stdio.h>
#include <math.h>
#include <inttypes.h>

void create_img(struct rgb_img **im, size_t height, size_t width){
    *im = (struct rgb_img *)malloc(sizeof(struct rgb_img));
    (*im)->height = height;
    (*im)->width = width;
    (*im)->raster = (uint8_t *)malloc(3 * height * width);   
}

int read_2bytes(FILE *fp){
    uint8_t bytes[2];
    fread(bytes, sizeof(uint8_t), 1, fp);
    fread(bytes+1, sizeof(uint8_t), 1, fp);
    return (  ((int)bytes[0]) << 8)  + (int)bytes[1];
}

void write_2bytes(FILE *fp, int num){
    uint8_t bytes[2];
    bytes[0] = (uint8_t)((num & 0XFFFF) >> 8);
    bytes[1] = (uint8_t)(num & 0XFF);
    fwrite(bytes, 1, 1, fp);
    fwrite(bytes+1, 1, 1, fp);
}

void read_in_img(struct rgb_img **im, char *filename){
    FILE *fp = fopen(filename, "rb");
    size_t height = read_2bytes(fp);
    size_t width = read_2bytes(fp);
    //printf("%ld, -%ld,", height, width);
    create_img(im, height, width);
    fread((*im)->raster, 1, 3*width*height, fp);
    fclose(fp);
}

void write_img(struct rgb_img *im, char *filename){
    FILE *fp = fopen(filename, "wb");
    write_2bytes(fp, im->height);
    write_2bytes(fp, im->width);
    fwrite(im->raster, 1, im->height * im->width * 3, fp);
    fclose(fp);
}

uint8_t get_pixel(struct rgb_img *im, int y, int x, int col){
    return im->raster[3 * (y*(im->width) + x) + col];
}

void set_pixel(struct rgb_img *im, int y, int x, int r, int g, int b){
    im->raster[3 * (y*(im->width) + x) + 0] = r;
    im->raster[3 * (y*(im->width) + x) + 1] = g;
    im->raster[3 * (y*(im->width) + x) + 2] = b;
}

void destroy_image(struct rgb_img *im)
{
    free(im->raster);
    free(im);
}


void print_grad(struct rgb_img *grad){
    int height = grad->height;
    int width = grad->width;
    //printf("%d , %d,\n",width, height);
    for(int i = 0; i < height; i++){
        for(int j = 0; j < width; j++){
            printf("%d\t", get_pixel(grad, i, j, 0));
        }
        printf("\n");    
    }
}
void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    int h = im->height;
    int w = im->width;
    int diffx0;int diffx1;int diffx2;int diffy0;int diffy1;int diffy2;
    double sum;
    int val;
    create_img(grad,h,w);
    for(int i = 0;i<w;i++){
        sum = 0.0;
        for(int j =0;j<h;j++){
            diffx0 = get_pixel(im,j,(i+1)%w,0) - get_pixel(im,j,(w-1+i)%w,0);
            diffx1 = get_pixel(im,j,(i+1)%w,1) - get_pixel(im,j,(w-1+i)%w,1);
            diffx2 = get_pixel(im,j,(i+1)%w,2) - get_pixel(im,j,(w-1+i)%w,2);
            diffy0 = get_pixel(im,(j+1)%h,i,0) - get_pixel(im,(h-1+j)%h,i,0);
            diffy1 = get_pixel(im,(j+1)%h,i,1) - get_pixel(im,(h-1+j)%h,i,1);
            diffy2 = get_pixel(im,(j+1)%h,i,2) - get_pixel(im,(h-1+j)%h,i,2);
            sum = pow(diffx0,2) + pow(diffx1,2) + pow(diffx2,2)+ pow(diffy0,2)+ pow(diffy1,2)+ pow(diffy2,2);
            val = (uint8_t)(sqrt(sum)/10.0);
            set_pixel(*grad,j,i,val,val,val);
            sum = 0.0;

        }
    }
}
void dynamic_seam(struct rgb_img *grad, double **best_arr){    
    double *cost = (double*)malloc(sizeof(double)*(grad->height)*(grad->width));           
    for(int i =0;i<(grad->height)*(grad->width);i++){
        cost[i]=0.0;
    }
     for(int i =0;i<(grad->width);i++){
         for(int j=0;j<grad->height;j++){
         cost[j*grad->width +i] = (double)get_pixel(grad,j,i,0);
         }
     }
     int w = grad->width;
     int max_width = grad->width-1;
     for(int i =1;i<grad->height;i++){
         for(int j =0;j<grad->width;j++){
             if(j==0){
                cost[i*w +j] = fmin(cost[(i-1)*w +j],cost[(i-1)*w +j+1]) + cost[i*w +j];
             }
             else if (j==max_width){
                 cost[i*w+j]= fmin(cost[(i-1)*w +j-1],cost[(i-1)*w +j]) + cost[i*w +j];
             }
             else{
                 cost[i*w+j] = fmin(fmin(cost[(i-1)*w +j-1],cost[(i-1)*w +j]),cost[(i-1)*w +j+1]) + cost[i*w +j];
             }
         }
     }
     *best_arr = cost;
     /*for(int i =0;i<(grad->height);i++){
         for(int j = 0; j<grad->width;j++){
         //printf("%f  ",cost[i*grad->width +j]);
         }
         //printf("\n");
     }*/

}
void recover_path(double *best, int height, int width, int **path){
    int *p= (int *)malloc(sizeof(int)*height);
    double min;
    min = best[(height-1)*width];
    int index=0;
    for(int i =0;i<width;i++){
        if(best[(height-1)*width +i]<min){
            index = i;
        }
    } 

    p[height-1]= index;
    for(int i =height-2;i>=0;i--){
        //printf("ind: %d\n",index);
             if(index==0){
                if(best[(i)*width]>best[(i)*width+1]){
                    index = index+1;
                }
             }
             else if (index==width-1){
                  if(best[(i)*width +index]>best[(i)*width+index-1]){
                    index = index-1;
                }
             }
             else{
                 min = fmin(fmin(best[(i)*width +index-1],best[(i)*width +index]),best[(i)*width +index+1]);
                 if(min==best[(i)*width +index-1]){
                     index = index-1;
                 }
                 else if(min ==best[(i)*width +index+1]){
                     index = index+1;
                 }
             }

             p[i] = index;
     }

    *path= p;
    /*for(int i =0;i<height;i++){
        printf("%d ",p[i]);
    }
    printf("\n");*/
}
void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path){
    int ind;
    create_img(dest,src->height,src->width-1);
    for(int i = 0;i<src->height;i++){
        ind = 0;
        for(int j =0;j<src->width;j++){
            if(path[i]!=j){
                set_pixel(*dest,i,ind,get_pixel(src,i,j,0),get_pixel(src,i,j,1),get_pixel(src,i,j,2));
                ind= ind +1;
            }
        }
    }
}


int main(void){

    struct rgb_img *im;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;

    read_in_img(&im, "img149.bin");
    calc_energy(im,  &grad);
        dynamic_seam(grad, &best);
        recover_path(best, grad->height, grad->width, &path);
        for(int i =0;i<grad->height;i++){
            printf("%d\n",path[i]);
        }
        remove_seam(im, &cur_im, path);

}